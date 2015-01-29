#include <unistd.h>

#include "client.h"

typedef struct {
	GSource src;
	ALSACtlClient *client;
	gpointer tag;
} CtlClientSource;

struct _ALSACtlClientPrivate {
	snd_ctl_event_t *event;

	GList *elemsets;
	GMutex lock;

	CtlClientSource *src;
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
		g_value_set_string(val, snd_ctl_name(self->handle));
		break;
	case CTL_CLIENT_PROP_TYPE:
		g_value_set_int(val, snd_ctl_type(self->handle));
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

	alsactl_client_unlisten(self);

	snd_ctl_close(self->handle);

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
 * @node: the name for ALSA control node
 * @exception: A #GError
 *
 * Returns: (transfer full): A #ALSACtlClient
 */
ALSACtlClient *alsactl_client_open(const gchar *node, GError **exception)
{
	ALSACtlClient *self;
	ALSACtlClientPrivate *priv;
	snd_ctl_t *handle;
	int err;

	err = snd_ctl_open(&handle, node, 0);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}

	self = g_object_new(ALSACTL_TYPE_CLIENT, NULL);
	self->handle = handle;

	return self;
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
	snd_ctl_elem_list_t *l = NULL;
	guint id;
	unsigned int i, count;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Check the size of element in given list. */
	if (g_array_get_element_size(list) != sizeof(guint)) {
		err = -EINVAL;
		goto end;
	}

	/* Use stack. */
	snd_ctl_elem_list_alloca(&l);

	/* Get the number of elements in this control device. */
	err = snd_ctl_elem_list(self->handle, l);
	if (err < 0)
		goto end;

	count = snd_ctl_elem_list_get_count(l);
	if (count == 0)
		goto end;

	/* Use heap. */
	err = snd_ctl_elem_list_alloc_space(l, count);
	if (err < 0)
		goto end;

	/* Get the IDs of elements in this control device. */
	err = snd_ctl_elem_list(self->handle, l);
	if (err < 0)
		goto end;

	/* Return current 'numid' as ID. */
	for (i = 0; i < count; i++) {
		id = snd_ctl_elem_list_get_numid(l, i);
		g_array_append_val(list, id);
	}
end:
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		count = 0;
	}
	/* The value of 'used' means how many allocated entries are used. */
	if (l != NULL & snd_ctl_elem_list_get_used(l))
		snd_ctl_elem_list_free_space(l);
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

