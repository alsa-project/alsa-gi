#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <sound/asound.h>
#include <sound/asequencer.h>
#include "client.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* For error handling. */
G_DEFINE_QUARK("ALSASeqClient", alsaseq_client)
#define raise(exception, errno)                             \
    g_set_error(exception, alsaseq_client_quark(), errno,   \
            "%d: %s", __LINE__, strerror(errno))

#define BUFFER_SIZE    1024
#define NUM_EVENTS    10

typedef struct {
    GSource src;
    ALSASeqClient *self;
    gpointer tag;
} SeqClientSource;

struct _ALSASeqClientPrivate {
    struct snd_seq_client_info info;
    struct snd_seq_client_pool pool;

    int fd;
    SeqClientSource *src;

    void *write_buf;
    unsigned int writable_bytes;
    GMutex write_lock;

    struct snd_seq_event read_ev[NUM_EVENTS];

    GList *ports;
    GMutex lock;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSASeqClient, alsaseq_client, G_TYPE_OBJECT)

/* TODO: Event filter. */

enum seq_client_prop {
    /* Client information */
    SEQ_CLIENT_PROP_NUMBER = 1,
    SEQ_CLIENT_PROP_TYPE,
    SEQ_CLIENT_PROP_NAME,
    SEQ_CLIENT_PROP_PORTS,
    SEQ_CLIENT_PROP_LOST,
    /* Event filter parameters */
    SEQ_CLIENT_PROP_BROADCAST_FILTER,
    SEQ_CLIENT_PROP_ERROR_BOUNCE,
    /* Client pool information */
    SEQ_CLIENT_PROP_OUTPUT_POOL,
    SEQ_CLIENT_PROP_INPUT_POOL,
    SEQ_CLIENT_PROP_OUTPUT_ROOM,
    SEQ_CLIENT_PROP_OUTPUT_FREE,
    SEQ_CLIENT_PROP_INPUT_FREE,
    SEQ_CLIENT_PROP_COUNT,
};

static GParamSpec *seq_client_props[SEQ_CLIENT_PROP_COUNT] = { NULL, };

static void seq_client_get_property(GObject *obj, guint id,
                                    GValue *val, GParamSpec *spec)
{
    ALSASeqClient *self = ALSASEQ_CLIENT(obj);
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);

    switch (id) {
    /* client information */
    case SEQ_CLIENT_PROP_NUMBER:
        g_value_set_uchar(val, priv->info.client);
        break;
    case SEQ_CLIENT_PROP_TYPE:
        g_value_set_enum(val, (ALSASeqClientTypeEnum)priv->info.type);
        break;
    case SEQ_CLIENT_PROP_NAME:
        g_value_set_string(val, priv->info.name);
        break;
    case SEQ_CLIENT_PROP_PORTS:
        g_value_set_int(val, priv->info.num_ports);
        break;
    case SEQ_CLIENT_PROP_LOST:
        g_value_set_int(val, priv->info.event_lost);
        break;
    case SEQ_CLIENT_PROP_BROADCAST_FILTER:
        g_value_set_boolean(val,
                priv->info.filter & SNDRV_SEQ_FILTER_BROADCAST);
        break;
    case SEQ_CLIENT_PROP_ERROR_BOUNCE:
        g_value_set_boolean(val,
                priv->info.filter & SNDRV_SEQ_FILTER_BOUNCE);
        break;
    /* pool information */
    case SEQ_CLIENT_PROP_OUTPUT_POOL:
        g_value_set_int(val, priv->pool.output_pool);
        break;
    case SEQ_CLIENT_PROP_INPUT_POOL:
        g_value_set_int(val, priv->pool.input_pool);
        break;
    case SEQ_CLIENT_PROP_OUTPUT_ROOM:
        g_value_set_int(val, priv->pool.output_room);
        break;
    case SEQ_CLIENT_PROP_OUTPUT_FREE:
        g_value_set_int(val, priv->pool.output_free);
        break;
    case SEQ_CLIENT_PROP_INPUT_FREE:
        g_value_set_int(val, priv->pool.input_free);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static void seq_client_set_property(GObject *obj, guint id,
                                    const GValue *val, GParamSpec *spec)
{
    ALSASeqClient *self = ALSASEQ_CLIENT(obj);
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);

    switch (id) {
    case SEQ_CLIENT_PROP_NAME:
        strncpy(priv->info.name, g_value_get_string(val),
            sizeof(priv->info.name));
        break;
    case SEQ_CLIENT_PROP_BROADCAST_FILTER:
        if (g_value_get_boolean(val))
            priv->info.filter |= SNDRV_SEQ_FILTER_BROADCAST;
        else
            priv->info.filter &= ~SNDRV_SEQ_FILTER_BROADCAST;
        break;
    case SEQ_CLIENT_PROP_ERROR_BOUNCE:
        if (g_value_get_boolean(val))
            priv->info.filter |= SNDRV_SEQ_FILTER_BOUNCE;
        else
            priv->info.filter &= ~SNDRV_SEQ_FILTER_BOUNCE;
        break;
    /* pool information */
    case SEQ_CLIENT_PROP_OUTPUT_POOL:
        priv->pool.output_pool = g_value_get_int(val);
        break;
    case SEQ_CLIENT_PROP_INPUT_POOL:
        priv->pool.input_pool = g_value_get_int(val);
        break;
    case SEQ_CLIENT_PROP_OUTPUT_ROOM:
        priv->pool.output_pool = g_value_get_int(val);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
        break;
    }
}

