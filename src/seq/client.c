#include <poll.h>
#include <alsa/asoundlib.h>
#include "client.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

typedef struct {
	GSource src;
	ALSASeqClient *self;
	gpointer tag;
} SeqClientSource;

struct _ALSASeqClientPrivate {
	snd_seq_client_info_t *info;
	snd_seq_client_pool_t *pool;

	SeqClientSource *src;

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
	/* Buffer parameters */
	SEQ_CLIENT_PROP_OUTPUT_BUFFER,
	SEQ_CLIENT_PROP_INPUT_BUFFER,
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
		g_value_set_int(val,
				snd_seq_client_info_get_client(priv->info));
		break;
	case SEQ_CLIENT_PROP_TYPE:
		g_value_set_int(val,
				snd_seq_client_info_get_type(priv->info));
		break;
	case SEQ_CLIENT_PROP_NAME:
		g_value_set_string(val,
				   snd_seq_client_info_get_name(priv->info));
		break;
	case SEQ_CLIENT_PROP_PORTS:
		g_value_set_int(val,
				snd_seq_client_info_get_num_ports(priv->info));
		break;
	case SEQ_CLIENT_PROP_LOST:
		g_value_set_int(val,
				snd_seq_client_info_get_event_lost(priv->info));
		break;
	case SEQ_CLIENT_PROP_BROADCAST_FILTER:
		g_value_set_boolean(val,
			snd_seq_client_info_get_broadcast_filter(priv->info));
		break;
	case SEQ_CLIENT_PROP_ERROR_BOUNCE:
		g_value_set_boolean(val,
			snd_seq_client_info_get_error_bounce(priv->info));
		break;
	case SEQ_CLIENT_PROP_OUTPUT_BUFFER:
		g_value_set_int(val,
				snd_seq_get_output_buffer_size(self->handle));
		break;
	case SEQ_CLIENT_PROP_INPUT_BUFFER:
		g_value_set_int(val,
				snd_seq_get_input_buffer_size(self->handle));
		break;
	/* pool information */
	case SEQ_CLIENT_PROP_OUTPUT_POOL:
		g_value_set_int(val,
			snd_seq_client_pool_get_output_pool(priv->pool));
		break;
	case SEQ_CLIENT_PROP_INPUT_POOL:
		g_value_set_int(val,
				snd_seq_client_pool_get_input_pool(priv->pool));
		break;
	case SEQ_CLIENT_PROP_OUTPUT_ROOM:
		g_value_set_int(val,
			snd_seq_client_pool_get_output_room(priv->pool));
		break;
	case SEQ_CLIENT_PROP_OUTPUT_FREE:
		g_value_set_int(val,
			snd_seq_client_pool_get_output_free(priv->pool));
		break;
	case SEQ_CLIENT_PROP_INPUT_FREE:
		g_value_set_int(val,
			snd_seq_client_pool_get_input_free(priv->pool));
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
		snd_seq_client_info_set_name(priv->info,
					     g_value_get_string(val));
		break;
	case SEQ_CLIENT_PROP_BROADCAST_FILTER:
		snd_seq_client_info_set_broadcast_filter(priv->info,
						g_value_get_boolean(val));
		break;
	case SEQ_CLIENT_PROP_ERROR_BOUNCE:
		snd_seq_client_info_set_error_bounce(priv->info,
						g_value_get_boolean(val));
		break;
	case SEQ_CLIENT_PROP_OUTPUT_BUFFER:
		snd_seq_set_output_buffer_size(self->handle,
					       g_value_get_int(val));
		break;
	case SEQ_CLIENT_PROP_INPUT_BUFFER:
		snd_seq_set_input_buffer_size(self->handle,
					      g_value_get_int(val));
		break;
	/* pool information */
	case SEQ_CLIENT_PROP_OUTPUT_POOL:
		snd_seq_client_pool_set_output_pool(priv->pool,
						    g_value_get_int(val));
		break;
	case SEQ_CLIENT_PROP_INPUT_POOL:
		snd_seq_client_pool_set_input_pool(priv->pool,
						   g_value_get_int(val));
		break;
	case SEQ_CLIENT_PROP_OUTPUT_ROOM:
		snd_seq_client_pool_set_output_room(priv->pool,
						    g_value_get_int(val));
		break;
	case SEQ_CLIENT_PROP_ID:
	case SEQ_CLIENT_PROP_TYPE:
	case SEQ_CLIENT_PROP_PORTS:
	case SEQ_CLIENT_PROP_LOST:
	case SEQ_CLIENT_PROP_OUTPUT_FREE:
	case SEQ_CLIENT_PROP_INPUT_FREE:
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

