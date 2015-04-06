#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stddef.h>

#include <sound/asound.h>

#include "client.h"
#include "elem_int.h"
#include "elem_bool.h"
#include "elem_enum.h"
#include "elem_byte.h"
#include "elem_iec60958.h"

/* For error handling. */
G_DEFINE_QUARK("ALSACtlClient", alsactl_client)
#define raise(exception, errno)						\
	g_set_error(exception, alsactl_client_quark(), errno,		\
		    "%d: %s", __LINE__, strerror(errno))

#define NUM_EVENTS	10

typedef struct {
	GSource src;
	ALSACtlClient *client;
	gpointer tag;
} CtlClientSource;

struct _ALSACtlClientPrivate {
	/* To save stack usage. */
	struct snd_ctl_event event[NUM_EVENTS];

	GList *elems;
	GMutex lock;

	CtlClientSource *src;

	int fd;
};
G_DEFINE_TYPE_WITH_PRIVATE(ALSACtlClient, alsactl_client, G_TYPE_OBJECT)
#define CTL_CLIENT_GET_PRIVATE(obj)					\
	(G_TYPE_INSTANCE_GET_PRIVATE((obj),				\
				ALSACTL_TYPE_CLIENT, ALSACtlClientPrivate))

enum ctl_client_prop_type {
	CTL_CLIENT_PROP_NAME = 1,
	CTL_CLIENT_PROP_TYPE,
	CTL_CLIENT_PROP_COUNT,
};
static GParamSpec *ctl_client_props[CTL_CLIENT_PROP_COUNT] = {NULL, };

/* This object has one signal. */
enum ctl_client_sig_type {
	CTL_CLIENT_SIG_ADDED = 0,
	CTL_CLIENT_SIG_COUNT,
};
static guint ctl_client_sigs[CTL_CLIENT_SIG_COUNT] = { 0 };

static void ctl_client_get_property(GObject *obj, guint id,
				    GValue *val, GParamSpec *spec)
{

	switch (id) {
	case CTL_CLIENT_PROP_NAME:
		g_value_set_string(val, "something");
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void ctl_client_set_property(GObject *obj, guint id,
				    const GValue *val, GParamSpec *spec)
{
	G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
}

static void ctl_client_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_client_parent_class)->dispose(obj);
}

static void ctl_client_finalize(GObject *obj)
{
	ALSACtlClient *self = ALSACTL_CLIENT(obj);
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);

	alsactl_client_unlisten(self);
	close(priv->fd);
	priv->fd = 0;

	G_OBJECT_CLASS(alsactl_client_parent_class)->finalize(obj);
}

