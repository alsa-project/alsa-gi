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

#include "client.h"

typedef struct {
	GSource src;
	ALSACtlClient *client;
	gpointer tag;
} CtlClientSource;

struct _ALSACtlClientPrivate {
	struct snd_ctl_event event;

	GList *elemsets;
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
	ALSACtlClient *self = ALSACTL_CLIENT(obj);
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);

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
	ALSACtlClient *self = ALSACTL_CLIENT(obj);
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);

	alsactl_client_unlisten(self);
	close(priv->fd);
	priv->fd = 0;

	G_OBJECT_CLASS(alsactl_client_parent_class)->dispose(obj);
}

static void ctl_client_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_client_parent_class)->finalize(gobject);
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
	 * @id: The id of element set newly added
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
	self->priv->elemsets = NULL;
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

	priv->fd = open(path, O_RDONLY);
	if (priv->fd < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    errno, "%s", strerror(errno));
		priv->fd = 0;
	}
}

static int allocate_element_ids(ALSACtlClientPrivate *priv,
				struct snd_ctl_elem_list *list)
{
	unsigned int i, count;
	int err;

	/* Help for deallocation. */
	memset(list, 0, sizeof(struct snd_ctl_elem_list));

	/* Get the number of elements in this control device. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0)
		return errno;

	/* No elements found. */
	if (list->count == 0)
		return 0;
	count = list->count;

	/* Allocate spaces for these elements. */
	list->pids = calloc(count, sizeof(struct snd_ctl_elem_id));
	if (list->pids == NULL)
		return ENOMEM;
	list->space = count;

	/* Get the IDs of elements in this control device. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_LIST, list) < 0) {
		free(list->pids);
		return errno;
	}

	return 0;
}

static inline void deallocate_element_ids(struct snd_ctl_elem_list *list)
{
	if (list->pids != NULL)
		free(list->pids);
}

/**
 * alsactl_client_get_elemset_list:
 * @self: An #ALSACtlClient
 * @list: (element-type guint)(array)(out caller-allocates): current ID of
 *	elements in this control device.
 * @exception: A #GError
 *
 */
void alsactl_client_get_elemset_list(ALSACtlClient *self, GArray *list,
				     GError **exception)
{
	ALSACtlClientPrivate *priv;
	struct snd_ctl_elem_list elem_list = {0};
	unsigned int i, count;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Check the size of element in given list. */
	if (g_array_get_element_size(list) != sizeof(guint)) {
		err = EINVAL;
		goto end;
	}

	err = allocate_element_ids(priv, &elem_list);
	if (err > 0)
		goto end;
	count = elem_list.count;

	/* Return current 'numid' as ID. */
	for (i = 0; i < count; i++)
		g_array_append_val(list, elem_list.pids[i].numid);
end:
	deallocate_element_ids(&elem_list);
	if (err > 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    err, "%s", strerror(err));
}

static void insert_to_link_list(ALSACtlClient *self, ALSACtlElemset *elem)
{
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(self);

	/* This element has a reference to this client for some operations. */
	g_object_ref(self);

	/* Add this element to the list in this client. */
	g_mutex_lock(&priv->lock);
	priv->elemsets = g_list_append(priv->elemsets, elem);
	g_mutex_unlock(&priv->lock);
}

/**
 * alsactl_client_get_elemset:
 * @self: A #ALSACtlClient
 * @numid: the numerical id for the element set
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlElemset
 */
ALSACtlElemset *alsactl_client_get_elemset(ALSACtlClient *self, guint numid,
					   GError **exception)
{
	ALSACtlClientPrivate *priv;
	ALSACtlElemset *elemset = NULL;
	struct snd_ctl_elem_list elem_list;
	struct snd_ctl_elem_id *id;
	unsigned int i, count;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	err = allocate_element_ids(priv, &elem_list);
	if (err > 0)
		goto end;
	count = elem_list.count;

	/* Seek a element indicated by the numerical ID. */
	for (i = 0; i < count; i++) {
		if (elem_list.pids[i].numid == numid)
			break;
	}
	if (i == count) {
		err = ENODEV;
		goto end;
	}

	/* Keep the new instance for this element. */
	id = &elem_list.pids[i];
	elemset = g_object_new(ALSACTL_TYPE_ELEMSET,
			       "fd", priv->fd,
			       "id", id->numid,
			       "iface", id->iface,
			       "device", id->device,
			       "subdevice", id->subdevice,
			       "name", id->name,
			       NULL);
	elemset->client = g_object_ref(self);

	/* Update the element information. */
	alsactl_elemset_update(elemset, exception);
	if (*exception != NULL) {
		g_clear_object(&elemset);
		elemset = NULL;
		goto end;
	}

	/* Insert this element to the list in this client. */
	insert_to_link_list(self, elemset);
end:
	deallocate_element_ids(&elem_list);
	if (err > 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    err, "%s", strerror(err));
	return elemset;
}

/**
 * alsactl_client_add_elemset:
 * @self: A #ALSACtlClient
 * @name: the name of new element set
 * @count: the number of elements in new element set
 * @min: the minimum value for elements in new element set
 * @max: the maximum value for elements in new element set
 * @step: the step of value for elements in new element set
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlElemset
 */
ALSACtlElemset *alsactl_client_add_elemset(ALSACtlClient *self, gint iface,
					   const gchar *name, guint count,
					   guint64 min, guint64 max,
					   guint step, GError **exception)
{
	ALSACtlClientPrivate *priv;
	ALSACtlElemset *elemset = NULL;
	struct snd_ctl_elem_info info = {0};
	struct snd_ctl_elem_id *id;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Check interface type. */
	if (iface < 0 || iface >= SNDRV_CTL_ELEM_IFACE_LAST) {
		err = EINVAL;
		goto end;
	}
	info.id.iface = iface;

	/* Check eleset name. */
	if (name == NULL || strlen(name) >= sizeof(info.id.name)) {
		err = EINVAL;
		goto end;
	}
	strcpy(info.id.name, name);

	info.access = SNDRV_CTL_ELEM_ACCESS_READWRITE;
	info.count = count;

	/* Type-specific information. */
	info.type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	if (min >= max || (max - min) % step) {
		err = EINVAL;
		goto end;
	}
	info.value.integer.min = min;
	info.value.integer.max = max;
	info.value.integer.step = step;

	/* Add this element set. */
	if (ioctl(priv->fd, SNDRV_CTL_IOCTL_ELEM_ADD, &info) < 0) {
		err = errno;
		goto end;
	}
	id = &info.id;

	/* Keep the new instance for this element. */
	elemset = g_object_new(ALSACTL_TYPE_ELEMSET,
			       "fd", priv->fd,
			       "id", id->numid,
			       "iface", id->iface,
			       "device", id->device,
			       "subdevice", id->subdevice,
			       "name", id->name,
			       NULL);
	elemset->client = g_object_ref(self);

	/* Update the element information. */
	alsactl_elemset_update(elemset, exception);
	if (*exception != NULL) {
		g_clear_object(&elemset);
		elemset = NULL;
		goto end;
	}

	/* Insert this element to the list in this client. */
	insert_to_link_list(self, elemset);

	err = 0;
end:
	if (err > 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    err, "%s", strerror(err));
	return elemset;
}

void alsactl_client_remove_elem(ALSACtlClient *self, ALSACtlElemset *elemset,
				GError **exception)
{
	ALSACtlClientPrivate *priv;
	GList *entry;
	ALSACtlElemset *e;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	g_mutex_lock(&priv->lock);
	for (entry = priv->elemsets; entry != NULL; entry = entry->next) {
		if (entry->data != elemset)
			continue;

		priv->elemsets = g_list_delete_link(priv->elemsets, entry);
		g_object_unref(self);
	}
	g_mutex_unlock(&priv->lock);
}

static gboolean prepare_src(GSource *src, gint *timeout)
{
	/* Set 2msec for poll(2) timeout. */
	*timeout = 2;

	/* This source is not ready, let's poll(2) */
	return FALSE;
}

static void handle_elem_event(ALSACtlClient *client, unsigned int event,
			      struct snd_ctl_elem_id *id)
{
	ALSACtlClientPrivate *priv = CTL_CLIENT_GET_PRIVATE(client);
	GList *entry;
	ALSACtlElemset *elemset;

