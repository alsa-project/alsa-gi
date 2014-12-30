#include <alsa/asoundlib.h>
#include "query.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSATimerQueryPrivate {
	snd_timer_id_t *id;
	snd_timer_query_t *handle;
	snd_timer_ginfo_t *info;
};
G_DEFINE_TYPE_WITH_PRIVATE (ALSATimerQuery, alsatimer_query, G_TYPE_OBJECT)
#define TIMER_QUERY_GET_PRIVATE(obj)					\
        (G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSA_TIMER_TYPE_QUERY,		\
				     ALSATimerQueryPrivate))

enum timer_query_prop {
	TIMER_QUERY_PROP_ID = 1,
	TIMER_QUERY_PROP_NAME,
	TIMER_QUERY_PROP_CLASS,
	TIMER_QUERY_PROP_SUB_CLASS,
	TIMER_QUERY_PROP_CARD,
	TIMER_QUERY_PROP_DEVICE,
	TIMER_QUERY_PROP_SUB_DEVICE,
	TIMER_QUERY_PROP_COUNT,
};

static GParamSpec *timer_query_props[TIMER_QUERY_PROP_COUNT] = { NULL, };

static void alsatimer_query_get_property(GObject *obj, guint id,
					 GValue *val, GParamSpec *spec)
{
	ALSATimerQuery *self = ALSA_TIMER_QUERY(obj);
	ALSATimerQueryPrivate *priv = TIMER_QUERY_GET_PRIVATE(self);

	switch (id) {
	case TIMER_QUERY_PROP_ID:
		g_value_set_string(val, snd_timer_ginfo_get_id(priv->info));
		break;
	case TIMER_QUERY_PROP_NAME:
		g_value_set_string(val, snd_timer_ginfo_get_name(priv->info));
		break;
	case TIMER_QUERY_PROP_CLASS:
		g_value_set_int(val, snd_timer_id_get_class(priv->id));
		break;
	case TIMER_QUERY_PROP_SUB_CLASS:
		g_value_set_int(val, snd_timer_id_get_sclass(priv->id));
		break;
	case TIMER_QUERY_PROP_CARD:
		g_value_set_int(val, snd_timer_id_get_card(priv->id));
		break;
	case TIMER_QUERY_PROP_DEVICE:
		g_value_set_int(val, snd_timer_id_get_device(priv->id));
		break;
	case TIMER_QUERY_PROP_SUB_DEVICE:
		g_value_set_int(val, snd_timer_id_get_subdevice(priv->id));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		break;
	}
}

static void alsatimer_query_set_property(GObject *obj, guint id,
					 const GValue *val, GParamSpec *spec)
{
	/* All of properties are read-only. */
	G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
}

static void alsatimer_query_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsatimer_query_parent_class)->dispose(gobject);
}

static void alsatimer_query_finalize(GObject *gobject)
{
	ALSATimerQuery *self = ALSA_TIMER_QUERY(gobject);
	ALSATimerQueryPrivate *priv = TIMER_QUERY_GET_PRIVATE(self);

	snd_timer_id_free(priv->id);
	snd_timer_ginfo_free(priv->info);
	snd_timer_query_close(self->priv->handle);

	G_OBJECT_CLASS(alsatimer_query_parent_class)->finalize(gobject);
}

