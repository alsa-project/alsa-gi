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
#include "elem.h"

struct _ALSACtlElemPrivate {
	int fd;
	struct snd_ctl_elem_info info;
};
G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElem, alsactl_elem, G_TYPE_OBJECT)
#define CTL_ELEM_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				ALSACTL_TYPE_ELEM, ALSACtlElemPrivate))

enum ctl_elem_prop_type {
	CTL_ELEM_PROP_FD = 1,
	CTL_ELEM_PROP_TYPE,
	CTL_ELEM_PROP_VALUES,
	/* Identifications */
	CTL_ELEM_PROP_NAME,
	CTL_ELEM_PROP_ID,
	CTL_ELEM_PROP_IFACE,
	CTL_ELEM_PROP_DEVICE,
	CTL_ELEM_PROP_SUBDEVICE,
	/* Permissions */
	CTL_ELEM_PROP_READABLE,
	CTL_ELEM_PROP_WRITABLE,
	CTL_ELEM_PROP_VOLATILE,
	CTL_ELEM_PROP_INACTIVE,
	CTL_ELEM_PROP_LOCKED,
	CTL_ELEM_PROP_IS_OWNED,
	CTL_ELEM_PROP_IS_USER,
	CTL_ELEM_PROP_COUNT,
};
static GParamSpec *ctl_elem_props[CTL_ELEM_PROP_COUNT] = { NULL, };

/* This object has one signal. */
enum ctl_elem_sig_type {
	CTL_ELEM_SIG_CHANGED = 0,
	CTL_ELEM_SIG_UPDATED,
	CTL_ELEM_SIG_REMOVED,
	CTL_ELEM_SIG_COUNT,
};
static guint ctl_elem_sigs[CTL_ELEM_SIG_COUNT] = { 0 };

static void ctl_elem_get_property(GObject *obj, guint id,
				     GValue *val, GParamSpec *spec)
{
	ALSACtlElem *self = ALSACTL_ELEM(obj);
	ALSACtlElemPrivate *priv = CTL_ELEM_GET_PRIVATE(self);

	switch (id) {
	case CTL_ELEM_PROP_TYPE:
		g_value_set_int(val, priv->info.type);
		break;
	case CTL_ELEM_PROP_VALUES:
		g_value_set_uint(val, priv->info.count);
		break;
	case CTL_ELEM_PROP_NAME:
		g_value_set_string(val, (char *)priv->info.id.name);
		break;
	case CTL_ELEM_PROP_ID:
		g_value_set_uint(val, priv->info.id.numid);
		break;
	case CTL_ELEM_PROP_IFACE:
		g_value_set_int(val, priv->info.id.iface);
		break;
	case CTL_ELEM_PROP_DEVICE:
		g_value_set_uint(val, priv->info.id.device);
		break;
	case CTL_ELEM_PROP_SUBDEVICE:
		g_value_set_uint(val, priv->info.id.subdevice);
		break;
	case CTL_ELEM_PROP_READABLE:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_READ));
		break;
	case CTL_ELEM_PROP_WRITABLE:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_WRITE));
		break;
	case CTL_ELEM_PROP_VOLATILE:
		g_value_set_boolean(val,
		    !!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_VOLATILE));
		break;
	case CTL_ELEM_PROP_INACTIVE:
		g_value_set_boolean(val,
		    !!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_INACTIVE));
		break;
	case CTL_ELEM_PROP_LOCKED:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_LOCK));
		break;
	case CTL_ELEM_PROP_IS_OWNED:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_OWNER));
		break;
	case CTL_ELEM_PROP_IS_USER:
		g_value_set_boolean(val,
			!!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_USER));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void ctl_elem_set_property(GObject *obj, guint id,
				     const GValue *val, GParamSpec *spec)
{
	ALSACtlElem *self = ALSACTL_ELEM(obj);
	ALSACtlElemPrivate *priv = CTL_ELEM_GET_PRIVATE(self);

	switch (id) {
	/* These should be set by constructor. */
	case CTL_ELEM_PROP_FD:
		priv->fd = g_value_get_int(val);
		break;
	case CTL_ELEM_PROP_NAME:
		strncpy((char *)priv->info.id.name, g_value_get_string(val),
			sizeof(priv->info.id.name));
		break;
	case CTL_ELEM_PROP_ID:
		priv->info.id.numid = g_value_get_uint(val);
		break;
	case CTL_ELEM_PROP_IFACE:
		priv->info.id.iface = g_value_get_int(val);
		break;
	case CTL_ELEM_PROP_DEVICE:
		priv->info.id.device = g_value_get_uint(val);
		break;
	case CTL_ELEM_PROP_SUBDEVICE:
		priv->info.id.subdevice = g_value_get_uint(val);
		break;
	/* The index is not required. */
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void ctl_elem_dispose(GObject *obj)
{
	ALSACtlElem *self = ALSACTL_ELEM(obj);
	ALSACtlElemPrivate *priv = CTL_ELEM_GET_PRIVATE(self);
	GError *exception = NULL;

	/* Leave ownership to release this elemset. */
	alsactl_elem_unlock(self, &exception);
	if (exception != NULL)
		g_error_free(exception);

	/* Remove this element as long as no processes owns. */
	if (!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_OWNER))
		ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_REMOVE, &priv->info.id);

	alsactl_client_remove_elem(self->_client, self);

	G_OBJECT_CLASS(alsactl_elem_parent_class)->dispose(obj);
}