static void seq_client_finalize(GObject *obj)
{
    ALSASeqClient *self = ALSASEQ_CLIENT(obj);
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);

    /* TODO: drop messages in my pool. */
    close(priv->fd);

    G_OBJECT_CLASS(alsaseq_client_parent_class)->finalize(obj);
}

static void alsaseq_client_class_init(ALSASeqClientClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    gobject_class->get_property = seq_client_get_property;
    gobject_class->set_property = seq_client_set_property;
    gobject_class->finalize = seq_client_finalize;

    seq_client_props[SEQ_CLIENT_PROP_NUMBER] =
        g_param_spec_uchar("number", "number",
                           "An identical number of this client, including "
                           "ALSASeqClientNumberEnum",
                           0, UCHAR_MAX,
                           0,
                           G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_TYPE] =
        g_param_spec_enum("type", "type",
                          "The type for this client, one of "
                          "ALSASeqClientTypeEnum",
                          ALSASEQ_TYPE_CLIENT_TYPE_ENUM,
                          ALSASEQ_CLIENT_TYPE_ENUM_NO_CLIENT,
                          G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_NAME] =
        g_param_spec_string("name", "name",
                            "The name for this client",
                            "client",
                            G_PARAM_READWRITE);
    seq_client_props[SEQ_CLIENT_PROP_PORTS] =
        g_param_spec_int("ports", "ports",
                         "The number of ports for this client",
                         0, INT_MAX,
                         0,
                         G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_LOST] =
        g_param_spec_int("lost", "lost",
                         "The number of events are lost.",
                         0, INT_MAX,
                         0,
                         G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_BROADCAST_FILTER] =
        g_param_spec_boolean("broadcast-filter", "broadcast-filter",
                             "Receive broadcast event or not",
                             FALSE,
                             G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_ERROR_BOUNCE] =
        g_param_spec_boolean("error-bounce", "error-bounce",
                             "Receive error bounce event or not",
                             FALSE,
                             G_PARAM_READABLE);
    seq_client_props[SEQ_CLIENT_PROP_OUTPUT_POOL] =
        g_param_spec_int("output-pool", "output-pool",
                         "The size of pool for output",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_client_props[SEQ_CLIENT_PROP_INPUT_POOL] =
        g_param_spec_int("input-pool", "input-pool",
                         "The size of pool for input",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_client_props[SEQ_CLIENT_PROP_OUTPUT_ROOM] =
        g_param_spec_int("output-room", "output-room",
                         "The size of room for output room",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_client_props[SEQ_CLIENT_PROP_OUTPUT_FREE] =
        g_param_spec_int("output-free", "output-free",
                         "The free size of pool for output",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);
    seq_client_props[SEQ_CLIENT_PROP_INPUT_FREE] =
        g_param_spec_int("input-free", "input-free",
                         "The free size of pool for input",
                         0, INT_MAX,
                         0,
                         G_PARAM_READWRITE);

    g_object_class_install_properties(gobject_class, SEQ_CLIENT_PROP_COUNT,
                                      seq_client_props);
}

static void alsaseq_client_init(ALSASeqClient *self)
{
    ALSASeqClientPrivate *priv;

    priv = alsaseq_client_get_instance_private(self);
    priv->ports = NULL;
    g_mutex_init(&priv->lock);
}

void alsaseq_client_open(ALSASeqClient *self, gchar *path, const gchar *name,
                         GError **exception)
{
    ALSASeqClientPrivate *priv;
    int id;

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    /* Always open duplex ports. */
    priv->fd = open(path, O_RDWR | O_NONBLOCK);
    if (priv->fd < 0) {
        raise(exception, errno);
        return;
    }

    /* Get client ID. */
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_CLIENT_ID, &id) < 0) {
        raise(exception, errno);
        return;
    }

    /* Get client info with name. */
    priv->info.client = id;
    strncpy(priv->info.name, name, sizeof(priv->info.name));
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, &priv->info) < 0) {
        raise(exception, errno);
        return;
    }

    /* Get client pool info. */
    priv->pool.client = id;
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, &priv->pool) < 0)
        raise(exception, errno);
}

