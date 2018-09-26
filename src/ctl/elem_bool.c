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

/* For error handling. */
G_DEFINE_QUARK("ALSACtlElemBool", alsactl_elem_bool)
#define raise(exception, errno)                                 \
    g_set_error(exception, alsactl_elem_bool_quark(), errno,    \
            "%d: %s", __LINE__, strerror(errno))

G_DEFINE_TYPE(ALSACtlElemBool, alsactl_elem_bool, ALSACTL_TYPE_ELEM)

static void alsactl_elem_bool_class_init(ALSACtlElemBoolClass *klass)
{
    return;
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
    unsigned int channels;
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_BOOL(self));

    if ((values == NULL) ||
         (g_array_get_element_size(values) != sizeof(gboolean))) {
        raise(exception, EINVAL);
        return;
    }

    alsactl_elem_value_ioctl(ALSACTL_ELEM(self),
                             SNDRV_CTL_IOCTL_ELEM_READ, &elem_val, exception);
    if (*exception != NULL)
        return;

    /* Check the number of values in this element. */
    g_value_init(&tmp, G_TYPE_UINT);
    g_object_get_property(G_OBJECT(self), "channels", &tmp);
    channels = g_value_get_uint(&tmp);

    /* Copy for application. */
    for (i = 0; i < channels; i++)
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
    unsigned int channels;
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_BOOL(self));

    if (values == NULL ||
        g_array_get_element_size(values) != sizeof(gboolean) ||
        values->len == 0) {
        raise(exception, EINVAL);
        return;
    }

    /* Calculate the number of values in this element. */
    g_value_init(&tmp, G_TYPE_UINT);
    g_object_get_property(G_OBJECT(self), "channels", &tmp);
    channels = MIN(values->len, g_value_get_uint(&tmp));

    /* Copy for driver. */
    for (i = 0; i < channels; i++)
        vals[i] = g_array_index(values, gboolean, i);

    alsactl_elem_value_ioctl(ALSACTL_ELEM(self), SNDRV_CTL_IOCTL_ELEM_WRITE,
                             &elem_val, exception);
}
