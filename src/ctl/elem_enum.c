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

#define CHARS_PER_LABEL	64
#define LABELS_PER_ELEM	1024

struct _ALSACtlElemEnumPrivate {
	unsigned int item_count;
	char (*strings)[CHARS_PER_LABEL];
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlElemEnum, alsactl_elem_enum,
			   ALSACTL_TYPE_ELEM)
#define CTL_ELEM_ENUM_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				     ALSACTL_TYPE_ELEM_ENUM,		\
				     ALSACtlElemEnumPrivate))

static void ctl_elem_enum_dispose(GObject *obj)
{
	ALSACtlElemEnum *self = ALSACTL_ELEM_ENUM(obj);
	ALSACtlElemEnumPrivate *priv = CTL_ELEM_ENUM_GET_PRIVATE(self);

	g_slice_free1(CHARS_PER_LABEL * LABELS_PER_ELEM, priv->strings);

	G_OBJECT_CLASS(alsactl_elem_enum_parent_class)->dispose(obj);
}

static void ctl_elem_enum_finalize(GObject *obj)
{
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
	priv = CTL_ELEM_ENUM_GET_PRIVATE(self);
	strings = priv->strings;

	/* Get the count of items. */
	alsactl_elem_info_ioctl(ALSACTL_ELEM(self), &info, exception);
	if (*exception != NULL)
		return;
	priv->item_count = info.value.enumerated.items;

	/* Set the name of each item. */
	for (i = 0; i < priv->item_count; i++) {
		info.value.enumerated.item = i;

		alsactl_elem_info_ioctl(ALSACTL_ELEM(self), &info,
					exception);
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
	
	gobject_class->dispose = ctl_elem_enum_dispose;
	gobject_class->finalize = ctl_elem_enum_finalize;
}

static void alsactl_elem_enum_init(ALSACtlElemEnum *self)
{
	self->priv = alsactl_elem_enum_get_instance_private(self);

	/*
	 * The maximum length of each item is 64 characters.
	 * The maximum number of items in one element is 1024 entries.
	 */
	self->priv->strings = (char (*)[CHARS_PER_LABEL])
			g_slice_alloc0(CHARS_PER_LABEL * LABELS_PER_ELEM);
}

/**
 * alsactl_elem_enum_get_items:
 * @self: A #ALSACtlElemEnum
 * @items: (element-type utf8) (out caller-allocates) (transfer container): a strings array
 * @exception: A #GError
 *
 */
void alsactl_elem_enum_get_items(ALSACtlElemEnum *self, GArray *items,
				 GError **exception)
{
	ALSACtlElemEnumPrivate *priv;
	char (*strings)[CHARS_PER_LABEL];
	char *string;
	unsigned int i;

	g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
	priv = CTL_ELEM_ENUM_GET_PRIVATE(self);
	strings = priv->strings;

	for (i = 0; i < priv->item_count; i++) {
		string = strings[i];

		/* The type of last parameter is important. */
		g_array_insert_val(items, i, string);
	}
}

static void fill_as_string(GArray *values, char (*strings)[CHARS_PER_LABEL],
			   unsigned int count,
			   struct snd_ctl_elem_value *elem_val)
{
	unsigned int *vals = elem_val->value.enumerated.item;
	char *string;
	unsigned int i;

	for (i = 0; i < count; i++) {
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
	unsigned int count;

	g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
	priv = CTL_ELEM_ENUM_GET_PRIVATE(self);

	if ((values == NULL) ||
	    (g_array_get_element_size(values) != sizeof(gpointer))) {
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
	g_value_init(&tmp, G_TYPE_UINT);
	g_object_get_property(G_OBJECT(self), "count", &tmp);
	count = g_value_get_uint(&tmp);

	/* Copy for application. */
	fill_as_string(values, priv->strings, count, &elem_val);
}

static void pull_as_string(struct snd_ctl_elem_value *elem_val,
			   unsigned int count, char (*strings)[CHARS_PER_LABEL],
			   unsigned int item_count, GArray *values)
{
	unsigned int *vals = elem_val->value.enumerated.item;
	char *string;
	unsigned int i;
	unsigned int j;

	for (i = 0; i < count; i++) {
		string = (char *)g_array_index(values, gpointer, i);
		for (j = 0; j < item_count; j++) {
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
	unsigned int count;

	g_return_if_fail(ALSACTL_IS_ELEM_ENUM(self));
	priv = CTL_ELEM_ENUM_GET_PRIVATE(self);

	if ((values == NULL) ||
	    (g_array_get_element_size(values) != sizeof(gpointer)) ||
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
	pull_as_string(&elem_val, count, priv->strings, priv->item_count,
		       values);

	alsactl_elem_value_ioctl(ALSACTL_ELEM(self),
				 SNDRV_CTL_IOCTL_ELEM_WRITE, &elem_val,
				 exception);
}