/**
 * alsaseq_client_update
 * @self: A ##ALSASeqClient
 * @exception: A #GError
 *
 * After calling this, all properties are updated. When changing properties,
 * this should be called.
 */
void alsaseq_client_update(ALSASeqClient *self, GError **exception)
{
    ALSASeqClientPrivate *priv;

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    /* Set client info. */
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_SET_CLIENT_INFO, &priv->info) < 0) {
        raise(exception, errno);
        return;
    }

    /*
     * TODO: sound/core/seq/seq_clientmgr.c has a bug to initialize this
     * member...
     */
    priv->pool.client = priv->info.client;
    /* Set client pool info. */
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_SET_CLIENT_POOL, &priv->pool) < 0) {
        raise(exception, errno);
        return;
    }

    /* Get client info. */
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, &priv->info) < 0) {
        raise(exception, errno);
        return;
    }

    /*
     * TODO: sound/core/seq/seq_clientmgr.c has a bug to initialize this
     * member...
     */
    priv->pool.client = priv->info.client;
    /* Get client pool info. */
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, &priv->pool) < 0)
        raise(exception, errno);
}

static ALSASeqPort *add_port(ALSASeqClient *self,
                             struct snd_seq_port_info *info)
{
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);
    ALSASeqPort *port;

    port = g_object_new(ALSASEQ_TYPE_PORT,
                        "fd", priv->fd,
                        "name", info->name,
                        "client-number", info->addr.client,
                        "number", info->addr.port,
                        NULL);
    port->_client = g_object_ref(self);

    g_mutex_lock(&priv->lock);
    priv->ports = g_list_prepend(priv->ports, port);
    g_mutex_unlock(&priv->lock);

    return port;
}

/**
 * alsaseq_client_open_port:
 * @self: A #ALSASeqClient
 * @name: The name of new port
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSASeqPort
 */
ALSASeqPort *alsaseq_client_open_port(ALSASeqClient *self, const gchar *name,
                                      GError **exception)
{
    ALSASeqClientPrivate *priv;
    struct snd_seq_port_info info = {{0}};

    g_return_val_if_fail(ALSASEQ_IS_CLIENT(self), NULL);
    priv = alsaseq_client_get_instance_private(self);

    /* Add new port to this client. */
    info.addr.client = priv->info.client;
    strncpy(info.name, name, sizeof(info.name));
    if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_CREATE_PORT, &info) < 0) {
        raise(exception, errno);
        return NULL;
    }

    return add_port(self, &info);
}