static void alsactl_client_class_init(ALSACtlClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->get_property = ctl_client_get_property;
	gobject_class->set_property = ctl_client_set_property;
	gobject_class->dispose = ctl_client_dispose;
	gobject_class->finalize = ctl_client_finalize;

	ctl_client_props[CTL_CLIENT_PROP_NAME] =
		g_param_spec_string("name", "name",
				    "The name of this client",
				    "client",
				    G_PARAM_READABLE);
	ctl_client_props[CTL_CLIENT_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type of this client",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	g_object_class_install_properties(gobject_class,
					  CTL_CLIENT_PROP_COUNT,
					  ctl_client_props);

	/**
	 * ALSACtlClient::added:
	 * @self: A #ALSACtlClient
	 * @id: The id of an element newly added
	 */
	ctl_client_sigs[CTL_CLIENT_SIG_ADDED] =
		g_signal_new("added",
			     G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_LAST,
			     0,
			     NULL, NULL,
			     g_cclosure_marshal_VOID__UINT,
			     G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void alsactl_client_init(ALSACtlClient *self)
{
	self->priv = alsactl_client_get_instance_private(self);
	self->priv->elems = NULL;
}

/**
 * alsactl_client_open:
 * @path: a path for the special file of ALSA control device
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlClient
 */
void alsactl_client_open(ALSACtlClient *self, const gchar *path,
			 GError **exception)
{
	ALSACtlClientPrivate *priv;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	priv->fd = open(path, O_RDONLY | O_NONBLOCK);
	if (priv->fd < 0) {
		raise(exception, errno);
		priv->fd = 0;
	}
}

static void allocate_elem_ids(ALSACtlClientPrivate *priv,
			      struct snd_ctl_elem_list *list,
			      GError **exception)
{
	struct snd_ctl_elem_id *ids;

	/* Help for deallocation. */
	memset(list, 0, sizeof(struct snd_ctl_elem_list));

	/* Get the number of elements in this control device. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
		raise(exception, errno);
		return;
	}

	/* No elements found. */
	if (list->count == 0)
		return;

	/* Allocate spaces for these elements. */
	ids = calloc(list->count, sizeof(struct snd_ctl_elem_id));
	if (ids == NULL) {
		raise(exception, ENOMEM);
		return;
	}

	list->offset = 0;
	while (list->offset < list->count) {
		/*
		 * ALSA middleware has limitation of one operation.
		 * 1000 is enought less than the limitation.
		 */
		list->space = MIN(list->count - list->offset, 1000);
		list->pids = ids + list->offset;

		/* Get the IDs of elements in this control device. */
		if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
			raise(exception, errno);
			free(list->pids);
			list->pids = NULL;
		}

		list->offset += list->space;
	}
	list->pids = ids;
	list->space = list->count;
}

static inline void deallocate_elem_ids(struct snd_ctl_elem_list *list)
{
	if (list->pids != NULL)
		free(list->pids);
}

/**
 * alsactl_client_get_elem_list:
 * @self: An #ALSACtlClient
 * @list: (element-type guint)(array)(out caller-allocates): current ID of
 *	elements in this control device.
 * @exception: A #GError
 *
 */
void alsactl_client_get_elem_list(ALSACtlClient *self, GArray *list,
				  GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_list elem_list = {0};
	unsigned int i, count;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Check the size of element in given list. */
	if (g_array_get_element_size(list) != sizeof(guint)) {
		raise(exception, EINVAL);
		return;
	}

	allocate_elem_ids(priv, &elem_list, exception);
	if (*exception != NULL)
		goto end;
	count = elem_list.count;

	/* Return current 'numid' as ID. */
	for (i = 0; i < count; i++)
		g_array_append_val(list, elem_list.pids[i].numid);
end:
	deallocate_elem_ids(&elem_list);
}

static void insert_to_link_list(ALSACtlClient *self, ALSACtlElem *elem)
{
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);

	/* This element has a reference to this client for some operations. */
	g_object_ref(self);

	/* Add this element to the list in this client. */
	g_mutex_lock(&priv->lock);
	priv->elems = g_list_append(priv->elems, elem);
	g_mutex_unlock(&priv->lock);
}

/**
 * alsactl_client_get_elem:
 * @self: A #ALSACtlClient
 * @numid: the numerical id for the element
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlElem
 */
ALSACtlElem *alsactl_client_get_elem(ALSACtlClient *self, guint numid,
				     GError **exception)
{
	GType types[] = {
		[SNDRV_CTL_ELEM_TYPE_BOOLEAN] = ALSACTL_TYPE_ELEM_BOOL,
		[SNDRV_CTL_ELEM_TYPE_INTEGER] = ALSACTL_TYPE_ELEM_INT,
		[SNDRV_CTL_ELEM_TYPE_ENUMERATED] = ALSACTL_TYPE_ELEM_ENUM,
		[SNDRV_CTL_ELEM_TYPE_BYTES] = ALSACTL_TYPE_ELEM_BYTE,
		[SNDRV_CTL_ELEM_TYPE_IEC958] = ALSACTL_TYPE_ELEM_IEC60958,
		[SNDRV_CTL_ELEM_TYPE_INTEGER64] = ALSACTL_TYPE_ELEM_INT,
	};
	ALSACtlClientPrivate *priv;
	ALSACtlElem *elem = NULL;
	struct snd_ctl_elem_list elem_list;
	struct snd_ctl_elem_id *id;
	struct snd_ctl_elem_info info = {{0}};
	unsigned int i, count;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	allocate_elem_ids(priv, &elem_list, exception);
	if (*exception != NULL)
		return NULL;
	count = elem_list.count;

	/* Seek a element indicated by the numerical ID. */
	for (i = 0; i < count; i++) {
		if (elem_list.pids[i].numid == numid)
			break;
	}
	if (i == count) {
		raise(exception, ENODEV);
		goto end;
	}
	id = &elem_list.pids[i];

	info.id = *id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0) {
		raise(exception, errno);
		goto end;
	}

	/* Keep the new instance for this element. */
	elem = g_object_new(types[info.type],
			    "fd", priv->fd,
			    "id", id->numid,
			    "iface", id->iface,
			    "device", id->device,
			    "subdevice", id->subdevice,
			    "name", id->name,
			    NULL);
	elem->_client = g_object_ref(self);

	/* Update the element information. */
	alsactl_elem_update(elem, exception);
	if (*exception != NULL) {
		g_clear_object(&elem);
		elem = NULL;
		goto end;
	}

	/* Insert this element to the list in this client. */
	insert_to_link_list(self, elem);
