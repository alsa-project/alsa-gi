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
#define client_raise(exception, errno)					\
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

enum ctl_client_prop_type {
	CTL_CLIENT_PROP_NAME = 1,
	CTL_CLIENT_PROP_TYPE,
	CTL_CLIENT_PROP_COUNT,
};
static GParamSpec *ctl_client_props[CTL_CLIENT_PROP_COUNT] = {NULL, };

/* This object has one signal. */
enum ctl_client_sig_type {
	CTL_CLIENT_SIG_ADDED = 0,
	CTL_CLIENT_SIG_INSERT,
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

static void ctl_client_finalize(GObject *obj)
{
	ALSACtlClient *self = ALSACTL_CLIENT(obj);
	ALSACtlClientPrivate *priv = alsactl_client_get_instance_private(self);

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
	ALSACtlClientPrivate *priv = alsactl_client_get_instance_private(self);
	priv->elems = NULL;
}

/**
 * alsactl_client_open:
 * @path: a path for the special file of ALSA control device
 * @exception: A #GError
 */
void alsactl_client_open(ALSACtlClient *self, const gchar *path,
			 GError **exception)
{
	ALSACtlClientPrivate *priv;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = alsactl_client_get_instance_private(self);

	priv->fd = open(path, O_RDONLY | O_NONBLOCK);
	if (priv->fd < 0) {
		client_raise(exception, errno);
		priv->fd = 0;
	}
}

static int client_ioctl(ALSACtlClient *self, unsigned long request, void *arg)
{
	if (!ALSACTL_IS_CLIENT(self)) {
		errno = EINVAL;
		return -1;
	}
	ALSACtlClientPrivate *priv = alsactl_client_get_instance_private(self);
	return ioctl(priv->fd, request, arg);
}

static void allocate_elem_ids(ALSACtlClient *self,
			      struct snd_ctl_elem_list *list,
			      GError **exception)
{
	struct snd_ctl_elem_id *ids;

	/* Help for deallocation. */
	memset(list, 0, sizeof(struct snd_ctl_elem_list));

	/* Get the number of elements in this control device. */
	if (client_ioctl(self, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
		client_raise(exception, errno);
		return;
	}

	/* No elements found. */
	if (list->count == 0)
		return;

	/* Allocate spaces for these elements. */
	ids = calloc(list->count, sizeof(struct snd_ctl_elem_id));
	if (ids == NULL) {
		client_raise(exception, ENOMEM);
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
		if (client_ioctl(self, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
			client_raise(exception, errno);
			free(ids);
			list->pids = NULL;
			return;
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
	struct snd_ctl_elem_list elem_list = {0};
	unsigned int i;

	/* Check the size of element in given list. */
	if (g_array_get_element_size(list) != sizeof(guint)) {
		client_raise(exception, EINVAL);
		return;
	}

	allocate_elem_ids(self, &elem_list, exception);
	if (*exception != NULL)
		return;

	/* Return current 'numid' as ID. */
	for (i = 0; i < elem_list.count; i++)
		g_array_append_val(list, elem_list.pids[i].numid);

	deallocate_elem_ids(&elem_list);
}

static ALSACtlElem *insert_elem_to_cache(ALSACtlClient *self,
					 snd_ctl_elem_type_t type,
					 struct snd_ctl_elem_id *id,
					 GError **exception)
{
	GType types[] = {
		[SNDRV_CTL_ELEM_TYPE_BOOLEAN]	= ALSACTL_TYPE_ELEM_BOOL,
		[SNDRV_CTL_ELEM_TYPE_INTEGER]	= ALSACTL_TYPE_ELEM_INT,
		[SNDRV_CTL_ELEM_TYPE_ENUMERATED] = ALSACTL_TYPE_ELEM_ENUM,
		[SNDRV_CTL_ELEM_TYPE_BYTES]	= ALSACTL_TYPE_ELEM_BYTE,
		[SNDRV_CTL_ELEM_TYPE_IEC958]	= ALSACTL_TYPE_ELEM_IEC60958,
		[SNDRV_CTL_ELEM_TYPE_INTEGER64] = ALSACTL_TYPE_ELEM_INT,
	};
	ALSACtlClientPrivate *priv = alsactl_client_get_instance_private(self);
	ALSACtlElem *elem;

	elem = g_object_new(types[type],
			    "client", self,
			    "id", id->numid,
			    "iface", id->iface,
			    "device", id->device,
			    "subdevice", id->subdevice,
			    "name", id->name,
			    NULL);

	/* Update the element information. */
	alsactl_elem_update(elem, exception);
	if (*exception != NULL) {
		g_clear_object(&elem);
		return NULL;
	}

	/* Insert this element to the list in this client. */
	g_mutex_lock(&priv->lock);
	priv->elems = g_list_append(priv->elems, elem);
	g_mutex_unlock(&priv->lock);

	return elem;
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
	ALSACtlElem *elem = NULL;
	struct snd_ctl_elem_list elem_list;
	struct snd_ctl_elem_id *id;
	struct snd_ctl_elem_info info = {{0}};
	unsigned int i;

	g_return_val_if_fail(ALSACTL_IS_CLIENT(self), NULL);

	allocate_elem_ids(self, &elem_list, exception);
	if (*exception != NULL)
		return NULL;

	/* Seek a element indicated by the numerical ID. */
	for (i = 0; i < elem_list.count; i++) {
		if (elem_list.pids[i].numid == numid)
			break;
	}
	if (i == elem_list.count) {
		client_raise(exception, ENODEV);
		goto end;
	}
	id = &elem_list.pids[i];

	info.id = *id;
	if (client_ioctl(self, SNDRV_CTL_IOCTL_ELEM_INFO, &info) < 0) {
		client_raise(exception, errno);
		goto end;
	}

	elem = insert_elem_to_cache(self, info.type, id, exception);
end:
	deallocate_elem_ids(&elem_list);
	return elem;
}

static void init_info(struct snd_ctl_elem_info *info, snd_ctl_elem_type_t type,
		      snd_ctl_elem_iface_t iface, guint number,
		      const gchar *name, guint count, const GArray *dimen,
		      GError **exception)
{
	unsigned int i;
	unsigned int val;

	/* Check element type. */
	if (type < SNDRV_CTL_ELEM_TYPE_BOOLEAN ||
	    type >= SNDRV_CTL_ELEM_TYPE_LAST) {
		client_raise(exception, EINVAL);
		return;
	}
	info->type = type;

	/* Check interface type. */
	if (iface < 0 || iface >= SNDRV_CTL_ELEM_IFACE_LAST) {
		client_raise(exception, EINVAL);
		return;
	}
	info->id.iface = iface;

	/* The number of elements added by this operation. */
	info->owner = number;

	/* Check eleset name. */
	if (name == NULL || strlen(name) >= sizeof(info->id.name)) {
		client_raise(exception, EINVAL);
		return;
	}
	strcpy((char *)info->id.name, name);

	/* Check the number of elements in the array for dimension. */
	if (dimen != NULL) {
		if (dimen->len > G_N_ELEMENTS(info->dimen.d)) {
			client_raise(exception, EINVAL);
			return;
		}
		for (i = 0; i < dimen->len; i++) {
			val = g_array_index(dimen, gushort, i);
			if (val > count) {
				client_raise(exception, EINVAL);
				return;
			}
			info->dimen.d[i] = val;
		}
	}

	info->access = SNDRV_CTL_ELEM_ACCESS_USER |
		       SNDRV_CTL_ELEM_ACCESS_READ |
		       SNDRV_CTL_ELEM_ACCESS_WRITE;
	info->count = count;
}

static void add_elems(ALSACtlClient *self, GType type,
		      struct snd_ctl_elem_info *info, unsigned int elem_count,
		      GArray *elems, GError **exception)
{
	struct snd_ctl_elem_id id;
	ALSACtlElem *elem;
	unsigned int i;

	/* Add this elements. */
	if (client_ioctl(self, SNDRV_CTL_IOCTL_ELEM_ADD, info) < 0) {
		client_raise(exception, errno);
		return;
	}

	/* Keep the new instance for this element. */
	for (i = 0; i < elem_count; i++) {
		id = info->id;
		id.numid += i;
		id.index += i;
		elem = insert_elem_to_cache(self, type, &id, exception);
		if (*exception != NULL) {
			/* Cacheed objects are released by I/O handler. */
			client_ioctl(self, SNDRV_CTL_IOCTL_ELEM_REMOVE,
				     &info->id);
			break;
		}

		/* Insert into given array. */
		g_array_insert_val(elems, i, elem);
	}
}

/**
 * alsactl_client_add_int_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @channels: the number of channelss in each element
 * @min: the minimum value for elements in new element
 * @max: the maximum value for elements in new element
 * @step: the step of value for elements in new element
 * @dimen: (element-type gushort) (array) (in) (nullable): dimension array with 4 elements
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): element array added by this operation
 * @exception: A #GError
 *
 */
void alsactl_client_add_int_elems(ALSACtlClient *self, gint iface,
				  guint number, const gchar *name,
				  guint channels, guint64 min, guint64 max,
				  guint step, const GArray *dimen,
				  GArray *elems, GError **exception)
{
	struct snd_ctl_elem_info info = {{0}};
	snd_ctl_elem_type_t type;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));

	if (max > UINT_MAX)
		type = SNDRV_CTL_ELEM_TYPE_INTEGER64;
	else
		type = SNDRV_CTL_ELEM_TYPE_INTEGER;

	init_info(&info, type, iface, number, name, channels, dimen, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	if (min >= max || (max - min) % step) {
		client_raise(exception, EINVAL);
		return;
	}
	info.value.integer.min = min;
	info.value.integer.max = max;
	info.value.integer.step = step;

	add_elems(self, type, &info, number, elems, exception);
}

/**
 * alsactl_client_add_bool_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @channels: the number of values in each element
 * @dimen: (element-type gushort) (array) (in) (nullable): dimension array with 4 elements
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 */
void alsactl_client_add_bool_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint channels, const GArray *dimen,
				   GArray *elems, GError **exception)
{
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));

	init_info(&info, SNDRV_CTL_ELEM_TYPE_BOOLEAN, iface, number, name,
		  channels, dimen, exception);
	if (*exception != NULL)
		return;

	add_elems(self, SNDRV_CTL_ELEM_TYPE_BOOLEAN, &info, number, elems,
		  exception);
}

/**
 * alsactl_client_add_enum_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @channels: the number of values in each element
 * @items: (element-type utf8): (array) (in): string items for each items
 * @dimen: (element-type gushort) (array) (in) (nullable): dimension array with 4 elements
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 */
void alsactl_client_add_enum_elems(ALSACtlClient *self, gint iface,
				   guint number,  const gchar *name,
				   guint channels, GArray *items,
				   const GArray *dimen,
				   GArray *elems, GError **exception)
{
	struct snd_ctl_elem_info info = {{0}};
	unsigned int i;
	unsigned int len;
	char *buf;
	gchar *label;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));

	init_info(&info, SNDRV_CTL_ELEM_TYPE_ENUMERATED, iface, number, name,
		  channels, dimen, exception);
	if (*exception != NULL)
		return;

	/* Type-specific information. */
	info.value.enumerated.items = items->len;

	/* Calcurate total length of items. */
	len = 0;
	for (i = 0; i < items->len; i++)
		len += strlen(g_array_index(items, gchar *, i)) + 1;
	if (len == 0) {
		client_raise(exception, EINVAL);
		return;
	}

	/* Allocate temporary buffer. */
	buf = malloc(len);
	if (buf == NULL) {
		client_raise(exception, ENOMEM);
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

	add_elems(self, SNDRV_CTL_ELEM_TYPE_ENUMERATED, &info, number, elems,
		  exception);
}