/**
 * alsaseq_client_close_port:
 * @self: A #ALSASeqClient
 * @port: A #ALSASeqPort
 *
 */
void alsaseq_client_close_port(ALSASeqClient *self, ALSASeqPort *port)
{
    ALSASeqClientPrivate *priv;
    GList *entry;

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    g_mutex_lock(&priv->lock);
    for (entry = priv->ports; entry; entry = entry->next) {
        if (entry->data != port)
            continue;

        priv->ports = g_list_delete_link(priv->ports, entry);
        g_object_unref(port->_client);
    }
    g_mutex_unlock(&priv->lock);
}

/**
 * alsaseq_client_get_ports:
 * @self: A #ALSASeqClient
 * @ports: (element-type ALSASeqPort) (array) (out caller-allocates) (transfer container): port array in this client
 * @exception: A #GError
 */
void alsaseq_client_get_ports(ALSASeqClient *self, GArray *ports,
                              GError **exception)
{
    ALSASeqClientPrivate *priv;
    ALSASeqPort *port;
    struct snd_seq_port_info info = {{0}};

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    info.addr.client = priv->info.client;
    info.addr.port = -1;
    while (1) {
        if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_QUERY_NEXT_PORT,
              &info) < 0) {
            if (errno != ENOENT)
                raise(exception, errno);
            break;
        }

        port = add_port(self, &info);
        g_array_append_val(ports, port);
    }
}

static gboolean prepare_src(GSource *gsrc, gint *timeout)
{
    /* Use blocking poll(2) to save CPU usage. */
    *timeout = -1;

    /* This source is not ready, let's poll(2) */
    return FALSE;
}

static void write_messages(ALSASeqClientPrivate *priv)
{
    int len;

    /* Spin lock. */
    while (!g_mutex_trylock(&priv->write_lock))
        continue;

    do {
        len = write(priv->fd, priv->write_buf,
                BUFFER_SIZE - priv->writable_bytes);
        if (len <= 0)
            break;
        priv->writable_bytes -= len;
    } while (priv->writable_bytes < BUFFER_SIZE);

    g_mutex_unlock(&priv->write_lock);
}