static void alsatimer_query_class_init(ALSATimerQueryClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->get_property = alsatimer_query_get_property;
	gobject_class->set_property = alsatimer_query_set_property;
	gobject_class->dispose = alsatimer_query_dispose;
	gobject_class->finalize = alsatimer_query_finalize;

	timer_query_props[TIMER_QUERY_PROP_ID] = 
		g_param_spec_string("id", "id",
				    "The id of this timer",
				    NULL,
				    G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_NAME] = 
		g_param_spec_string("name", "name",
				    "The name of this timer",
				    NULL,
				    G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_CLASS] = 
		g_param_spec_int("class", "class",
				 "The class for this timer",
				 -1, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_SUB_CLASS] = 
		g_param_spec_int("sub-class", "sub-class",
				 "The sub class for this timer",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_CARD] = 
		g_param_spec_int("card", "card",
				 "The number of card for this timer",
				 -1, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_DEVICE] = 
		g_param_spec_int("device", "device",
				 "The device for this timer",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_query_props[TIMER_QUERY_PROP_SUB_DEVICE] = 
		g_param_spec_int("sub-device", "sub-device",
				 "The sub-device for this timer",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);

	g_object_class_install_properties(gobject_class, TIMER_QUERY_PROP_COUNT,
					  timer_query_props);
}

static void
alsatimer_query_init(ALSATimerQuery *self)
{
	self->priv = alsatimer_query_get_instance_private(self);
}

ALSATimerQuery *alsatimer_query_new(GError **exception)
{
	ALSATimerQuery *self;
	ALSATimerQueryPrivate *priv;
	snd_timer_id_t *id;
	snd_timer_ginfo_t *info;
	snd_timer_query_t *handle;
	int err;

	err = snd_timer_query_open(&handle, "hw", 0);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	err = snd_timer_ginfo_malloc(&info);
	if (err < 0) {
		snd_timer_query_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}
	
	err = snd_timer_id_malloc(&id);
	if (err < 0) {
		snd_timer_ginfo_free(info);
		snd_timer_query_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}
	snd_timer_id_set_class(id, SND_TIMER_CLASS_NONE);

	/* NOTE: The system may have at least one timer. */
	snd_timer_query_next_device(handle, id);
	snd_timer_ginfo_set_tid(info, id);

	/* Fill information of this timer timer. */
	err = snd_timer_query_info(handle, info);
	if (err < 0) {
		snd_timer_id_free(id);
		snd_timer_ginfo_free(info);
		snd_timer_query_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	self = g_object_new(ALSA_TIMER_TYPE_QUERY, NULL);
	if (self == NULL) {
		snd_timer_id_free(id);
		snd_timer_ginfo_free(info);
		snd_timer_query_close(handle);
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", strerror(ENOMEM));
		return NULL;
	}
	priv = TIMER_QUERY_GET_PRIVATE(self);

	priv->id = id;
	priv->info = info;
	priv->handle = handle;

	return self;
}

/**
 * alsatimer_query_adjoin
 *
 * Return a query with adjoined timer.
 *
 * Returns: (transfer full): a new #ALSATimerClient
 */
ALSATimerQuery *alsatimer_query_adjoin(ALSATimerQuery *self, GError **exception)
{
	ALSATimerQueryPrivate *priv = TIMER_QUERY_GET_PRIVATE(self);
	int err;

	/* Go to next timer. */
	err = snd_timer_query_next_device(priv->handle, priv->id);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* No more timers. */
	if (snd_timer_id_get_class(priv->id) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENODEV, "%s", strerror(ENODEV));
		return NULL;
	}
	snd_timer_ginfo_set_tid(priv->info, priv->id);

	/* Fill information for this timer timer. */
	err = snd_timer_query_info(priv->handle, priv->info);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* NOTE: Without this, the object will be garbage-collected. */
        return g_object_ref(self);
}

/**
 * alsatimer_query_get_client:
 *
 * Return a client corresponding to a timer indicated by the query
 *
 * Returns: (transfer full): a new #ALSATimerClient
 */
ALSATimerClient *alsatimer_query_get_client(ALSATimerQuery *self,
					    GError **exception)
{
	ALSATimerQueryPrivate *priv = TIMER_QUERY_GET_PRIVATE(self);
	ALSATimerClient *client;
	unsigned char timer[50];
	int class, card;

	class = snd_timer_id_get_class(priv->id);
	if (class < 0)
		class = 0;

	card = snd_timer_id_get_card(priv->id);
	if (card < 0)
		card = 0;

	snprintf(timer, sizeof(timer),
		 "hw:CLASS=%i,SCLASS=%i,CARD=%i,DEV=%i,SUBDEV=%i",
		class,
		snd_timer_id_get_sclass(priv->id),
		card,
		snd_timer_id_get_device(priv->id),
		snd_timer_id_get_subdevice(priv->id));

	return alsatimer_client_new(timer, exception);
}