/**
 * alsactl_client_add_byte_elems:
 * @self: A #ALSACtlClient
 * @iface: the type of interface
 * @number: the number of elements added by this operation
 * @name: the name of new elements
 * @channels: the number of values in each element
 * @dimen: (element-type gushort) (array) (in) (nullable): dimension array with 4 elements
 * @elems: (element-type ALSACtlElem) (array) (out caller-allocates) (transfer container): hoge
 * @exception: A #GError
 *
 */
void alsactl_client_add_byte_elems(ALSACtlClient *self, gint iface,
				   guint number, const gchar *name,
				   guint channels, const GArray *dimen,
				   GArray *elems, GError **exception)
{
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));

	init_info(&info, SNDRV_CTL_ELEM_TYPE_BYTES, iface, number, name,
		  channels, dimen, exception);
	if (*exception != NULL)
		return;

	add_elems(self, SNDRV_CTL_ELEM_TYPE_BYTES, &info, number, elems,
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
	struct snd_ctl_elem_info info = {{0}};

	g_return_if_fail(ALSACTL_IS_CLIENT(self));

	init_info(&info, SNDRV_CTL_ELEM_TYPE_IEC958, iface, number, name, 1,
		  NULL, exception);
	if (*exception != NULL)
		return;

	add_elems(self, SNDRV_CTL_ELEM_TYPE_IEC958, &info, number, elems,
		  exception);
}

