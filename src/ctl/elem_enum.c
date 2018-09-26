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
#include "elem_enum.h"

#define CHARS_PER_LABEL    64
#define LABELS_PER_ELEM    1024

/* For error handling. */
G_DEFINE_QUARK("ALSACtlElemEnum", alsactl_elem_enum)
#define raise(exception, errno)                                 \
    g_set_error(exception, alsactl_elem_enum_quark(), errno,    \
            "%d: %s", __LINE__, strerror(errno))

struct _ALSACtlElemEnumPrivate {
    unsigned int labels_count;
    char (*strings)[CHARS_PER_LABEL];
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElemEnum, alsactl_elem_enum,
                           ALSACTL_TYPE_ELEM)

static void ctl_elem_enum_finalize(GObject *obj)
{
    ALSACtlElemEnum *self = ALSACTL_ELEM_ENUM(obj);
    ALSACtlElemEnumPrivate *priv = alsactl_elem_enum_get_instance_private(self);

    g_slice_free1(CHARS_PER_LABEL * LABELS_PER_ELEM, priv->strings);

    G_OBJECT_CLASS(alsactl_elem_enum_parent_class)->finalize(obj);
}

static void elem_enum_update(ALSACtlElem *parent, GError **exception)
{
    ALSACtlElemEnum *self;
    ALSACtlElemEnumPrivate *priv;
    struct snd_ctl_elem_info info = {{0}};
    char (*strings)[CHARS_PER_LABEL];
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_ENUM(parent));
    self = ALSACTL_ELEM_ENUM(parent);
    priv = alsactl_elem_enum_get_instance_private(self);
    strings = priv->strings;

    /* Get the count of items. */
    alsactl_elem_info_ioctl(ALSACTL_ELEM(self), &info, exception);
    if (*exception != NULL)
        return;
    priv->labels_count = info.value.enumerated.items;

    /* Set the name of each item. */
    for (i = 0; i < priv->labels_count; i++) {
        info.value.enumerated.item = i;

        alsactl_elem_info_ioctl(ALSACTL_ELEM(self), &info, exception);
        if (*exception != NULL)
            return;

        strcpy(strings[i], info.value.enumerated.name);
    }
}

static void alsactl_elem_enum_class_init(ALSACtlElemEnumClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    /* Override parent method. */
    ALSACTL_ELEM_CLASS(klass)->update = elem_enum_update;

    gobject_class->finalize = ctl_elem_enum_finalize;
}

static void alsactl_elem_enum_init(ALSACtlElemEnum *self)
{
    ALSACtlElemEnumPrivate *priv = alsactl_elem_enum_get_instance_private(self);

    /*
     * The maximum length of each item is 64 characters.
     * The maximum number of items in one element is 1024 entries.
     */
    priv->strings = (char (*)[CHARS_PER_LABEL])
            g_slice_alloc0(CHARS_PER_LABEL * LABELS_PER_ELEM);
}

/**
 * alsactl_elem_enum_get_labels:
 * @self: A #ALSACtlElemEnum
 * @labels: (element-type utf8) (out caller-allocates) (transfer container): a strings array
 * @exception: A #GError
 *
 */
void alsactl_elem_enum_get_labels(ALSACtlElemEnum *self, GArray *labels,
                                  GError **exception)
{
    ALSACtlElemEnumPrivate *priv;
    char (*strings)[CHARS_PER_LABEL];
    char *string;
    unsigned int i;

    g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
    priv = alsactl_elem_enum_get_instance_private(self);
    strings = priv->strings;

    for (i = 0; i < priv->labels_count; i++) {
        string = strings[i];

        /* The type of last parameter is important. */
        g_array_insert_val(labels, i, string);
    }
}

static void fill_as_string(GArray *values, char (*strings)[CHARS_PER_LABEL],
                           unsigned int channels,
                           struct snd_ctl_elem_value *elem_val)
{
    unsigned int *vals = elem_val->value.enumerated.item;
    char *string;
    unsigned int i;

    for (i = 0; i < channels; i++) {
        string = strings[vals[i]];

        /* The type of a third argument is important. */
        g_array_insert_val(values, i, string);
    }
}

/**
 * alsactl_elem_enum_read:
 * @self: A #ALSACtlElemEnum
 * @values: (element-type utf8) (out caller-allocates): a strings array
 * @exception: A #GError
 *
 */
void alsactl_elem_enum_read(ALSACtlElemEnum *self, GArray *values,
                            GError **exception)
{
    ALSACtlElemEnumPrivate *priv;

    struct snd_ctl_elem_value elem_val = {{0}};

    GValue tmp = G_VALUE_INIT;
    unsigned int channels;

    g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
    priv = alsactl_elem_enum_get_instance_private(self);

    if ((values == NULL) ||
        (g_array_get_element_size(values) != sizeof(gpointer))) {
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
    fill_as_string(values, priv->strings, channels, &elem_val);
}

static void pull_as_string(struct snd_ctl_elem_value *elem_val,
                           unsigned int channels,
                           char (*strings)[CHARS_PER_LABEL],
                           unsigned int labels_count, GArray *values)
{
    unsigned int *vals = elem_val->value.enumerated.item;
    char *string;
    unsigned int i;
    unsigned int j;

    for (i = 0; i < channels; i++) {
        string = (char *)g_array_index(values, gpointer, i);
        for (j = 0; j < labels_count; j++) {
            if (strcmp(string, strings[j]) == 0) {
                vals[i] = j;
                break;
            }
        }
    }
}

/**
 * alsactl_elem_enum_write:
 * @self: A #ALSACtlElemEnum
 * @values: (element-type utf8) (in): a strings array
 * @exception: A #GError
 *
 */
void alsactl_elem_enum_write(ALSACtlElemEnum *self, GArray *values,
                             GError **exception)
{
    ALSACtlElemEnumPrivate *priv;

    struct snd_ctl_elem_value elem_val = {{0}};

    GValue tmp = G_VALUE_INIT;
    unsigned int channels;

    g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
    priv = alsactl_elem_enum_get_instance_private(self);

    if ((values == NULL) ||
        (g_array_get_element_size(values) != sizeof(gpointer)) ||
        values->len == 0) {
        raise(exception, EINVAL);
        return;
    }

    /* Calculate the number of values in this element. */
    g_value_init(&tmp, G_TYPE_UINT);
    g_object_get_property(G_OBJECT(self), "channels", &tmp);
    channels = MIN(values->len, g_value_get_uint(&tmp));

    /* Copy for driver. */
    pull_as_string(&elem_val, channels, priv->strings, priv->labels_count,
                   values);

    alsactl_elem_value_ioctl(ALSACTL_ELEM(self), SNDRV_CTL_IOCTL_ELEM_WRITE,
                             &elem_val, exception);
}