end:
	deallocate_elem_ids(&elem_list);
	return elem;
}

static void init_info(struct snd_ctl_elem_info *info, gint iface, guint number,
		      const gchar *name, guint count, GError **exception)
{
	/* Check interface type. */
	if (iface < 0 || iface >= SNDRV_CTL_ELEM_IFACE_LAST) {
		raise(exception, EINVAL);
		return;
	}
	info->id.iface = iface;

	/* The number of elements added by this operation. */
	info->owner = number;

	/* Check eleset name. */
	if (name == NULL || strlen(name) >= sizeof(info->id.name)) {
		raise(exception, EINVAL);
		return;
	}
	strcpy((char *)info->id.name, name);

	info->access = SNDRV_CTL_ELEM_ACCESS_USER |
		       SNDRV_CTL_ELEM_ACCESS_READ |
		       SNDRV_CTL_ELEM_ACCESS_WRITE;
	info->count = count;
}

static void add_elems(ALSACtlClient *self, GType type,
		      struct snd_ctl_elem_id *id, unsigned int number,
		      GArray *elems, GError **exception)
{
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);
	struct snd_ctl_elem_info info = {{0}};
	ALSACtlElem *elem;
	unsigned int i;

	/* To get proper numeric ID, sigh... */
	/* TODO: fix upstream. ELEM_ADD ioctl should fill enough info! */
	info.id = *id;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0) {
		raise(exception, errno);
		return;
	}
	*id = info.id;

	/* Keep the new instance for this element. */
	for (i = 0; i < number; i++) {
		elem = g_object_new(type,
				    "fd", priv->fd,
				    "id", id->numid + i,
				    "iface", id->iface,
				    "device", id->device,
				    "subdevice", id->subdevice,
				    "name", id->name,
				    NULL);
		elem->_client = g_object_ref(self);

		/* Update the element information. */
		alsactl_elem_update(elem, exception);
		if (*exception != NULL) {
			g_clear_object(&elem);
			return;
		}

		/* Insert into given array. */
		g_array_insert_val(elems, i, elem);

		/* Insert this element to the list in this client. */
		insert_to_link_list(self, elem);
	}
}

/**
 * alsactl_client_add_int_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @count: the number of values in each element
 * @min: the minimum value for elements in new element
 * @max: the maximum value for elements in new element
 * @step: the step of value for elements in new element
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): element array added by this operation
 * @exception: A #GError
 *
 */