static const char *const ev_names[] = {
    [SNDRV_SEQ_EVENT_SYSTEM]        = "system",
    [SNDRV_SEQ_EVENT_RESULT]        = "result",

    /* snd_seq_ev_note */
    [SNDRV_SEQ_EVENT_NOTE]          = "note",
    [SNDRV_SEQ_EVENT_NOTEON]        = "noteon",
    [SNDRV_SEQ_EVENT_NOTEOFF]       = "noteoff",
    [SNDRV_SEQ_EVENT_KEYPRESS]      = "keypress",

    /* snd_seq_ev_ctrl */
    [SNDRV_SEQ_EVENT_CONTROLLER]    = "controller",
    [SNDRV_SEQ_EVENT_PGMCHANGE]     = "pgmchange",
    [SNDRV_SEQ_EVENT_CHANPRESS]     = "chanpress",
    [SNDRV_SEQ_EVENT_PITCHBEND]     = "pitchbend",
    [SNDRV_SEQ_EVENT_CONTROL14]     = "control14",
    [SNDRV_SEQ_EVENT_NONREGPARAM]   = "nonregparam",
    [SNDRV_SEQ_EVENT_REGPARAM]      = "regparam",

    /* snd_seq_ev_ctrl */
    [SNDRV_SEQ_EVENT_SONGPOS]       = "songpos",
    [SNDRV_SEQ_EVENT_SONGSEL]       = "songsel",
    [SNDRV_SEQ_EVENT_QFRAME]        = "qframe",
    [SNDRV_SEQ_EVENT_TIMESIGN]      = "timesign",
    [SNDRV_SEQ_EVENT_KEYSIGN]       = "keysign",

    /* snd_seq_ev_queue_control */
    [SNDRV_SEQ_EVENT_START]         = "start",
    [SNDRV_SEQ_EVENT_CONTINUE]      = "continue",
    [SNDRV_SEQ_EVENT_STOP]          = "stop",
    [SNDRV_SEQ_EVENT_SETPOS_TICK]   = "setpos-tick",
    [SNDRV_SEQ_EVENT_SETPOS_TIME]   = "setpos-time",
    [SNDRV_SEQ_EVENT_TEMPO]         = "tempo",
    [SNDRV_SEQ_EVENT_CLOCK]         = "clock",
    [SNDRV_SEQ_EVENT_TICK]          = "tick",
    [SNDRV_SEQ_EVENT_QUEUE_SKEW]    = "queue-skew",

    /* no data */
    [SNDRV_SEQ_EVENT_TUNE_REQUEST]  = "tune-request",
    [SNDRV_SEQ_EVENT_RESET]         = "reset",
    [SNDRV_SEQ_EVENT_SENSING]       = "sensing",

    /* any data */
    [SNDRV_SEQ_EVENT_ECHO]          = "echo",
    [SNDRV_SEQ_EVENT_OSS]           = "oss",

    /* snd_seq_addr */
    [SNDRV_SEQ_EVENT_CLIENT_START]  = "client-start",
    [SNDRV_SEQ_EVENT_CLIENT_EXIT]   = "client-exit",
    [SNDRV_SEQ_EVENT_CLIENT_CHANGE] = "client-change",
    [SNDRV_SEQ_EVENT_PORT_START]    = "port-start",
    [SNDRV_SEQ_EVENT_PORT_EXIT]     = "port-exit",
    [SNDRV_SEQ_EVENT_PORT_CHANGE]   = "port-change",

    /* snd_seq_connect */
    [SNDRV_SEQ_EVENT_PORT_SUBSCRIBED]   = "port-subscribed",
    [SNDRV_SEQ_EVENT_PORT_UNSUBSCRIBED] = "port-unsubscribed",

    /* any data */
    [SNDRV_SEQ_EVENT_USR0]      = "user0",
    [SNDRV_SEQ_EVENT_USR1]      = "user1",
    [SNDRV_SEQ_EVENT_USR2]      = "user2",
    [SNDRV_SEQ_EVENT_USR3]      = "user3",
    [SNDRV_SEQ_EVENT_USR4]      = "user4",
    [SNDRV_SEQ_EVENT_USR5]      = "user5",
    [SNDRV_SEQ_EVENT_USR6]      = "user6",
    [SNDRV_SEQ_EVENT_USR7]      = "user7",
    [SNDRV_SEQ_EVENT_USR8]      = "user8",
    [SNDRV_SEQ_EVENT_USR9]      = "user9",

    /* snd_seq_ev_ext */
    [SNDRV_SEQ_EVENT_SYSEX]     = "sysex",
    [SNDRV_SEQ_EVENT_BOUNCE]    = "bounce",

    [SNDRV_SEQ_EVENT_USR_VAR0]  = "user-var0",
    [SNDRV_SEQ_EVENT_USR_VAR1]  = "user-var1",
    [SNDRV_SEQ_EVENT_USR_VAR2]  = "user-var2",
    [SNDRV_SEQ_EVENT_USR_VAR3]  = "user-var3",
    [SNDRV_SEQ_EVENT_USR_VAR4]  = "user-var4",

    [SNDRV_SEQ_EVENT_NONE]      = "none",
};

static void deliver_event(ALSASeqPort *port, struct snd_seq_event *ev)
{
    switch (ev->type) {
    default:
        /* TODO: delivery data */
        g_signal_emit_by_name(G_OBJECT(port), "event",
                              ev_names[ev->type], ev->flags, ev->tag,
                              ev->queue, ev->time.time.tv_sec,
                              ev->time.time.tv_nsec,
                              ev->source.client, ev->source.port,
                              NULL);
        break;
    }
}

