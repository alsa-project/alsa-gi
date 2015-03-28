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
#include "elem_bool.h"

G_DEFINE_TYPE(ALSACtlElemBool, alsactl_elem_bool, ALSACTL_TYPE_ELEM)

static void ctl_elem_bool_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_bool_parent_class)->dispose(obj);
}

static void ctl_elem_bool_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_bool_parent_class)->finalize(gobject);
}

static void alsactl_elem_bool_class_init(ALSACtlElemBoolClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elem_bool_dispose;
	gobject_class->finalize = ctl_elem_bool_finalize;
}

static void alsactl_elem_bool_init(ALSACtlElemBool *self)
{
	return;
}

/**
 * alsactl_elem_bool_read:
 * @self: A #ALSACtlElemBool
 * @values: (element-type gboolean) (array) (out caller-allocates): a bool array
 * @exception: A #GError
 *
 */
void alsactl_elem_bool_read(ALSACtlElemBool *self, GArray *values,
			    GError **exception)
{
	struct snd_ctl_elem_value elem_val = {{0}};
	long *vals = elem_val.value.integer.value;

	GValue tmp = G_VALUE_INIT;
	unsigned int count;
	unsigned int i;

	g_return_if_fail(ALSACTL_IS_ELEM_BOOL(self));

	if ((values == NULL) ||
	     (g_array_get_element_size(values) != sizeof(gboolean))) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	/* ioctl(2) */
	if (*exception != NULL)
		return;

	/* Check the number of values in this element. */
	g_value_init(&tmp, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(self), "count", &tmp);
	count = g_value_get_uint(&tmp);

	/* Copy for application. */
	for (i = 0; i < count; i++)
		g_array_insert_val(values, i, vals[i]);
}

/**
 * alsactl_elem_bool_write:
 * @self: A #ALSACtlElemBool
 * @values: (element-type gboolean) (array) (in): a bool array
 * @exception: A #GError
 *
 */
void alsactl_elem_bool_write(ALSACtlElemBool *self, GArray *values,
			     GError **exception)
{
	struct snd_ctl_elem_value elem_val = {{0}};
	long *vals = elem_val.value.integer.value;

	GValue tmp = G_VALUE_INIT;
	unsigned int count;
	unsigned int i;

	g_return_if_fail(ALSACTL_IS_ELEM_BOOL(self));

	if (values == NULL ||
	    g_array_get_element_size(values) != sizeof(gboolean) ||
	    values->len == 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	/* Calculate the number of values in this element. */
	g_value_init(&tmp, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(self), "count", &tmp);
	count = MIN(values->len, g_value_get_uint(&tmp));

	/* Copy for driver. */
	for (i = 0; i < count; i++)
		vals[i] = g_array_index(values, gboolean, i);

	/* ioctl(2) */
}