static int fill_id_by_numid(ALSACtlClient *self, snd_ctl_elem_id_t *id,
			    unsigned int numid)
{
	snd_ctl_elem_list_t *list;
	unsigned int i, count;
	int err;

	/* Use stack. */
	snd_ctl_elem_list_alloca(&list);

	/* Get the number of elements in this control device. */
	err = snd_ctl_elem_list(self->handle, list);
	if (err < 0)
		goto end;

	count = snd_ctl_elem_list_get_count(list);
	if (count == 0)
		goto end;

	/* Use heap. */
	err = snd_ctl_elem_list_alloc_space(list, count);
	if (err < 0)
		goto end;

	/* Get the IDs of elements in this control device. */
	err = snd_ctl_elem_list(self->handle, list);
	if (err < 0)
		goto end;

	/* Seek an element with the ID. */
	for (i = 0; i < count; i++) {
		if (snd_ctl_elem_list_get_numid(list, i) == numid)
			break;
	}
	if (i == count) {
		err = -ENODEV;
		goto end;
	}

	/* Set this ID. */
	snd_ctl_elem_list_get_id(list, i, id);
end:
	if (snd_ctl_elem_list_get_used(list) > 0)
		snd_ctl_elem_list_free_space(list);
	return err;
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
	snd_ctl_elem_id_t *id;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Fill ID by given id. */
	snd_ctl_elem_id_alloca(&id);
	err = fill_id_by_numid(self, id, numid);
	if (err < 0)
		goto end;

	/* Keep the new instance for this element. */
	elemset = g_object_new(ALSACTL_TYPE_ELEMSET,
			       "id", snd_ctl_elem_id_get_numid(id),
			       "iface", snd_ctl_elem_id_get_interface(id),
			       "device", snd_ctl_elem_id_get_device(id),
			       "subdevice", snd_ctl_elem_id_get_subdevice(id),
			       "name", snd_ctl_elem_id_get_name(id),
			       NULL);
	elemset->client = g_object_ref(self);

	/* Update the element information. */
	alsactl_elemset_update(elemset, exception);
	if (*exception != NULL)
		goto end;

	/* Insert this element to the list in this client. */
	insert_to_link_list(self, elemset);
end:
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	if (*exception != NULL) {
		if (elemset != NULL)
			g_clear_object(&elemset);
		elemset == NULL;
	}
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
					   guint64 min, guint64 max, guint step,
					   GError **exception)
{
	ALSACtlClientPrivate *priv;
	ALSACtlElemset *elemset = NULL;
	snd_ctl_elem_id_t *id;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* Check arguments. */
	if (iface < 0 || iface >= SND_CTL_ELEM_IFACE_LAST ||
	    name == NULL || strlen(name) >= 43 ||
	    count == 0 || count >= 32 || min >= max || (max - min) % step) {
		err = -EINVAL;
		goto end;
	}

	/* The name is required. */
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_id_set_name(id, name);
	snd_ctl_elem_id_set_interface(id, iface);
	err = snd_ctl_elem_add_integer(self->handle, id, count, min, max, step);
	if (err < 0)
		goto end;

	/* Keep the new instance for this element. */
	elemset = g_object_new(ALSACTL_TYPE_ELEMSET,
			       "id", snd_ctl_elem_id_get_numid(id),
			       "iface", snd_ctl_elem_id_get_interface(id),
			       "device", snd_ctl_elem_id_get_device(id),
			       "subdevice", snd_ctl_elem_id_get_subdevice(id),
			       "name", snd_ctl_elem_id_get_name(id),
			       NULL);
	elemset->client = g_object_ref(self);

	/* Update the element information. */
	alsactl_elemset_update(elemset, exception);
	if (*exception != NULL) {
		goto end;
	}

	/* Insert this element to the list in this client. */
	insert_to_link_list(self, elemset);
end:
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	if (*exception != NULL) {
		if (elemset != NULL)
			g_clear_object(&elemset);
		elemset = NULL;
	}
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

static gboolean check_src(GSource *gsrc)
{
	CtlClientSource *src = (CtlClientSource *)gsrc;
	GIOCondition condition;

	ALSACtlClient *client = src->client;
	ALSACtlClientPrivate *priv;

	GList *entry;
	ALSACtlElemset *elemset;
	unsigned int mask;

	GValue val = G_VALUE_INIT;
	unsigned int numid_ev, numid_elem;
	unsigned int index;

	int err;

	condition = g_source_query_unix_fd(gsrc, src->tag);
	if (!(condition & G_IO_IN))
		return;

	if ((client == NULL) || !ALSACTL_IS_CLIENT(client))
		return;
	priv = CTL_CLIENT_GET_PRIVATE(client);

	snd_ctl_event_clear(priv->event);
	err = snd_ctl_read(client->handle, priv->event);
	if (err < 0)
		goto end;

	/* NOTE: currently ALSA middleware supports 'elem' only. */
	if (snd_ctl_event_get_type(priv->event) != SND_CTL_EVENT_ELEM)
		goto end;

	/* Retrieve event mask */
	mask = snd_ctl_event_elem_get_mask(priv->event);
	numid_ev = snd_ctl_event_elem_get_numid(priv->event);
	index = snd_ctl_event_elem_get_index(priv->event);

	/* A new element is added. . */
	if (mask & SND_CTL_EVENT_MASK_ADD) {
		/* NOTE: The numid is always zero, maybe bug. */
		g_signal_emit(G_OBJECT(client),
			      ctl_client_sigs[CTL_CLIENT_SIG_ADDED], 0,
			      numid_ev);
	}

	/* No other events except for the add. */
	if (!(mask & ~SND_CTL_EVENT_MASK_ADD))
		goto end;

	/* Deliver the events to elements. */
	g_mutex_lock(&priv->lock);
	g_value_init(&val, G_TYPE_UINT);
	for (entry = priv->elemsets; entry != NULL; entry = entry->next) {
		elemset = (ALSACtlElemset *)entry->data;
		if (!ALSACTL_IS_ELEMSET(elemset))
			continue;

		g_object_get_property(G_OBJECT(elemset), "id", &val);
		numid_elem = g_value_get_uint(&val);

		/* Here, I check the value of numid only. */
		if (numid_elem != numid_ev)
			continue;

		/* The mask of remove event is strange, not mask. */
		if (mask == SND_CTL_EVENT_MASK_REMOVE) {
			priv->elemsets = g_list_delete_link(priv->elemsets,
							    entry);
			g_signal_emit_by_name(G_OBJECT(elemset), "removed",
					      NULL);
			continue;
		}

		if (mask & SND_CTL_EVENT_MASK_VALUE)
			g_signal_emit_by_name(G_OBJECT(elemset), "changed",
					      NULL);
		if (mask & SND_CTL_EVENT_MASK_INFO)
			g_signal_emit_by_name(G_OBJECT(elemset), "updated",
					      NULL);
		if (mask & SND_CTL_EVENT_MASK_TLV)
			g_signal_emit_by_name(G_OBJECT(elemset), "tlv", NULL);
	}
	g_mutex_unlock(&priv->lock);
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
	struct pollfd pfds;
	GSource *src;
	int err;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	/* This library doesn't support a handle with multi-descriptor. */
	if (snd_ctl_poll_descriptors_count(self->handle) != 1) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	if (snd_ctl_poll_descriptors(self->handle, &pfds, 1) != 1) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    EINVAL, "%s", strerror(EINVAL));
		return;
	}

	src = g_source_new(&funcs, sizeof(CtlClientSource));
	if (src == NULL) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    ENOMEM, "%s", strerror(ENOMEM));
		return;
	}

	g_source_set_name(src, "ALSACtlClient");
	g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
	g_source_set_can_recurse(src, TRUE);

	((CtlClientSource *)src)->client = self;
	priv->src = (CtlClientSource *)src;

	err = snd_ctl_event_malloc(&priv->event);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		alsactl_client_unlisten(self);
		return;
	}
	
	/* Attach the source to context. */
	g_source_attach(src, g_main_context_default());
	((CtlClientSource *)src)->tag =
			g_source_add_unix_fd(src, pfds.fd, G_IO_IN);

	/* Be sure to subscribe events. */
	err = snd_ctl_subscribe_events(self->handle, 1);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		alsactl_client_unlisten(self);
	}
}

void alsactl_client_unlisten(ALSACtlClient *self)
{
	ALSACtlClientPrivate *priv;

	g_return_if_fail(ALSACTL_IS_CLIENT(self));
	priv = CTL_CLIENT_GET_PRIVATE(self);

	if (priv->src != NULL) {
		g_source_destroy((GSource *)priv->src);
		g_free(priv->src);
		priv->src = NULL;
	}

	if (priv->event != NULL) {
		snd_ctl_event_free(priv->event);
		priv->event = NULL;
	}
}