	GValue val = G_VALUE_INIT;
	unsigned int numid;

	int err;

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
	for (entry = priv->elemsets; entry != NULL; entry = entry->next) {
		elemset = (ALSACtlElemset *)entry->data;
		if (!ALSACTL_IS_ELEMSET(elemset))
			continue;

		g_object_get_property(G_OBJECT(elemset), "id", &val);
		numid = g_value_get_uint(&val);

		/* Here, I check the value of numid only. */
		if (numid != id->numid)
			continue;

		/* The mask of remove event is strange, not mask. */
		if (event == SNDRV_CTL_EVENT_MASK_REMOVE) {
			priv->elemsets = g_list_delete_link(priv->elemsets,
							    entry);
			g_signal_emit_by_name(G_OBJECT(elemset), "removed",
					      NULL);
			continue;
		}

		if (event & SNDRV_CTL_EVENT_MASK_VALUE)
			g_signal_emit_by_name(G_OBJECT(elemset), "changed",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_INFO)
			g_signal_emit_by_name(G_OBJECT(elemset), "updated",
					      NULL);
		if (event & SNDRV_CTL_EVENT_MASK_TLV)
			g_signal_emit_by_name(G_OBJECT(elemset), "tlv", NULL);
	}
	g_mutex_unlock(&priv->lock);
}

static gboolean check_src(GSource *gsrc)
{
	CtlClientSource *src = (CtlClientSource *)gsrc;
	GIOCondition condition;
	ALSACtlClient *client = src->client;
	ALSACtlClientPrivate *priv;
	int len;

	condition = g_source_query_unix_fd(gsrc, src->tag);
	if (!(condition & G_IO_IN))
		return;

	if (!ALSACTL_IS_CLIENT(client))
		return;
	priv = CTL_CLIENT_GET_PRIVATE(client);

	/* To save stack usage. */
	len = read(priv->fd, &priv->event, sizeof(struct snd_ctl_event));
	if ((len < 0) || (len != sizeof(struct snd_ctl_event))) {
		/* Read error but ignore it. */
		return;
	}

	/* NOTE: currently ALSA middleware supports 'elem' event only. */
	switch (priv->event.type) {
	case SNDRV_CTL_EVENT_ELEM:
		handle_elem_event(client, priv->event.data.elem.mask,
				  &priv->event.data.elem.id);
		break;
	default:
		break;
	}

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
	int err = 0;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	src = g_source_new(&funcs, sizeof(CtlClientSource));
	if (src == NULL) {
		err = ENOMEM;
		goto end;
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
		err = errno;
		alsactl_client_unlisten(self);
	}
end:
	if (err > 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    err, "%s", strerror(err));
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
