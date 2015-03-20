#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stddef.h>

#include <sound/asound.h>
#include "elemset.h"

struct _ALSACtlElemsetPrivate {
	int fd;
	struct snd_ctl_elem_info info;
};
G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElemset, alsactl_elemset, G_TYPE_OBJECT)
#define CTL_ELEMSET_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				ALSACTL_TYPE_ELEMSET, ALSACtlElemsetPrivate))

enum ctl_elemset_prop_type {
	CTL_ELEMSET_PROP_FD = 1,
	CTL_ELEMSET_PROP_TYPE,
	CTL_ELEMSET_PROP_ELEMENTS,
	/* Identifications */
	CTL_ELEMSET_PROP_NAME,
	CTL_ELEMSET_PROP_ID,
	CTL_ELEMSET_PROP_IFACE,
	CTL_ELEMSET_PROP_DEVICE,
	CTL_ELEMSET_PROP_SUBDEVICE,
	/* Permissions */
	CTL_ELEMSET_PROP_READABLE,
	CTL_ELEMSET_PROP_WRITABLE,
	CTL_ELEMSET_PROP_VOLATILE,
	CTL_ELEMSET_PROP_INACTIVE,
	CTL_ELEMSET_PROP_LOCKED,
	CTL_ELEMSET_PROP_IS_OWNED,
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
	case CTL_ELEMSET_PROP_TYPE:
		g_value_set_int(val, priv->info.type);
		break;
	case CTL_ELEMSET_PROP_ELEMENTS:
		g_value_set_uint(val, priv->info.count);
		break;
	case CTL_ELEMSET_PROP_NAME:
		g_value_set_string(val, priv->info.id.name);
		break;
	case CTL_ELEMSET_PROP_ID:
		g_value_set_uint(val, priv->info.id.numid);
		break;
	case CTL_ELEMSET_PROP_IFACE:
		g_value_set_int(val, priv->info.id.iface);
		break;
	case CTL_ELEMSET_PROP_DEVICE:
		g_value_set_uint(val, priv->info.id.device);
		break;
	case CTL_ELEMSET_PROP_SUBDEVICE:
		g_value_set_uint(val, priv->info.id.subdevice);
		break;
	case CTL_ELEMSET_PROP_READABLE:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_READ));
		break;
	case CTL_ELEMSET_PROP_WRITABLE:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_WRITE));
		break;
	case CTL_ELEMSET_PROP_VOLATILE:
		g_value_set_boolean(val,
		    !!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_VOLATILE));
		break;
	case CTL_ELEMSET_PROP_INACTIVE:
		g_value_set_boolean(val,
		    !!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_INACTIVE));
		break;
	case CTL_ELEMSET_PROP_LOCKED:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_LOCK));
		break;
	case CTL_ELEMSET_PROP_IS_OWNED:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_OWNER));
		break;
	case CTL_ELEMSET_PROP_IS_USER:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_USER));
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
	case CTL_ELEMSET_PROP_FD:
		priv->fd = g_value_get_int(val);
		break;
	case CTL_ELEMSET_PROP_NAME:
		strncpy(priv->info.id.name, g_value_get_string(val),
						sizeof(priv->info.id.name));
		break;
	case CTL_ELEMSET_PROP_ID:
		priv->info.id.numid = g_value_get_uint(val);
		break;
	case CTL_ELEMSET_PROP_IFACE:
		priv->info.id.iface = g_value_get_int(val);
		break;
	case CTL_ELEMSET_PROP_DEVICE:
		priv->info.id.device = g_value_get_uint(val);
		break;
	case CTL_ELEMSET_PROP_SUBDEVICE:
		priv->info.id.subdevice = g_value_get_uint(val);
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
	GError *exception = NULL;

	/* Leave ownership to release this elemset. */
	alsactl_elemset_unlock(self, &exception);
	if (exception != NULL)
		g_error_free(exception);

	/* Remove this element as long as no processes owns. */
	if (!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_OWNER))
		ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_REMOVE, &priv->info.id);

	alsactl_client_remove_elem(self->client, self, NULL);

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

	ctl_elemset_props[CTL_ELEMSET_PROP_FD] =
		g_param_spec_int("fd", "fd",
			"file descriptor for special file of control device",
				 INT_MIN, INT_MAX,
				 -1,
				 G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elemset_props[CTL_ELEMSET_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type of this element",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_ELEMENTS] =
		g_param_spec_uint("count", "count",
				  "The number of elements in this element set",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READABLE);
	ctl_elemset_props[CTL_ELEMSET_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "The name for this element set",
				    "element",
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
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
	ctl_elemset_props[CTL_ELEMSET_PROP_IS_OWNED] =
		g_param_spec_boolean("is-owned", "is-owned",
			"Whether some processes owns this element set or not",
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
}

void alsactl_elemset_update(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	int err = 0;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_INFO, &priv->info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
}

void alsactl_elemset_lock(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	struct snd_ctl_elem_id *id;
	int err = 0;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	id = &priv->info.id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LOCK, id) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	} else {
		alsactl_elemset_update(self, exception);
	}
}

void alsactl_elemset_unlock(ALSACtlElemset *self, GError **exception)
{
	ALSACtlElemsetPrivate *priv;
	struct snd_ctl_elem_id *id;
	int err;

	g_return_if_fail(ALSACTL_IS_ELEMSET(self));
	priv = CTL_ELEMSET_GET_PRIVATE(self);

	id = &priv->info.id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, id) >= 0) {
		alsactl_elemset_update(self, exception);
	} else if (errno != -EINVAL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
}