void alsactl_client_remove_elem(ALSACtlClient *self, ALSACtlElem *elem)
{
	ALSACtlClientPrivate *priv;
	GList *entry;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = alsactl_client_get_instance_private(self);

	g_mutex_lock(&priv->lock);
	for (entry = priv->elems; entry != NULL; entry = entry->next) {
		if (entry->data != elem)
			continue;

		priv->elems = g_list_delete_link(priv->elems, entry);
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
	ALSACtlClientPrivate *priv = alsactl_client_get_instance_private(client);
	GList *entry;
	ALSACtlElem *elem;

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
		elem = (ALSACtlElem *)entry->data;
		if (!ALSACTL_IS_ELEM(elem))
			continue;

		g_object_get_property(G_OBJECT(elem), "id", &val);
		numid = g_value_get_uint(&val);

		/* Here, I check the value of numid only. */
		if (numid != id->numid)
			continue;

		/* The mask of remove event is strange, not mask. */
		if (event == SNDRV_CTL_EVENT_MASK_REMOVE) {
			priv->elems = g_list_delete_link(priv->elems, entry);
			g_signal_emit_by_name(G_OBJECT(elem), "removed",
					      NULL);
			continue;
		}

		if (event & SNDRV_CTL_EVENT_MASK_VALUE)
			g_signal_emit_by_name(G_OBJECT(elem), "changed",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_INFO)
			g_signal_emit_by_name(G_OBJECT(elem), "updated",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_TLV)
			g_signal_emit_by_name(G_OBJECT(elem), "tlv", NULL);
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
	priv = alsactl_client_get_instance_private(client);

	len = read(priv->fd, &priv->event, sizeof(priv->event));
	if (len < 0) {
		/* Read error but ignore it. */
		goto end;
	}

	count = len / sizeof(struct snd_ctl_event);
	for (i = 0; i < count; i++) {
		/* Currently ALSA middleware supports 'elem' event only. */
		if (priv->event[i].type != SNDRV_CTL_EVENT_ELEM)
			continue;

		handle_elem_event(client, priv->event[i].data.elem.mask,
				  &priv->event[i].data.elem.id);
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
	priv = alsactl_client_get_instance_private(self);

	src = g_source_new(&funcs, sizeof(CtlClientSource));
	if (src == NULL) {
		client_raise(exception, ENOMEM);
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
		client_raise(exception, errno);
		alsactl_client_unlisten(self);
	}
}

void alsactl_client_unlisten(ALSACtlClient *self)
{
	ALSACtlClientPrivate *priv;
	int subscribe;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = alsactl_client_get_instance_private(self);

	if (priv->src == NULL)
		return;

	/* Be sure to unsubscribe events. */
	subscribe = 0;
	ioctl(priv->fd, SNDRV_CTL_IOCTL_SUBSCRIBE_EVENTS, &subscribe);

	g_source_destroy((GSource *)priv->src);
	g_free(priv->src);
	priv->src = NULL;
}

/* For error handling. */
G_DEFINE_QUARK("ALSACtlElem", alsactl_elem)
#define elem_raise(exception, errno)					\
	g_set_error(exception, alsactl_elem_quark(), errno,		\
		    "%d: %s", __LINE__, strerror(errno))

struct _ALSACtlElemPrivate {
	ALSACtlClient *client;
	int fd;
	struct snd_ctl_elem_info info;
	GArray *dimen;
};
G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(ALSACtlElem, alsactl_elem, G_TYPE_OBJECT)

enum ctl_elem_prop_type {
	CTL_ELEM_PROP_CLIENT = 1,
	CTL_ELEM_PROP_TYPE,
	CTL_ELEM_PROP_CHANNELS,
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
	CTL_ELEM_PROP_DIMENSION,
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
	ALSACtlElemPrivate *priv = alsactl_elem_get_instance_private(self);

	switch (id) {
	case CTL_ELEM_PROP_TYPE:
		g_value_set_int(val, priv->info.type);
		break;
	case CTL_ELEM_PROP_CHANNELS:
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
	case CTL_ELEM_PROP_DIMENSION:
		g_value_set_static_boxed(val, priv->dimen);
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
	ALSACtlElemPrivate *priv = alsactl_elem_get_instance_private(self);

	switch (id) {
	/* These should be set by constructor. */
	case CTL_ELEM_PROP_CLIENT:
		priv->client = g_object_ref(g_value_get_object(val));
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

static void ctl_elem_finalize(GObject *obj)
{
	ALSACtlElem *self = ALSACTL_ELEM(obj);
	ALSACtlElemPrivate *priv = alsactl_elem_get_instance_private(self);
	GError *exception = NULL;

	/* Leave ownership to release this elemset. */
	alsactl_elem_unlock(self, &exception);
	if (exception != NULL)
		g_error_free(exception);

	/* Remove this element as long as no processes owns. */
	if (!(priv->info.access & SNDRV_CTL_ELEM_ACCESS_OWNER))
		client_ioctl(priv->client, SNDRV_CTL_IOCTL_ELEM_REMOVE,
			     &priv->info.id);

	g_array_free(priv->dimen, TRUE);

	alsactl_client_remove_elem(priv->client, self);
	g_object_unref(priv->client);

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
	gobject_class->finalize = ctl_elem_finalize;

	ctl_elem_props[CTL_ELEM_PROP_CLIENT] =
		g_param_spec_object("client", "client",
				    "Instance of client who has this element",
				    ALSACTL_TYPE_CLIENT,
				    G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
	ctl_elem_props[CTL_ELEM_PROP_TYPE] =
		g_param_spec_int("type", "type",
				 "The type of this element",
				 0, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	ctl_elem_props[CTL_ELEM_PROP_CHANNELS] =
		g_param_spec_uint("channels", "channels",
				  "The number of channels in this element",
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
	ctl_elem_props[CTL_ELEM_PROP_DIMENSION] =
		g_param_spec_boxed("dimension", "dimension",
				   "When channels construct matrix, return an"
				   "array filled with elements in each level",
				   G_TYPE_ARRAY,
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
	ALSACtlElemPrivate *priv = alsactl_elem_get_instance_private(self);
	priv->dimen = g_array_sized_new(FALSE, TRUE, sizeof(gushort),
					G_N_ELEMENTS(priv->info.dimen.d));
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
	priv = alsactl_elem_get_instance_private(self);

	id = &priv->info.id;

	if (client_ioctl(priv->client, SNDRV_CTL_IOCTL_ELEM_LOCK, id) < 0)
		elem_raise(exception, errno);
	else
		alsactl_elem_update(self, exception);
}

void alsactl_elem_unlock(ALSACtlElem *self, GError **exception)
{
	ALSACtlElemPrivate *priv;
	struct snd_ctl_elem_id *id;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = alsactl_elem_get_instance_private(self);

	id = &priv->info.id;

	if (client_ioctl(priv->client, SNDRV_CTL_IOCTL_ELEM_UNLOCK, id) >= 0)
		alsactl_elem_update(self, exception);
	else if (errno != -EINVAL)
		elem_raise(exception, errno);
}

void alsactl_elem_value_ioctl(ALSACtlElem *self, int cmd,
			      struct snd_ctl_elem_value *elem_val,
			      GError **exception)
{
	ALSACtlElemPrivate *priv;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = alsactl_elem_get_instance_private(self);

	elem_val->id.numid = priv->info.id.numid;

	if (client_ioctl(priv->client, cmd, elem_val) < 0)
		elem_raise(exception, errno);
}

void alsactl_elem_info_ioctl(ALSACtlElem *self, struct snd_ctl_elem_info *info,
			     GError **exception)
{
	ALSACtlElemPrivate *priv;
	unsigned int i;

	g_return_if_fail(ALSACTL_IS_ELEM(self));
	priv = alsactl_elem_get_instance_private(self);

	info->id.numid = priv->info.id.numid;

	if (client_ioctl(priv->client, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0)
		elem_raise(exception, errno);

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

	/* Update dimension information. */
	g_array_set_size(priv->dimen, 0);
	for (i = 0; i < G_N_ELEMENTS(priv->info.dimen.d); i++)
		g_array_insert_val(priv->dimen, i, priv->info.dimen.d[i]);
}
