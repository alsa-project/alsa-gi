#include <alsa/asoundlib.h>
#include "client.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

struct _ALSASeqClientPrivate {
	snd_seq_t *handle;
};

G_DEFINE_TYPE_WITH_PRIVATE (ALSASeqClient, alsaseq_client, G_TYPE_OBJECT)

enum alsaseq_client_prop {
	CLIENT_PROP_HANDLE = 1,
	CLIENT_PROP_COUNT,
};

static GParamSpec *client_props[CLIENT_PROP_COUNT] = { NULL, };

static void alseseq_client_get_property(GObject *obj, guint id,
					GValue *val, GParamSpec *spec)
{
	ALSASeqClient *self = ALSASEQ_CLIENT(obj);

	switch (id) {
	case CLIENT_PROP_HANDLE:
		g_value_set_pointer(val, self->priv->handle);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		break;
	}
}

static void alseseq_client_set_property(GObject *obj, guint id,
					const GValue *val, GParamSpec *spec)
{
	ALSASeqClient *self = ALSASEQ_CLIENT(obj);

	switch (id) {
	case CLIENT_PROP_HANDLE:
		if (self->priv->handle != NULL)
			snd_seq_close(self->priv->handle);
		self->priv->handle = g_value_get_pointer(val);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		break;
	}
}

static void alsaseq_client_dispose(GObject *gobject)
{
	G_OBJECT_CLASS (alsaseq_client_parent_class)->dispose(gobject);
}

static void alsaseq_client_finalize (GObject *gobject)
{
	ALSASeqClient *self = ALSASEQ_CLIENT(gobject);

	if (self->priv->handle != NULL)
		snd_seq_close(self->priv->handle);

	G_OBJECT_CLASS(alsaseq_client_parent_class)->finalize (gobject);
}

static void alsaseq_client_class_init(ALSASeqClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->get_property = alseseq_client_get_property;
	gobject_class->set_property = alseseq_client_set_property;
	gobject_class->dispose = alsaseq_client_dispose;
	gobject_class->finalize = alsaseq_client_finalize;

	client_props[CLIENT_PROP_HANDLE] =
		g_param_spec_pointer("handle", "handle",
				     "a pointer to snd_seq_t structure",
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

	g_object_class_install_properties(gobject_class, CLIENT_PROP_COUNT,
					  client_props);
}

static void
alsaseq_client_init(ALSASeqClient *self)
{
	self->priv = alsaseq_client_get_instance_private(self);
}

ALSASeqClient *alsaseq_client_new(gchar *node, GError **exception)
{
	ALSASeqClient *self;
	snd_seq_t *handle;
	int err;

	/* Always open duplex ports. */
	err = snd_seq_open(&handle, node, SND_SEQ_OPEN_DUPLEX, 0);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	self = g_object_new(ALSASEQ_TYPE_CLIENT, NULL);
	self->priv->handle = handle;

	return self;
}

const guchar *alsaseq_client_get_name(ALSASeqClient *self)
{
	return snd_seq_name(self->priv->handle);
}

gint alsaseq_client_get_id(ALSASeqClient *self)
{
	return snd_seq_client_id(self->priv->handle);
}

guint alsaseq_client_get_output_buffer_size(ALSASeqClient *self)
{
	return snd_seq_get_output_buffer_size(self->priv->handle);
}

gboolean alsaseq_client_set_output_buffer_size(ALSASeqClient *self, guint size,
					    GError **exception)
{
	int err;

	err = snd_seq_set_output_buffer_size(self->priv->handle, size);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	}

	return err == 0;
}

guint alsaseq_client_get_input_buffer_size(ALSASeqClient *self)
{
	return snd_seq_get_input_buffer_size(self->priv->handle);
}

/*
 * Aligned to sizeof(snd_client_event_t) automatically.
 */
gboolean alsaseq_client_set_input_buffer_size(ALSASeqClient *self, guint size,
					   GError **exception)
{
	int err;

	err = snd_seq_set_input_buffer_size(self->priv->handle, size);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
	}

	return err == 0;
}

/**
 * alsaseq_client_get_info:
 *
 * Return a new #ALSASeqClientInfo for the client.
 *
 * Returns: (transfer full): a new #ALSASeqClientInfo.
 */
ALSASeqClientInfo *alsaseq_client_get_info(ALSASeqClient *client,
					   GError **exception)
{
	ALSASeqClientInfo *info;
	snd_seq_t *client_handle;
	snd_seq_client_info_t *info_handle;
	int err;

	/* Check handle is already retrieved. */
	client_handle = client->priv->handle;
	if  (client_handle == NULL) {
		err = -EINVAL;
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* Generate info object. */
	info = g_object_new(ALSASEQ_TYPE_CLIENT_INFO, NULL);
	g_object_get(G_OBJECT(info), "handle", &info_handle, NULL);

	err = snd_seq_get_client_info(client_handle, info_handle);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		g_clear_object(&info);
		return NULL;
	}

	return info;
}

