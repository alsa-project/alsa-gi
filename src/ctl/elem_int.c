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
#include "elem_int.h"

struct _ALSACtlElemIntPrivate {
	guint64 min;
	guint64 max;
	guint64 step;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElemInt, alsactl_elem_int, ALSACTL_TYPE_ELEM)
#define CTL_ELEM_INT_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSACTL_TYPE_ELEM_INT,		\
				     ALSACtlElemIntPrivate))

static void ctl_elem_int_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_int_parent_class)->dispose(obj);
}

static void ctl_elem_int_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_int_parent_class)->finalize(gobject);
}

static void elem_int_update(ALSACtlElem *parent, GError **exception)
{
	ALSACtlElemInt *self;
	ALSACtlElemIntPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_ELEM_INT(parent));
	self = ALSACTL_ELEM_INT(parent);
	priv = CTL_ELEM_INT_GET_PRIVATE(self);

	alsactl_elem_info_ioctl(parent, &info, exception);
	if (*exception != NULL)
		return;

	if (info.type == SNDRV_CTL_ELEM_TYPE_INTEGER) {
		priv->min = info.value.integer.min;
		priv->max = info.value.integer.max;
		priv->step = info.value.integer.step;
	} else {
		priv->min = info.value.integer64.min;
		priv->max = info.value.integer64.max;
		priv->step = info.value.integer64.step;
	}
}

static void alsactl_elem_int_class_init(ALSACtlElemIntClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	/* Override parent method. */
	ALSACTL_ELEM_CLASS(klass)->update = elem_int_update;

	gobject_class->dispose = ctl_elem_int_dispose;
	gobject_class->finalize = ctl_elem_int_finalize;
}

static void alsactl_elem_int_init(ALSACtlElemInt *self)
{
	self->priv = alsactl_elem_int_get_instance_private(self);
}

/**
 * alsactl_elem_int_get_max:
 * @self: A #ALSACtlElemInt
 * @max: (out caller-allocates): the maximum number for each value
 *
 */
void alsactl_elem_int_get_max(ALSACtlElemInt *self, unsigned int *max)
{
	ALSACtlElemIntPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM_INT(self));
	priv = CTL_ELEM_INT_GET_PRIVATE(self);

	*max = priv->max;
}

/**
 * alsactl_elem_int_get_min:
 * @self: A #ALSACtlElemInt
 * @min: (out caller-allocates): the maximum number for each value
 *
 */
void alsactl_elem_int_get_min(ALSACtlElemInt *self, unsigned int *min)
{
	ALSACtlElemIntPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM_INT(self));
	priv = CTL_ELEM_INT_GET_PRIVATE(self);

	*min = priv->min;
}

/**
 * alsactl_elem_int_get_step:
 * @self: A #ALSACtlElemInt
 * @step: (out caller-allocates): the maximum number for each value
 *
 */
void alsactl_elem_int_get_step(ALSACtlElemInt *self, unsigned int *step)
{
	ALSACtlElemIntPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM_INT(self));
	priv = CTL_ELEM_INT_GET_PRIVATE(self);

	*step = priv->step;
}

static void fill_as_uint32(GArray *values, unsigned int count,
			   struct snd_ctl_elem_value *elem_val)
{
	long *vals = elem_val->value.integer.value;
	unsigned int i;

	for (i = 0; i < count; i++)
		g_array_insert_val(values, i, vals[i]);
}

static void fill_as_uint64(GArray *values, unsigned int count,
			   struct snd_ctl_elem_value *elem_val)
{
	long long *vals = elem_val->value.integer64.value;
	unsigned int i;

	for (i = 0; i < count; i++)
		g_array_insert_val(values, i, vals[i]);
}

/**
 * alsactl_elem_int_read:
 * @self: A #ALSACtlElemInt
 * @values: (element-type guint64) (array) (out caller-allocates): a int array
 * @exception: A #GError
 *
 */
void alsactl_elem_int_read(ALSACtlElemInt *self, GArray *values,
			    GError **exception)
{
	struct snd_ctl_elem_value elem_val = {{0}};

	GValue count = G_VALUE_INIT;
	GValue type = G_VALUE_INIT;

	g_return_if_fail(ALSACTL_IS_ELEM_INT(self));

	if ((values == NULL) ||
	     (g_array_get_element_size(values) != sizeof(guint64))) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	alsactl_elem_value_ioctl(ALSACTL_ELEM(self),
				 SNDRV_CTL_IOCTL_ELEM_READ, &elem_val,
				 exception);
	if (*exception != NULL)
		return;


	/* Check the number of values in this element. */
	g_value_init(&count, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(self), "count", &count);

	/* Get type. */
	g_value_init(&type, G_TYPE_INT);
	g_object_get_property(G_OBJECT(self), "type", &type);

	/* Copy for application. */
	if (g_value_get_int(&type) == SNDRV_CTL_ELEM_TYPE_INTEGER)
		fill_as_uint32(values, g_value_get_uint(&count), &elem_val);
	else
		fill_as_uint64(values, g_value_get_uint(&count), &elem_val);
}

static void pull_as_uint32(GArray *values, unsigned int count,
			   struct snd_ctl_elem_value *elem_val)
{
	long *vals = elem_val->value.integer.value;
	unsigned int i;

	count = MIN(count, values->len);

	for (i = 0; i < count; i++)
		vals[i] = g_array_index(values, guint64, i) & 0xffffffff;
}

static void pull_as_uint64(GArray *values, unsigned int count,
			   struct snd_ctl_elem_value *elem_val)
{
	long long *vals = elem_val->value.integer64.value;
	unsigned int i;

	count = MIN(count, values->len);

	for (i = 0; i < count; i++)
		vals[i] = g_array_index(values, guint64, i);
}

/**
 * alsactl_elem_int_write:
 * @self: A #ALSACtlElemInt
 * @values: (element-type guint64) (array) (in): a int array
 * @exception: A #GError
 *
 */
void alsactl_elem_int_write(ALSACtlElemInt *self, GArray *values,
			     GError **exception)
{
	struct snd_ctl_elem_value elem_val = {{0}};

	GValue count = G_VALUE_INIT;
	GValue type = G_VALUE_INIT;

	g_return_if_fail(ALSACTL_IS_ELEM_INT(self));

	if ((values == NULL) ||
	    (g_array_get_element_size(values) != sizeof(guint64)) ||
	    values->len == 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	/* Get the number of values in this element. */
	g_value_init(&count, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(self), "count", &count);

	/* Get type of this element. */
	g_value_init(&type, G_TYPE_INT);
	g_object_get_property(G_OBJECT(self), "type", &type);

	/* Pull values from application for driver. */
	if (g_value_get_int(&type) == SNDRV_CTL_ELEM_TYPE_INTEGER)
		pull_as_uint32(values, g_value_get_uint(&count), &elem_val);
	else
		pull_as_uint64(values, g_value_get_uint(&count), &elem_val);

	alsactl_elem_value_ioctl(ALSACTL_ELEM(self),
				 SNDRV_CTL_IOCTL_ELEM_WRITE, &elem_val,
				 exception);
}
