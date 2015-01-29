#include <alsa/asoundlib.h>
#include "elemset.h"

struct _ALSACtlElemsetPrivate {
	snd_ctl_t *handle;
	snd_ctl_elem_info_t *info;
};
G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElemset, alsactl_elemset, G_TYPE_OBJECT)
#define CTL_ELEMSET_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				ALSACTL_TYPE_ELEMSET, ALSACtlElemsetPrivate))

enum ctl_elemset_prop_type {
	/* Identifications */
	CTL_ELEMSET_PROP_NAME = 1,
	CTL_ELEMSET_PROP_TYPE,
	CTL_ELEMSET_PROP_ID,
	CTL_ELEMSET_PROP_IFACE,
	CTL_ELEMSET_PROP_DEVICE,
	CTL_ELEMSET_PROP_SUBDEVICE,
	/* Parameters */
	CTL_ELEMSET_PROP_ELEMENTS,
	/* Permissions */
	CTL_ELEMSET_PROP_READABLE,
	CTL_ELEMSET_PROP_WRITABLE,
	CTL_ELEMSET_PROP_VOLATILE,
	CTL_ELEMSET_PROP_INACTIVE,
	CTL_ELEMSET_PROP_LOCKED,
	CTL_ELEMSET_PROP_IS_MINE,
	CTL_ELEMSET_PROP_IS_USER,
	CTL_ELEMSET_PROP_COUNT,
};
static GParamSpec *ctl_elemset_props[CTL_ELEMSET_PROP_COUNT] = { NULL, };

/* This object has one signal. */
enum ctl_elemset_sig_type {
	CTL_ELEMSET_SIG_CHANGED = 0,
	CTL_ELEMSET_SIG_UPDATED,
	CTL_ELEMSET_SIG_REMOVED,
	CTL_ELEMSET_SIG_COUNT,
};
static guint ctl_elemset_sigs[CTL_ELEMSET_SIG_COUNT] = { 0 };

static void ctl_elemset_get_property(GObject *obj, guint id,
				     GValue *val, GParamSpec *spec)
{
	ALSACtlElemset *self = ALSACTL_ELEMSET(obj);
	ALSACtlElemsetPrivate *priv = CTL_ELEMSET_GET_PRIVATE(self);

	switch (id) {
	case CTL_ELEMSET_PROP_NAME:
		g_value_set_string(val, snd_ctl_elem_info_get_name(priv->info));
		break;
	case CTL_ELEMSET_PROP_TYPE:
		g_value_set_int(val, snd_ctl_elem_info_get_type(priv->info));
		break;
	case CTL_ELEMSET_PROP_ID:
		g_value_set_uint(val, snd_ctl_elem_info_get_numid(priv->info));
		break;
	case CTL_ELEMSET_PROP_IFACE:
		g_value_set_int(val,
				snd_ctl_elem_info_get_interface(priv->info));
		break;
	case CTL_ELEMSET_PROP_DEVICE:
		g_value_set_uint(val,
				 snd_ctl_elem_info_get_device(priv->info));
		break;
	case CTL_ELEMSET_PROP_SUBDEVICE:
		g_value_set_uint(val,
				 snd_ctl_elem_info_get_subdevice(priv->info));
		break;
	case CTL_ELEMSET_PROP_ELEMENTS:
		g_value_set_uint(val,
				 snd_ctl_elem_info_get_count(priv->info));
		break;
	case CTL_ELEMSET_PROP_READABLE:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_readable(priv->info));
		break;
	case CTL_ELEMSET_PROP_WRITABLE:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_writable(priv->info));
		break;
	case CTL_ELEMSET_PROP_VOLATILE:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_volatile(priv->info));
		break;
	case CTL_ELEMSET_PROP_INACTIVE:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_inactive(priv->info));
		break;
	case CTL_ELEMSET_PROP_LOCKED:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_locked(priv->info));
		break;
	case CTL_ELEMSET_PROP_IS_MINE:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_owner(priv->info));
		break;
	case CTL_ELEMSET_PROP_IS_USER:
		g_value_set_boolean(val,
				    snd_ctl_elem_info_is_user(priv->info));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void ctl_elemset_set_property(GObject *obj, guint id,
				     const GValue *val, GParamSpec *spec)
{
	ALSACtlElemset *self = ALSACTL_ELEMSET(obj);
	ALSACtlElemsetPrivate *priv = CTL_ELEMSET_GET_PRIVATE(self);

	switch (id) {
	/* These should be set by constructor. */
	case CTL_ELEMSET_PROP_NAME:
		snd_ctl_elem_info_set_name(priv->info,
					   g_value_get_string(val));
		break;
	case CTL_ELEMSET_PROP_ID:
		snd_ctl_elem_info_set_numid(priv->info, g_value_get_uint(val));
		break;
	case CTL_ELEMSET_PROP_IFACE:
		snd_ctl_elem_info_set_interface(priv->info,
						g_value_get_int(val));
		break;
	case CTL_ELEMSET_PROP_DEVICE:
		snd_ctl_elem_info_set_device(priv->info, g_value_get_uint(val));
		break;
	case CTL_ELEMSET_PROP_SUBDEVICE:
		snd_ctl_elem_info_set_subdevice(priv->info,
						g_value_get_uint(val));
		break;
	/* The index is not required. */
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void ctl_elemset_dispose(GObject *obj)
{
	ALSACtlElemset *self = ALSACTL_ELEMSET(obj);
	ALSACtlElemsetPrivate *priv = CTL_ELEMSET_GET_PRIVATE(self);
	snd_ctl_elem_id_t *id;

	/* Ignore any errors. */
	GError *exception = NULL;

	alsactl_elemset_update(self, &exception);

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_get_id(priv->info, id);
	snd_ctl_elem_remove(self->client->handle, id);

	if (snd_ctl_elem_info_is_owner(priv->info))
		alsactl_client_remove_elem(self->client, self, &exception);

	G_OBJECT_CLASS(alsactl_elemset_parent_class)->dispose(obj);
}

static void ctl_elemset_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elemset_parent_class)->finalize(gobject);
}

