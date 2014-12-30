#include <alsa/asoundlib.h>
#include "client.h"
#include "alsatimer_sigs_marshal.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSATimerClientPrivate {
	snd_timer_info_t *info;
	snd_timer_params_t *params;
	snd_timer_t *handle;
};

G_DEFINE_TYPE_WITH_PRIVATE (ALSATimerClient, alsatimer_client, G_TYPE_OBJECT)
#define TIMER_CLIENT_GET_PRIVATE(obj)					\
        (G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSATIMER_TYPE_CLIENT,		\
				     ALSATimerClientPrivate))

enum timer_client_prop {
	/* snd_timer_info_t */
	TIMER_CLIENT_PROP_ID = 1,
	TIMER_CLIENT_PROP_NAME,
	TIMER_CLIENT_PROP_IS_SLAVE,
	TIMER_CLIENT_PROP_CARD,
	TIMER_CLIENT_PROP_RESOLUTION,
	/* snd_timer_params_t */
	TIMER_CLIENT_PROP_AUTO_START,
	TIMER_CLIENT_PROP_EXCLUSIVE,
	TIMER_CLIENT_PROP_EARLY_EVENT,
	TIMER_CLIENT_PROP_TICKS,
	TIMER_CLIENT_PROP_QUEUE_SIZE,
	TIMER_CLIENT_PROP_FILTER,
	TIMER_CLIENT_PROP_COUNT,
};
static GParamSpec *timer_client_props[TIMER_CLIENT_PROP_COUNT] = { NULL, };

enum timer_client_signal {
	TIMER_CLIENT_SIGNAL_EVENT = 1,
	TIMER_CLIENT_SIGNAL_COUNT,
};

static guint timer_client_signals[TIMER_CLIENT_SIGNAL_COUNT] = { 0 };

static void alsatimer_client_get_property(GObject *obj, guint id,
					GValue *val, GParamSpec *spec)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(obj);
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(obj);

	switch (id) {
	case TIMER_CLIENT_PROP_ID:
		g_value_set_string(val, snd_timer_info_get_id(priv->info));
		break;
	case TIMER_CLIENT_PROP_NAME:
		g_value_set_string(val, snd_timer_info_get_name(priv->info));
		break;
	case TIMER_CLIENT_PROP_IS_SLAVE:
		g_value_set_boolean(val, snd_timer_info_is_slave(priv->info));
		break;
	case TIMER_CLIENT_PROP_CARD:
		g_value_set_int(val, snd_timer_info_get_card(priv->info));
		break;
	case TIMER_CLIENT_PROP_RESOLUTION:
		g_value_set_long(val,
				 snd_timer_info_get_resolution(priv->info));
		break;
	case TIMER_CLIENT_PROP_AUTO_START:
		g_value_set_boolean(val,
				snd_timer_params_get_auto_start(priv->params));
		break;
	case TIMER_CLIENT_PROP_EXCLUSIVE:
		g_value_set_boolean(val,
				snd_timer_params_get_exclusive(priv->params));
		break;
	case TIMER_CLIENT_PROP_EARLY_EVENT:
		g_value_set_boolean(val,
				snd_timer_params_get_early_event(priv->params));
		break;
	case TIMER_CLIENT_PROP_TICKS:
		g_value_set_long(val,
				 snd_timer_params_get_ticks(priv->params));
		break;
	case TIMER_CLIENT_PROP_QUEUE_SIZE:
		g_value_set_long(val,
				 snd_timer_params_get_queue_size(priv->params));
		break;
	case TIMER_CLIENT_PROP_FILTER:
		g_value_set_uint(val,
				 snd_timer_params_get_filter(priv->params));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		break;
	}
}

static void alsatimer_client_set_property(GObject *obj, guint id,
					const GValue *val, GParamSpec *spec)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(obj);
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(obj);

	switch(id) {
	case TIMER_CLIENT_PROP_AUTO_START:
		snd_timer_params_set_auto_start(priv->params,
						g_value_get_boolean(val));
		break;
	case TIMER_CLIENT_PROP_EXCLUSIVE:
		snd_timer_params_set_exclusive(priv->params,
					       g_value_get_boolean(val));
		break;
	case TIMER_CLIENT_PROP_EARLY_EVENT:
		snd_timer_params_set_early_event(priv->params,
						 g_value_get_boolean(val));
		break;
	case TIMER_CLIENT_PROP_TICKS:
		snd_timer_params_set_ticks(priv->params,
					   g_value_get_long(val));
		break;
	case TIMER_CLIENT_PROP_QUEUE_SIZE:
		snd_timer_params_set_queue_size(priv->params,
						g_value_get_long(val));
		break;
	case TIMER_CLIENT_PROP_FILTER:
		snd_timer_params_set_filter(priv->params,
					    g_value_get_uint(val));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void alsatimer_client_dispose(GObject *gobject)
{
	G_OBJECT_CLASS(alsatimer_client_parent_class)->dispose(gobject);
}

static void alsatimer_client_finalize(GObject *gobject)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(gobject);
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(self);

	snd_timer_info_free(priv->info);
	snd_timer_close(priv->handle);

	G_OBJECT_CLASS(alsatimer_client_parent_class)->finalize(gobject);
}