void alsactl_client_add_int_elems(ALSACtlClient *self, gint iface,
				  guint number, const gchar *name,
				  guint count, guint64 min, guint64 max,
				  guint step, GArray *elems,
				  GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	init_info(&info, iface, number, name, count, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	if (max > UINT_MAX)
		info.type = SNDRV_CTL_ELEM_TYPE_INTEGER64;
	else
		info.type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	if (min >= max || (max - min) % step) {
		raise(exception, EINVAL);
		return;
	}
	info.value.integer.min = min;
	info.value.integer.max = max;
	info.value.integer.step = step;

	/* Add this elements. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info) < 0) {
		raise(exception, errno);
		return;
	}

	add_elems(self, ALSACTL_TYPE_ELEM_INT, &info.id, number, elems,
		  exception);
}

/**
 * alsactl_client_add_bool_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @count: the number of values in each element
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 */
void alsactl_client_add_bool_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint count, GArray *elems,
				   GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	init_info(&info, iface, number, name, count, exception);
	if (*exception)
		return;

	/* Type-specific information. */
	info.type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;

	/* Add this elements. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info) < 0) {
		raise(exception, errno);
		return;
	}

	add_elems(self, ALSACTL_TYPE_ELEM_BOOL, &info.id, number, elems,
		  exception);
}

/**
 * alsactl_client_add_enum_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @count: the number of values in each element
 * @items: (element-type utf8): (array) (in): string items for each items
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlElem
 */
void alsactl_client_add_enum_elems(ALSACtlClient *self, gint iface,
				   guint number,  const gchar *name,
				   guint count, GArray *items,
				   GArray *elems, GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};
	unsigned int i;
	unsigned int len;
	char *buf;
	gchar *label;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	init_info(&info, iface, number, name, count, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	info.type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
	info.value.enumerated.items = items->len;

	/* Calcurate total length of items. */
	len = 0;
	for (i = 0; i < items->len; i++)
		len += strlen(g_array_index(items, gchar *, i)) + 1;
	if (len == 0) {
		raise(exception, EINVAL);
		return;
	}

	/* Allocate temporary buffer. */
	buf = malloc(len);
	if (buf == NULL) {
		raise(exception, ENOMEM);
		return;
	}
	memset(buf, 0, len);

	/* Copy items. */
	info.value.enumerated.names_ptr = (uintptr_t)buf;
	info.value.enumerated.names_length = len;
	for (i = 0; i < items->len; i++) {
		label = g_array_index(items, gchar *, i);
		memcpy(buf, label, strlen(label));
		buf += strlen(label) + 1;
	}

	/* Add this elements. */
	err = ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info);
	free((void *)info.value.enumerated.names_ptr);
	if (err < 0) {
		raise(exception, errno);
		return;
	}

	add_elems(self, ALSACTL_TYPE_ELEM_ENUM, &info.id, number, elems,
		  exception);
}

/**
 * alsactl_client_add_byte_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @count: the number of values in each element
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 */
void alsactl_client_add_byte_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint count, GArray *elems,
				   GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	init_info(&info, iface, number, name, count, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	info.type = SNDRV_CTL_ELEM_TYPE_BYTES;

	/* Add this elements. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info) < 0) {
		raise(exception, errno);
		return;
	}

	add_elems(self, ALSACTL_TYPE_ELEM_BYTE, &info.id, number, elems,
		  exception);
}

/**
 * alsactl_client_add_iec60958_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 */
void alsactl_client_add_iec60958_elems(ALSACtlClient *self, gint iface,
				       guint number, const gchar *name,
				       GArray *elems, GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	init_info(&info, iface, number, name, 1, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	info.type = SNDRV_CTL_ELEM_TYPE_IEC958;

	/* Add this elements. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info) < 0) {
		raise(exception, errno);
		return;
	}

	add_elems(self, ALSACTL_TYPE_ELEM_IEC60958, &info.id, number, elems,
		  exception);
}

void alsactl_client_remove_elem(ALSACtlClient *self, ALSACtlElem *elem)
{
	ALSACtlClientPrivate *priv;
	GList *entry;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	g_mutex_lock(&priv->lock);
	for (entry = priv->elems; entry != NULL; entry = entry->next) {
		if (entry->data != elem)
			continue;

		priv->elems = g_list_delete_link(priv->elems, entry);
		g_object_unref(elem->_client);
	}
	g_mutex_unlock(&priv->lock);
}

static gboolean prepare_src(GSource *src, gint *timeout)
{
	/* Use blocking poll(2) to save CPU usage. */
	*timeout = -1;

	/* This source is not ready, let's poll(2) */
	return FALSE;
}

static void handle_elem_event(ALSACtlClient *client, unsigned int event,
			      struct snd_ctl_elem_id *id)
{
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(client);
	GList *entry;
	ALSACtlElem *elems;

	GValue val = G_VALUE_INIT;
	unsigned int numid;

	/* A new element is added. */
	if (event & SNDRV_CTL_EVENT_MASK_ADD) {
		g_signal_emit(G_OBJECT(client),
			      ctl_client_sigs[CTL_CLIENT_SIG_ADDED], 0,
			      id->numid);
	}

	/* No other events except for the add. */
	if (!(event & ~SNDRV_CTL_EVENT_MASK_ADD))
		return;

	/* Deliver the events to elements. */
	g_mutex_lock(&priv->lock);
	g_value_init(&val, G_TYPE_UINT);
	for (entry = priv->elems; entry != NULL; entry = entry->next) {
		elems = (ALSACtlElem *)entry->data;
		if (!ALSACTL_IS_ELEM(elems))
			continue;

		g_object_get_property(G_OBJECT(elems), "id", &val);
		numid = g_value_get_uint(&val);

		/* Here, I check the value of numid only. */
		if (numid != id->numid)
			continue;

		/* The mask of remove event is strange, not mask. */
		if (event == SNDRV_CTL_EVENT_MASK_REMOVE) {
			priv->elems = g_list_delete_link(priv->elems, entry);
			g_signal_emit_by_name(G_OBJECT(elems), "removed",
					      NULL);
			continue;
		}

		if (event & SNDRV_CTL_EVENT_MASK_VALUE)
			g_signal_emit_by_name(G_OBJECT(elems), "changed",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_INFO)
			g_signal_emit_by_name(G_OBJECT(elems), "updated",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_TLV)
			g_signal_emit_by_name(G_OBJECT(elems), "tlv", NULL);
	}
	g_mutex_unlock(&priv->lock);
}

static gboolean check_src(GSource *gsrc)
{
	CtlClientSource *src = (CtlClientSource *)gsrc;
	GIOCondition condition;
	ALSACtlClient *client = src->client;
	ALSACtlClientPrivate *priv;
	unsigned int i;
	unsigned int count;
	int len;

	condition = g_source_query_unix_fd(gsrc, src->tag);
	if (!(condition & G_IO_IN))
		goto end;

	if (!ALSACTL_IS_CLIENT(client))
		goto end;
	priv = CTL_CLIENT_GET_PRIVATE(client);

	len = read(priv->fd, &priv->event, sizeof(priv->event));
	if (len < 0) {
		/* Read error but ignore it. */
		goto end;
	}

	count = len / sizeof(struct snd_ctl_event);
	for (i = 0; i < count; i++) {
		/* Currently ALSA middleware supports 'elem' event only. */
		switch (priv->event[i].type) {
		case SNDRV_CTL_EVENT_ELEM:
			handle_elem_event(client, priv->event[i].data.elem.mask,
					  &priv->event[i].data.elem.id);
			break;
		default:
			break;
		}
	}
end:
	/* Don't go to dispatch, then continue to process this source. */
	return FALSE;
}

static gboolean dispatch_src(GSource *src, GSourceFunc callback,
			     gpointer user_data)
{
	/* Just be sure to continue to process this source. */
	return TRUE;
}

void alsactl_client_listen(ALSACtlClient *self, GError **exception)
{
	static GSourceFuncs funcs = {
		.prepare	= prepare_src,
		.check		= check_src,
		.dispatch	= dispatch_src,
		.finalize	= NULL,
	};
	ALSACtlClientPrivate *priv;
	GSource *src;
	int subscribe;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	src = g_source_new(&funcs, sizeof(CtlClientSource));
	if (src == NULL) {
		raise(exception, ENOMEM);
		return;
	}

	g_source_set_name(src, "ALSACtlClient");
	g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
	g_source_set_can_recurse(src, TRUE);

	((CtlClientSource *)src)->client = self;
	priv->src = (CtlClientSource *)src;

	/* Attach the source to context. */
	g_source_attach(src, g_main_context_default());
	((CtlClientSource *)src)->tag =
				g_source_add_unix_fd(src, priv->fd, G_IO_IN);

	/* Be sure to subscribe events. */
	subscribe = 1;
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &subscribe) < 0) {
		raise(exception, errno);
		alsactl_client_unlisten(self);
	}
}

void alsactl_client_unlisten(ALSACtlClient *self)
{
	ALSACtlClientPrivate *priv;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	if (priv->src == NULL)
		return;

	g_source_destroy((GSource *)priv->src);
	g_free(priv->src);
	priv->src = NULL;
}