	snd_seq_client_pool_free(priv->pool);
	snd_seq_client_info_free(priv->info);
	snd_seq_close(self->handle);

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
	seq_client_props[SEQ_CLIENT_PROP_OUTPUT_BUFFER] =
		g_param_spec_int("output-buffer", "output-buffer",
				 "The size of buffer for output",
				 0, INT_MAX,
				 0,
				 G_PARAM_READWRITE);
	seq_client_props[SEQ_CLIENT_PROP_INPUT_BUFFER] =
		g_param_spec_int("input-buffer", "output-buffer",
				 "The size of buffer for input",
				 0, INT_MAX,
				 0,
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
}

ALSASeqClient *alsaseq_client_new(gchar *seq, const gchar *name,
				  GError **exception)
{
	ALSASeqClient *self;
	ALSASeqClientPrivate *priv;

	snd_seq_client_info_t *info = NULL;
	snd_seq_client_pool_t *pool = NULL;
	snd_seq_t *handle = NULL;
	int err;

	/* Always open duplex ports. */
	err = snd_seq_open(&handle, seq, SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0)
		goto error;

	/* Retrieve client information. */
	err = snd_seq_client_info_malloc(&info);
	if (err < 0)
		goto error;
	err = snd_seq_get_client_info(handle, info);
	if (err < 0)
		goto error;
	snd_seq_client_info_set_name(info, name);
	err = snd_seq_set_client_info(handle, info);
	if (err < 0)
		goto error;

	/* Retrieve client pool information. */
	err = snd_seq_client_pool_malloc(&pool);
	if (err < 0)
		goto error;
	err = snd_seq_get_client_pool(handle, pool);
	if (err < 0)
		goto error;

	/* Gain new object. */
	self = g_object_new(ALSASEQ_TYPE_CLIENT, NULL);
	if (self == NULL) {
		err = -ENOMEM;
		goto error;
	}
	priv = SEQ_CLIENT_GET_PRIVATE(self);

	priv->pool = pool;
	priv->info = info;
	self->handle = handle;
	priv->ports = NULL;
	g_mutex_init(&priv->lock);

	return self;
error:
	if (pool != NULL)
		snd_seq_client_pool_free(pool);
	if (info != NULL)
		snd_seq_client_info_free(info);
	if (handle != NULL)
		snd_seq_close(handle);
	g_set_error(exception, g_quark_from_static_string(__func__),
		    -err, "%s", snd_strerror(err));
	return NULL;
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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);
	int err;

	err = snd_seq_set_client_info(self->handle, priv->info);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}

	err = snd_seq_set_client_pool(self->handle, priv->pool);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);
	ALSASeqPort *port;

	port = alsaseq_port_new(self, name, exception);
	if (*exception != NULL)
		return NULL;

	/* TODO: when should we remove this entry? */
	g_mutex_lock(&priv->lock);
	priv->ports = g_list_prepend(priv->ports, port);
	g_mutex_unlock(&priv->lock);

	return port;
}

static gboolean prepare_src(GSource *gsrc, gint *timeout)
{
	SeqClientSource *src = (SeqClientSource *)gsrc;
	ALSASeqClient *self = src->self;

	/* Set 2msec for poll(2) timeout if need to output. */
	if (snd_seq_event_output_pending(self->handle) > 0)
		*timeout = 2;
	else
		*timeout = -1;

	/* This source is not ready, let's poll(2) */
	return FALSE;
}

static gboolean check_src(GSource *gsrc)
{
	SeqClientSource *src = (SeqClientSource *)gsrc;
	GIOCondition condition;

	ALSASeqClient *self = src->self;
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

	snd_seq_event_t *ev;
	GList *entry;
	ALSASeqPort *port;
	GValue val = G_VALUE_INIT;

	condition = g_source_query_unix_fd((GSource *)src, src->tag);

	if ((condition & G_IO_OUT) &&
	    (snd_seq_event_output_pending(self->handle) > 0))
		snd_seq_drain_output(self->handle);

	if (!(condition & G_IO_IN))
		goto end;

	g_value_init(&val, G_TYPE_INT);
	g_mutex_lock(&priv->lock);
	do {
		if (snd_seq_event_input(self->handle, &ev) < 0)
			break;

		for (entry = priv->ports; entry != NULL; entry = entry->next) {
			port = (ALSASeqPort *)entry->data;

			g_object_get_property(G_OBJECT(port), "id", &val);
			if (ev->dest.port != g_value_get_int(&val))
				continue;

			/* TODO: data */
			g_signal_emit_by_name(G_OBJECT(port), "event",
				ev->type, ev->flags, ev->tag, ev->queue,
				ev->time.time.tv_sec, ev->time.time.tv_nsec,
				ev->source.client, ev->source.port,
				NULL);
		}
	} while (snd_seq_event_input_pending(self->handle, 0) > 0);
	g_mutex_unlock(&priv->lock);
end:
	return FALSE;
}

static gboolean dispatch_src(GSource *gsrc, GSourceFunc callback,
			     gpointer user_data)
{
	SeqClientSource *src = (SeqClientSource *)gsrc;
	ALSASeqClient *self = src->self;
	GIOCondition condition;

	/* Decide next event to wait for. */
	condition = G_IO_IN;
	if (snd_seq_event_output_pending(self->handle) > 0)
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
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);
	struct pollfd pfds;

	static GSourceFuncs funcs = {
		.prepare	= prepare_src,
		.check		= check_src,
		.dispatch	= dispatch_src,
		.finalize	= finalize_src,
	};
	GSource *src;
	GMainContext *ctx;

	/* Create a source. */
	src = g_source_new(&funcs, sizeof(SeqClientSource));
	if (src == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", strerror(ENOMEM));
		return;
	}
	g_source_set_name(src, "ALSASeqClient");
	g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
	g_source_set_can_recurse(src, TRUE);
	((SeqClientSource *)src)->self = self;
	priv->src = (SeqClientSource *)src;

	/* Attach the source to context. */
	g_source_attach(src, g_main_context_default());
	snd_seq_poll_descriptors(self->handle, &pfds, 1, POLLIN | POLLOUT);
	((SeqClientSource *)src)->tag =
			g_source_add_unix_fd(src, pfds.fd, G_IO_IN);
}

void alsaseq_client_unlisten(ALSASeqClient *self, GError **exception)
{
	ALSASeqClientPrivate *priv = SEQ_CLIENT_GET_PRIVATE(self);

	g_source_destroy((GSource *)priv->src);
	g_free(priv->src);
	priv->src = NULL;
}