void alsaseq_client_set_info(ALSASeqClient *client, ALSASeqClientInfo *info,
			     GError **exception)
{
	snd_seq_t *client_handle;
	snd_seq_client_info_t *info_handle;
	int err;

	g_object_get(G_OBJECT(info), "handle", &info_handle, NULL);
	client_handle = client->priv->handle;

	err = snd_seq_set_client_info(client_handle, info_handle);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}

/**
 * alsaseq_client_get_system_info:
 *
 * Return a new #ALSASeqSystemInfo.
 *
 * Returns: (transfer full): a new #ALSASeqSystemInfo.
 */
ALSASeqSystemInfo *alsaseq_client_get_system_info(ALSASeqClient *client,
						  GError **exception)
{
	ALSASeqSystemInfo *info;
	snd_seq_t *client_handle;
	snd_seq_system_info_t *info_handle;
	int err;

	/* Check handle is already retrieved. */
	client_handle = client->priv->handle;
	if  (client_handle == NULL) {
		err = -EINVAL;
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* Generate info object. */
	info = g_object_new(ALSASEQ_TYPE_SYSTEM_INFO, NULL);
	g_object_get(G_OBJECT(info), "handle", &info_handle, NULL);

	err = snd_seq_system_info(client_handle, info_handle);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		g_clear_object(&info);
		return NULL;
	}

	return info;
}

/**
 * alsaseq_client_create_port:
 *
 * Return a new #ALSASeqPortInfo.
 *
 * Returns: (transfer full): a new #ALSASeqPortInfo.
 */
ALSASeqPortInfo *alsaseq_client_create_port(ALSASeqClient *client,
					    GError **exception)
{
	ALSASeqPortInfo *port_info;
	snd_seq_t *client_handle;
	snd_seq_port_info_t *port_info_handle;
	unsigned int id;
	int err;

	/* Check handle is already retrieved. */
	client_handle = client->priv->handle;
	if  (client_handle == NULL) {
		err = -EINVAL;
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* Generate info object. */
	port_info = g_object_new(ALSASEQ_TYPE_PORT_INFO, NULL);
	g_object_get(G_OBJECT(port_info), "handle", &port_info_handle, NULL);

	/* Create new port. */
	err = snd_seq_create_port(client_handle, port_info_handle);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		g_clear_object(&port_info);
		return NULL;
	}

	return port_info;
}

void alsaseq_client_delete_port(ALSASeqClient *client, guint id,
				GError **exception)
{
	snd_seq_t *client_handle;
	GValue val = G_VALUE_INIT;
	int err;

	/* Check handle is already retrieved. */
	client_handle = client->priv->handle;
	if  (client_handle == NULL) {
		err = -EINVAL;
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return;
	}

	err = snd_seq_delete_port(client_handle, id);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}

/**
 * alsaseq_client_get_port_info:
 *
 * Return a new #ALSASeqPortInfo.
 *
 * Returns: (transfer full): a new #ALSASeqPortInfo.
 */
ALSASeqPortInfo *alsaseq_client_get_port_info(ALSASeqClient *client, guint id,
					      GError **exception)
{
	ALSASeqPortInfo *port_info;
	snd_seq_t *client_handle;
	snd_seq_port_info_t *port_info_handle;
	int err;

	/* Check handle is already retrieved. */
	client_handle = client->priv->handle;
	if  (client_handle == NULL) {
		err = -EINVAL;
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		return NULL;
	}

	/* Generate info object. */
	port_info = g_object_new(ALSASEQ_TYPE_PORT_INFO, NULL);
	g_object_get(G_OBJECT(port_info), "handle", port_info_handle, NULL);

	err = snd_seq_get_port_info(client_handle, id, port_info_handle);
	if (err < 0) {
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
		g_clear_object(&port_info);
		return NULL;
	}

	return port_info;
}

void alsaseq_client_set_port_info(ALSASeqClient *client, guint id,
				  ALSASeqPortInfo *port_info,
				  GError **exception)
{
	snd_seq_t *client_handle;
	snd_seq_port_info_t *port_info_handle;
	int err;

	g_object_get(G_OBJECT(port_info), "handle", &port_info_handle, NULL);
	client_handle = client->priv->handle;

	err = snd_seq_set_port_info(client_handle, id, port_info_handle);
	if (err < 0)
		g_set_error(exception, g_quark_from_static_string(__func__),
			    -err, "%s", snd_strerror(err));
}