static void read_messages(ALSASeqClientPrivate *priv)
{
    int len, i;

    GList *entry;
    ALSASeqPort *port;
    GValue val = G_VALUE_INIT;

    g_value_init(&val, G_TYPE_INT);
    len = read(priv->fd, &priv->read_ev, sizeof(priv->read_ev));
    if (len <= 0)
        return;
    len /= sizeof(struct snd_seq_event);

    g_mutex_lock(&priv->lock);
    for (i = 0; i < len; i++) {
        for (entry = priv->ports; entry; entry = entry->next) {
            port = (ALSASeqPort *)entry->data;

            g_object_get_property(G_OBJECT(port), "number", &val);
            if (priv->read_ev[i].dest.port != g_value_get_int(&val))
                continue;

            deliver_event(port, &priv->read_ev[i]);
        }
    }
    g_mutex_unlock(&priv->lock);
}

static gboolean check_src(GSource *gsrc)
{
    SeqClientSource *src = (SeqClientSource *)gsrc;
    GIOCondition condition;

    ALSASeqClient *self = src->self;
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);

    condition = g_source_query_unix_fd((GSource *)src, src->tag);

    if (condition & G_IO_ERR)
        alsaseq_client_unlisten(self);
    if (condition & G_IO_OUT)
        write_messages(priv);
    if (condition & G_IO_IN)
        read_messages(priv);

    return FALSE;
}

static gboolean dispatch_src(GSource *gsrc, GSourceFunc callback,
                             gpointer user_data)
{
    SeqClientSource *src = (SeqClientSource *)gsrc;
    ALSASeqClient *self = src->self;
    ALSASeqClientPrivate *priv = alsaseq_client_get_instance_private(self);
    GIOCondition condition;

    /* Decide next event to wait for. */
    condition = G_IO_IN;
    if (priv->writable_bytes != BUFFER_SIZE)
        condition |= G_IO_OUT;
    g_source_modify_unix_fd(gsrc, src->tag, condition);

    /* Just be sure to continue to process this source. */
    return TRUE;
}

static void finalize_src(GSource *gsrc)
{
    /* Do nothing paticular. */
    return;
}

void alsaseq_client_listen(ALSASeqClient *self, GError **exception)
{
    ALSASeqClientPrivate *priv;

    static GSourceFuncs funcs = {
        .prepare    = prepare_src,
        .check        = check_src,
        .dispatch    = dispatch_src,
        .finalize    = finalize_src,
    };
    GSource *src;

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    priv->write_buf = g_malloc(BUFFER_SIZE);
    if (!priv->write_buf) {
        raise(exception, ENOMEM);
        return;
    }

    /* Create a source. */
    src = g_source_new(&funcs, sizeof(SeqClientSource));
    if (!src) {
        raise(exception, ENOMEM);
        g_free(priv->write_buf);
        priv->write_buf = NULL;
        return;
    }
    g_source_set_name(src, "ALSASeqClient");
    g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
    g_source_set_can_recurse(src, TRUE);
    ((SeqClientSource *)src)->self = self;
    priv->src = (SeqClientSource *)src;

    priv->writable_bytes = BUFFER_SIZE;

    /* Attach the source to context. */
    g_source_attach(src, g_main_context_default());
    ((SeqClientSource *)src)->tag =
            g_source_add_unix_fd(src, priv->fd, G_IO_IN);
}

void alsaseq_client_unlisten(ALSASeqClient *self)
{
    ALSASeqClientPrivate *priv;

    g_return_if_fail(ALSASEQ_IS_CLIENT(self));
    priv = alsaseq_client_get_instance_private(self);

    if (!priv->src)
        return;

    g_source_destroy((GSource *)priv->src);
    g_free(priv->src);
    priv->src = NULL;

    g_free(priv->write_buf);
}
