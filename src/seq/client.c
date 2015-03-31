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

#define BUFFER_SIZE	1024

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

	struct snd_seq_event read_ev;

	GList *ports;
	GMutex lock;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSASeqClient, alsaseq_client, G_TYPE_OBJECT)
#define SEQ_CLIENT_GET_PRIVATE(obj)					\
        (G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSASEQ_TYPE_CLIENT, ALSASeqClientPrivate))

/* TODO: Event filter. */

enum seq_client_prop {
	/* Client information */
	SEQ_CLIENT_PROP_ID = 1,
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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

	switch (id) {
	/* client information */
	case SEQ_CLIENT_PROP_ID:
		g_value_set_int(val, priv->info.client);
		break;
	case SEQ_CLIENT_PROP_TYPE:
		g_value_set_int(val, priv->info.type);
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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

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

static void seq_client_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(alsaseq_client_parent_class)->dispose(gobject);
}

static void seq_client_finalize(GObject *gobject)
{
	ALSASeqClient *self = ALSASEQ_CLIENT(gobject);
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

	close(priv->fd);

	G_OBJECT_CLASS(alsaseq_client_parent_class)->finalize(gobject);
}

static void alsaseq_client_class_init(ALSASeqClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = seq_client_get_property;
	gobject_class->set_property = seq_client_set_property;
	gobject_class->dispose = seq_client_dispose;
	gobject_class->finalize = seq_client_finalize;

	seq_client_props[SEQ_CLIENT_PROP_ID] =
		g_param_spec_int("id", "id",
				 "The id for this client",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	seq_client_props[SEQ_CLIENT_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type for this client",
				 0, INT_MAX,
				 0,
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
	self->priv = alsaseq_client_get_instance_private(self);
	self->priv->ports = NULL;
	g_mutex_init(&self->priv->lock);
}

void alsaseq_client_open(ALSASeqClient *self, gchar *path, const gchar *name,
			 GError **exception)
{
	ALSASeqClientPrivate *priv;
	int id;

	g_return_if_fail(ALSASEQ_IS_CLIENT(self));
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	/* Always open duplex ports. */
	priv->fd = open(path, O_RDWR);
	if (priv->fd < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/* Get client ID. */
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_CLIENT_ID, &id) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/* Get client info with name. */
	priv->info.client = id;
	strncpy(priv->info.name, name, sizeof(priv->info.name));
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, &priv->info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/* Get client pool info. */
	priv->pool.client = id;
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, &priv->pool) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
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
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	/* Set client info. */
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_SET_CLIENT_INFO, &priv->info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/*
	 * TODO: sound/core/seq/seq_clientmgr.c has a bug to initialize this
	 * member...
	 */
	priv->pool.client = priv->info.client;
	/* Set client pool info. */
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_SET_CLIENT_POOL, &priv->pool) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/* Get client info. */
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_INFO, &priv->info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return;
	}

	/*
	 * TODO: sound/core/seq/seq_clientmgr.c has a bug to initialize this
	 * member...
	 */
	priv->pool.client = priv->info.client;
	/* Get client pool info. */
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_GET_CLIENT_POOL, &priv->pool) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
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
	ALSASeqPort *port;
	struct snd_seq_port_info info = {{0}};

	g_return_if_fail(ALSASEQ_IS_CLIENT(self));
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	/* Add new port to this client. */
	info.addr.client = priv->info.client;
	strncpy(info.name, name, sizeof(info.name));
	if (ioctl(priv->fd, SNDRV_SEQ_IOCTL_CREATE_PORT, &info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		return NULL;
	}

	port = g_object_new(ALSASEQ_TYPE_PORT,
			    "fd", priv->fd,
			    "name", name,
			    "client", info.addr.client,
			    "id", info.addr.port,
			    NULL);
	port->_client = g_object_ref(self);

	/* TODO: when should we remove this entry? */
	g_mutex_lock(&priv->lock);
	priv->ports = g_list_prepend(priv->ports, port);
	g_mutex_unlock(&priv->lock);

	return port;
}

void alsaseq_client_close_port(ALSASeqClient *self, ALSASeqPort *port)
{
	ALSASeqClientPrivate *priv;
	GList *entry;

	g_return_if_fail(ALSASEQ_IS_CLIENT(self));
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	g_mutex_lock(&priv->lock);
	for (entry = priv->ports; entry != NULL; entry = entry->next) {
		if (entry->data != port)
			continue;

		priv->ports = g_list_delete_link(priv->ports, entry);
		g_object_unref(port->_client);
	}
	g_mutex_unlock(&priv->lock);
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
	} while (priv->writable_bytes == 0);

	g_mutex_unlock(&priv->write_lock);
}

static void read_messages(ALSASeqClientPrivate *priv)
{
	struct snd_seq_event *ev = &priv->read_ev;
	int len;

	GList *entry;
	ALSASeqPort *port;
	GValue val = G_VALUE_INIT;

	g_value_init(&val, G_TYPE_INT);
	g_mutex_lock(&priv->lock);
	do {
		len = read(priv->fd, ev, sizeof(struct snd_seq_event));
		if (len <= 0)
			break;

		for (entry = priv->ports; entry != NULL; entry = entry->next) {
			port = (ALSASeqPort *)entry->data;

			g_object_get_property(G_OBJECT(port), "id", &val);
			if (ev->dest.port != g_value_get_int(&val))
				continue;

			/* TODO: delivery data */
			g_signal_emit_by_name(G_OBJECT(port), "event",
				ev->type, ev->flags, ev->tag, ev->queue,
				ev->time.time.tv_sec, ev->time.time.tv_nsec,
				ev->source.client, ev->source.port,
				NULL);
		}
	} while (1);
	g_mutex_unlock(&priv->lock);
}

static gboolean check_src(GSource *gsrc)
{
	SeqClientSource *src = (SeqClientSource *)gsrc;
	GIOCondition condition;

	ALSASeqClient *self = src->self;
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);
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
		.prepare	= prepare_src,
		.check		= check_src,
		.dispatch	= dispatch_src,
		.finalize	= finalize_src,
	};
	GSource *src;

	g_return_if_fail(ALSASEQ_IS_CLIENT(self));
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	priv->write_buf = g_malloc(BUFFER_SIZE);
	if (priv->write_buf == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", strerror(ENOMEM));
		return;
	}

	/* Create a source. */
	src = g_source_new(&funcs, sizeof(SeqClientSource));
	if (src == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", strerror(ENOMEM));
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
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	g_source_destroy((GSource *)priv->src);
	g_free(priv->src);
	priv->src = NULL;

	g_free(priv->write_buf);
}