static void alsactl_elemset_class_init(ALSACtlElemsetClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = ctl_elemset_get_property;
	gobject_class->set_property = ctl_elemset_set_property;
	gobject_class->dispose = ctl_elemset_dispose;
	gobject_class->finalize = ctl_elemset_finalize;

	ctl_elemset_props[CTL_ELEMSET_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "The name for this element set",
				    "element",
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type of this element",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_ID] =
		g_param_spec_uint("id", "id",
				  "The numerical ID for this element set",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_IFACE] =
		g_param_spec_int("iface", "iface",
				 "The type of interface for this element set",
				 0, INT_MAX,
				 0,
				 G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_DEVICE] =
		g_param_spec_uint("device", "device",
			"The numerical number for device of this element set",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_SUBDEVICE] =
		g_param_spec_uint("subdevice", "subdevice",
		"The numerical number of subdevice for this element set",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_ELEMENTS] =
		g_param_spec_uint("count", "count",
				  "The number of elements in this element set",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_READABLE] =
		g_param_spec_boolean("readable", "readable",
				"Whether this element set is readable or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_WRITABLE] =
		g_param_spec_boolean("writable", "writable",
				"Whether this element set is writable or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_VOLATILE] =
		g_param_spec_boolean("volatile", "volatile",
				     "?????",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_INACTIVE] =
		g_param_spec_boolean("inactive", "inactive",
				"Whether this element set is inactive or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_LOCKED] =
		g_param_spec_boolean("locked", "locked",
				"Whether this element set is locked or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_IS_MINE] =
		g_param_spec_boolean("is-mine", "is-mine",
			"Whether this process owns this element set or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_IS_USER] =
		g_param_spec_boolean("is-user", "is-user",
			"Whether this elment set is added by userland or not",
				     FALSE,
				     G_PARAM_READABLE);
	g_object_class_install_properties(gobject_class,
					  CTL_ELEMSET_PROP_COUNT,
					  ctl_elemset_props);

	/**
	 * ALSACtlElemset::changed:
	 * @self: A #ALSACtlElemset
	 *
	 * The values of some elements in this element set are changed.
	 */
	ctl_elemset_sigs[CTL_ELEMSET_SIG_CHANGED] =
		g_signal_new("changed",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
	/**
	 * ALSACtlElemset::updated:
	 * @self: A #ALSACtlElemset
	 *
	 * The information of this element set are changed.
	 */
	ctl_elemset_sigs[CTL_ELEMSET_SIG_UPDATED] =
		g_signal_new("updated",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
	/**
	 * ALSACtlElemset::removed:
	 * @self: A #ALSACtlElemset
	 *
	 * This element set is removed.
	 */
	ctl_elemset_sigs[CTL_ELEMSET_SIG_REMOVED] =
		g_signal_new("removed",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
}

static void alsactl_elemset_init(ALSACtlElemset *self)
{
	self->priv = alsactl_elemset_get_instance_private(self);
	/* Mmm... */
	snd_ctl_elem_info_malloc(&self->priv->info);
}

void alsactl_elemset_update(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	int err;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	if (self->client->handle == NULL) {
		err = -EINVAL;
		goto end;
	}

	err = snd_ctl_elem_info(self->client->handle, priv->info);
end:
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}

void alsactl_elemset_lock(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	snd_ctl_elem_id_t *id;
	int err;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	if (self->client->handle == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_get_id(priv->info, id);
	err = snd_ctl_elem_lock(self->client->handle, id);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));

	alsactl_elemset_update(self, exception);
}

void alsactl_elemset_unlock(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	snd_ctl_elem_id_t *id;
	int err;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	if (self->client->handle == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_get_id(priv->info, id);
	err = snd_ctl_elem_unlock(self->client->handle, id);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));

	alsactl_elemset_update(self, exception);
}