static void ctl_elem_finalize(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_parent_class)->finalize(obj);
}

static void elem_update(ALSACtlElem *self, GError **exception)
{
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_ELEM(self));

	alsactl_elem_info_ioctl(ALSACTL_ELEM(self), &info, exception);
}

static void alsactl_elem_class_init(ALSACtlElemClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	/* Set default method. */
	klass->update = elem_update;

	gobject_class->get_property = ctl_elem_get_property;
	gobject_class->set_property = ctl_elem_set_property;
	gobject_class->dispose = ctl_elem_dispose;
	gobject_class->finalize = ctl_elem_finalize;

	ctl_elem_props[CTL_ELEM_PROP_FD] =
		g_param_spec_int("fd", "fd",
			"file descriptor for special file of control device",
			INT_MIN, INT_MAX,
			-1,
			G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type of this element",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_VALUES] =
		g_param_spec_uint("count", "count",
				  "The number of values in this element",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "The name for this element",
				    "element",
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_ID] =
		g_param_spec_uint("id", "id",
				  "The numerical ID for this element",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_IFACE] =
		g_param_spec_int("iface", "iface",
				 "The type of interface for this element",
				 0, INT_MAX,
				 0,
				 G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_DEVICE] =
		g_param_spec_uint("device", "device",
			"The numerical number for device of this element",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_SUBDEVICE] =
		g_param_spec_uint("subdevice", "subdevice",
		"The numerical number of subdevice for this element",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_READABLE] =
		g_param_spec_boolean("readable", "readable",
				"Whether this element is readable or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_WRITABLE] =
		g_param_spec_boolean("writable", "writable",
				"Whether this element is writable or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_VOLATILE] =
		g_param_spec_boolean("volatile", "volatile",
				     "?????",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_INACTIVE] =
		g_param_spec_boolean("inactive", "inactive",
				"Whether this element is inactive or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_LOCKED] =
		g_param_spec_boolean("locked", "locked",
				"Whether this element is locked or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_IS_OWNED] =
		g_param_spec_boolean("is-owned", "is-owned",
			"Whether some processes owns this element or not",
				     FALSE,
				     G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_IS_USER] =
		g_param_spec_boolean("is-user", "is-user",
			"Whether this elment set is added by userland or not",
				     FALSE,
				     G_PARAM_READABLE);
	g_object_class_install_properties(gobject_class,
					  CTL_ELEM_PROP_COUNT,
					  ctl_elem_props);

	/**
	 * ALSACtlElem::changed:
	 * @self: A #ALSACtlElem
	 *
	 * The values in this element are changed.
	 */
	ctl_elem_sigs[CTL_ELEM_SIG_CHANGED] =
		g_signal_new("changed",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
	/**
	 * ALSACtlElem::updated:
	 * @self: A #ALSACtlElem
	 *
	 * The information of this element are changed.
	 */
	ctl_elem_sigs[CTL_ELEM_SIG_UPDATED] =
		g_signal_new("updated",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
	/**
	 * ALSACtlElem::removed:
	 * @self: A #ALSACtlElem
	 *
	 * This element is removed.
	 */
	ctl_elem_sigs[CTL_ELEM_SIG_REMOVED] =
		g_signal_new("removed",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__VOID,
			     G_TYPE_NONE, 0, NULL);
}

static void alsactl_elem_init(ALSACtlElem *self)
{
	self->priv = alsactl_elem_get_instance_private(self);
}

void alsactl_elem_update(ALSACtlElem *self, GError **exception)
{
	ALSACTL_ELEM_GET_CLASS(self)->update(self, exception);
}

void alsactl_elem_lock(ALSACtlElem *self, GError **exception)
{
	ALSACtlElemPrivate *priv;
	struct snd_ctl_elem_id *id;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = CTL_ELEM_GET_PRIVATE(self);

	id = &priv->info.id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LOCK, id) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	} else {
		alsactl_elem_update(self, exception);
	}
}

void alsactl_elem_unlock(ALSACtlElem *self, GError **exception)
{
	ALSACtlElemPrivate *priv;
	struct snd_ctl_elem_id *id;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = CTL_ELEM_GET_PRIVATE(self);

	id = &priv->info.id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_UNLOCK, id) >= 0) {
		alsactl_elem_update(self, exception);
	} else if (errno != -EINVAL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
}

void alsactl_elem_value_ioctl(ALSACtlElem *self, int cmd,
			      struct snd_ctl_elem_value *elem_val,
			      GError **exception)
{
	ALSACtlElemPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = CTL_ELEM_GET_PRIVATE(self);

	elem_val->id.numid = priv->info.id.numid;
	if (ioctl(priv->fd, cmd, elem_val) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}
}

void alsactl_elem_info_ioctl(ALSACtlElem *self, struct snd_ctl_elem_info *info,
			     GError **exception)
{
	ALSACtlElemPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = CTL_ELEM_GET_PRIVATE(self);

	info->id.numid = priv->info.id.numid;

	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
	}

	/*
	 * The numid is rollback to a numid of the first element in this set.
	 * This is a workaround for this ugly design.
	 *
	 * TODO: upstream should fix this bug. numid or index should be
	 * kept as it was.
	 */
	info->id.numid = priv->info.id.numid;

	/* Copy updated information. */
	priv->info = *info;
}