static void alsatimer_client_class_init(ALSATimerClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->get_property = alsatimer_client_get_property;
	gobject_class->set_property = alsatimer_client_set_property;
	gobject_class->dispose = alsatimer_client_dispose;
	gobject_class->finalize = alsatimer_client_finalize;

	timer_client_props[TIMER_CLIENT_PROP_ID] = 
		g_param_spec_string("id", "id",
				    "The id of timer for this client",
				    NULL,
				    G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_NAME] = 
		g_param_spec_string("name", "name",
				    "The name of timer for this client",
				    NULL,
				    G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_IS_SLAVE] = 
		g_param_spec_boolean("is-slave", "is-slave",
			"Whether the timer of this client is slave or not",
				     FALSE,
				     G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_CARD] = 
		g_param_spec_int("card", "card",
				 "The card number of timer for this client",
				 -1, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_RESOLUTION] = 
		g_param_spec_long("resolution", "resolution",
				  "The resolution of timer for this client",
				  -1, LONG_MAX,
				  0,
				 G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_AUTO_START] = 
		g_param_spec_boolean("auto-start", "auto-start",
				     "Automatically restart or not",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_EXCLUSIVE] = 
		g_param_spec_boolean("exclusive", "exclusive",
				     "Exclusively keep the timer",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_EARLY_EVENT] = 
		g_param_spec_boolean("early-event", "early-event",
				     "Generate initial event or not",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_TICKS] = 
		g_param_spec_long("ticks", "ticks",
				  "The number of ticks for this client",
				  0, LONG_MAX,
				  1,
				  G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_QUEUE_SIZE] = 
		g_param_spec_long("queue-size", "queue-size",
				  "The size of event queue",
				  0, LONG_MAX,
				  128,
				  G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_FILTER] = 
		g_param_spec_uint("filter", "filter",
				  "The bit flag for event filter",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE);

	timer_client_signals[TIMER_CLIENT_SIGNAL_EVENT] =
		g_signal_new("event",
		     G_OBJECT_CLASS_TYPE(klass),
		     G_SIGNAL_RUN_LAST,
		     0,
		     NULL, NULL,
		     alsatimer_sigs_marshal_VOID__INT_LONG_LONG_UINT,
		     G_TYPE_NONE, 4,
		     G_TYPE_INT, G_TYPE_LONG, G_TYPE_LONG, G_TYPE_UINT);

	g_object_class_install_properties(gobject_class,
					  TIMER_CLIENT_PROP_COUNT,
					  timer_client_props);
}

static void
alsatimer_client_init(ALSATimerClient *self)
{
	self->priv = alsatimer_client_get_instance_private(self);
}

ALSATimerClient *alsatimer_client_new(gchar *timer, GError **exception)
{
	ALSATimerClient *self;
	ALSATimerClientPrivate *priv;

	snd_timer_t *handle;
	snd_timer_info_t *info;
	snd_timer_params_t *params;

	int err;

	/* NOTE: Open with enhanced event notification. */
	err = snd_timer_open(&handle, timer, SND_TIMER_OPEN_TREAD);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	err = snd_timer_info_malloc(&info);
	if (err < 0) {
		snd_timer_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	err = snd_timer_params_malloc(&params);
	if (err < 0) {
		snd_timer_params_free(params);
		snd_timer_info_free(info);
		snd_timer_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	err = snd_timer_info(handle, info);
	if (err < 0) {
		snd_timer_params_free(params);
		snd_timer_info_free(info);
		snd_timer_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	snd_timer_params_set_ticks(params, 1);
	snd_timer_params_set_queue_size(params, 128);
	err = snd_timer_params(handle, params);
	if (err < 0) {
		snd_timer_params_free(params);
		snd_timer_info_free(info);
		snd_timer_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	self =  g_object_new(ALSATIMER_TYPE_CLIENT, NULL);
	if (self == NULL) {
		snd_timer_params_free(params);
		snd_timer_info_free(info);
		snd_timer_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", snd_strerror(ENOMEM));
		return NULL;
	}
	priv = TIMER_CLIENT_GET_PRIVATE(self);

	priv->info = info;
	priv->params = params;
	priv->handle = handle;

	return self;
}

/**
 * alsatimer_client_get_status:
 * @self: A #ALSATimerClient
 * @status: (element-type uint) (array) (out caller-allocates):
 *		A #GArray for status
 * @exception: A #GError
 */
void alsatimer_client_get_status(ALSATimerClient *self, GArray *status,
				 GError **exception)
{
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(self);

	snd_htimestamp_t ts = {0};
	snd_timer_status_t *s;
	long val;
	int err;

	snd_timer_status_alloca(&s);
	err = snd_timer_status(priv->handle, s);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}

	/* NOTE: narrow-conversions are expected... */
	ts = snd_timer_status_get_timestamp(s);
	g_array_append_val(status, ts.tv_sec);
	g_array_append_val(status, ts.tv_nsec);
	val = snd_timer_status_get_lost(s);
	g_array_append_val(status, val);
	val = snd_timer_status_get_overrun(s);
	g_array_append_val(status, val);
	val = snd_timer_status_get_queue(s);
	g_array_append_val(status, val);
}

void alsatimer_client_start(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(self);
	int err;

	err = snd_timer_params(priv->handle, priv->params);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}

	/* start listening */

	err = snd_timer_start(priv->handle);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}

void alsatimer_client_stop(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(self);
	int err;

	/* TODO: temporary */
	g_signal_emit(self, timer_client_signals[TIMER_CLIENT_SIGNAL_EVENT], 0,
		      0, 0, 0, 0);

	/* stop listening */

	err = snd_timer_stop(priv->handle);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}

void alsatimer_client_resume(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv = TIMER_CLIENT_GET_PRIVATE(self);
	int err;

	err = snd_timer_params(priv->handle, priv->params);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}
	/* start listening */

	err = snd_timer_continue(priv->handle);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}
