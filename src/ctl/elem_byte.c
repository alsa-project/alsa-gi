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
#include "elem_byte.h"

/* For error handling. */
G_DEFINE_QUARK("ALSACtlElemByte", alsactl_elem_byte)
#define raise(exception, errno)                                 \
    g_set_error(exception, alsactl_elem_byte_quark(), errno,    \
            "%d: %s", __LINE__, strerror(errno))

G_DEFINE_TYPE(ALSACtlElemByte, alsactl_elem_byte, ALSACTL_TYPE_ELEM)

static void alsactl_elem_byte_class_init(ALSACtlElemByteClass *klass)
{
    return;
}

static void alsactl_elem_byte_init(ALSACtlElemByte *self)
{
    return;
}

/**
 * alsactl_elem_byte_read:
 * @self: A #ALSACtlElemByte
 * @values: (element-type guint8) (array) (out caller-allocates): a 8bit array
 * @exception: A #GError
 *
 */
void alsactl_elem_byte_read(ALSACtlElemByte *self, GArray *values,
                            GError **exception)
{
    struct snd_ctl_elem_value elem_val = {{0}};
    unsigned char *vals = elem_val.value.bytes.data;

    GValue tmp = G_VALUE_INIT;
    unsigned int channels;
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_BYTE(self));

    if ((values == NULL) ||
        (g_array_get_element_size(values) != sizeof(guint8))) {
        raise(exception, EINVAL);
        return;
    }

    alsactl_elem_value_ioctl(ALSACTL_ELEM(self), SNDRV_CTL_IOCTL_ELEM_READ,
                             &elem_val, exception);
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
 * alsactl_elem_byte_write:
 * @self: A #ALSACtlElemByte
 * @values: (element-type guint8) (array) (in): a 8bit array
 * @exception: A #GError
 *
 */
void alsactl_elem_byte_write(ALSACtlElemByte *self, GArray *values,
                             GError **exception)
{
    struct snd_ctl_elem_value elem_val = {{0}};
    unsigned char *vals = elem_val.value.bytes.data;

    GValue tmp = G_VALUE_INIT;
    unsigned int channels;
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_BYTE(self));

    if ((values == NULL) ||
        (g_array_get_element_size(values) != sizeof(guint8)) ||
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
        vals[i] = g_array_index(values, guint8, i);

    alsactl_elem_value_ioctl(ALSACTL_ELEM(self), SNDRV_CTL_IOCTL_ELEM_WRITE,
                             &elem_val, exception);
}
