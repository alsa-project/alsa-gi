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
#include "alsatimer_sigs_marshal.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

/* For error handling. */
G_DEFINE_QUARK("ALSATimerClient", alsatimer_client)
#define raise(exception, errno)						\
	g_set_error(exception, alsatimer_client_quark(), errno,		\
		    "%d: %s", __LINE__, strerror(errno))

typedef struct {
	GSource src;
	ALSATimerClient *self;
	gpointer tag;
} TimerClientSource;

struct _ALSATimerClientPrivate {
	int fd;
	struct snd_timer_id id;
	struct snd_timer_info info;
	struct snd_timer_params params;

	TimerClientSource *src;
	void *buf;
	unsigned int len;
};

G_DEFINE_TYPE_WITH_PRIVATE(ALSATimerClient, alsatimer_client, G_TYPE_OBJECT)

enum timer_client_prop {
	/* snd_timer_info_t */
	TIMER_CLIENT_PROP_ID = 1,
	TIMER_CLIENT_PROP_NAME,
	TIMER_CLIENT_PROP_IS_SLAVE,
	TIMER_CLIENT_PROP_CARD,
	TIMER_CLIENT_PROP_RESOLUTION,
	/* snd_timer_params_t */
	TIMER_CLIENT_PROP_AUTO_START,
	TIMER_CLIENT_PROP_EXCLUSIVE,
	TIMER_CLIENT_PROP_EARLY_EVENT,
	TIMER_CLIENT_PROP_TICKS,
	TIMER_CLIENT_PROP_QUEUE_SIZE,
	TIMER_CLIENT_PROP_FILTER,
	TIMER_CLIENT_PROP_COUNT,
};
static GParamSpec *timer_client_props[TIMER_CLIENT_PROP_COUNT] = { NULL, };

enum timer_client_sig {
	TIMER_CLIENT_SIG_EVENT = 1,
	TIMER_CLIENT_SIG_COUNT,
};

static guint timer_client_sigs[TIMER_CLIENT_SIG_COUNT] = { 0 };

static void timer_client_get_property(GObject *obj, guint id,
				      GValue *val, GParamSpec *spec)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(obj);
	ALSATimerClientPrivate *priv =
				alsatimer_client_get_instance_private(self);

	switch (id) {
	case TIMER_CLIENT_PROP_ID:
		g_value_set_string(val, (char *)priv->info.id);
		break;
	case TIMER_CLIENT_PROP_NAME:
		g_value_set_string(val, (char *)priv->info.name);
		break;
	case TIMER_CLIENT_PROP_IS_SLAVE:
		g_value_set_boolean(val,
				priv->info.flags & SNDRV_TIMER_FLG_SLAVE);
		break;
	case TIMER_CLIENT_PROP_CARD:
		g_value_set_int(val, priv->info.card);
		break;
	case TIMER_CLIENT_PROP_RESOLUTION:
		g_value_set_long(val, priv->info.resolution);
		break;
	case TIMER_CLIENT_PROP_AUTO_START:
		g_value_set_boolean(val,
				priv->params.flags & SNDRV_TIMER_PSFLG_AUTO);
		break;
	case TIMER_CLIENT_PROP_EXCLUSIVE:
		g_value_set_boolean(val,
			priv->params.flags & SNDRV_TIMER_PSFLG_EXCLUSIVE);
		break;
	case TIMER_CLIENT_PROP_EARLY_EVENT:
		g_value_set_boolean(val,
			priv->params.flags & SNDRV_TIMER_PSFLG_EARLY_EVENT);
		break;
	case TIMER_CLIENT_PROP_TICKS:
		g_value_set_long(val, priv->params.ticks);
		break;
	case TIMER_CLIENT_PROP_QUEUE_SIZE:
		g_value_set_long(val, priv->params.queue_size);
		break;
	case TIMER_CLIENT_PROP_FILTER:
		g_value_set_uint(val, priv->params.filter);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, id, spec);
		break;
	}
}

static void timer_client_set_property(GObject *obj, guint id,
				      const GValue *val, GParamSpec *spec)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(obj);
	ALSATimerClientPrivate *priv =
				alsatimer_client_get_instance_private(self);

	switch(id) {
	case TIMER_CLIENT_PROP_AUTO_START:
		if (g_value_get_boolean(val))
			priv->params.flags |= SNDRV_TIMER_PSFLG_AUTO;
		else
			priv->params.flags &= ~SNDRV_TIMER_PSFLG_AUTO;
		break;
	case TIMER_CLIENT_PROP_EXCLUSIVE:
		if (g_value_get_boolean(val))
			priv->params.flags |= SNDRV_TIMER_PSFLG_EXCLUSIVE;
		else
			priv->params.flags &= ~SNDRV_TIMER_PSFLG_EXCLUSIVE;
		break;
	case TIMER_CLIENT_PROP_EARLY_EVENT:
		if (g_value_get_boolean(val))
			priv->params.flags |= SNDRV_TIMER_PSFLG_EARLY_EVENT;
		else
			priv->params.flags &= ~SNDRV_TIMER_PSFLG_EARLY_EVENT;
		break;
	case TIMER_CLIENT_PROP_TICKS:
		priv->params.ticks = g_value_get_long(val);
		break;
	case TIMER_CLIENT_PROP_QUEUE_SIZE:
		priv->params.queue_size = g_value_get_long(val);
		break;
	case TIMER_CLIENT_PROP_FILTER:
		priv->params.filter = g_value_get_uint(val);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, id, spec);
		break;
	}
}

static void timer_client_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsatimer_client_parent_class)->dispose(obj);
}

static void timer_client_finalize(GObject *obj)
{
	ALSATimerClient *self = ALSATIMER_CLIENT(obj);
	ALSATimerClientPrivate *priv =
				alsatimer_client_get_instance_private(self);

	close(priv->fd);

	G_OBJECT_CLASS(alsatimer_client_parent_class)->finalize(obj);
}

static void alsatimer_client_class_init(ALSATimerClientClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	gobject_class->get_property = timer_client_get_property;
	gobject_class->set_property = timer_client_set_property;
	gobject_class->dispose = timer_client_dispose;
	gobject_class->finalize = timer_client_finalize;

	timer_client_props[TIMER_CLIENT_PROP_ID] = 
		g_param_spec_string("id", "id",
				    "The id of timer for this client",
				    NULL,
				    G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_NAME] = 
		g_param_spec_string("name", "name",
				    "The name of timer for this client",
				    NULL,
				    G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_IS_SLAVE] = 
		g_param_spec_boolean("is-slave", "is-slave",
			"Whether the timer of this client is slave or not",
				     FALSE,
				     G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_CARD] = 
		g_param_spec_int("card", "card",
				 "The card number of timer for this client",
				 -1, INT_MAX,
				 0,
				 G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_RESOLUTION] = 
		g_param_spec_long("resolution", "resolution",
				  "The resolution of timer for this client",
				  -1, LONG_MAX,
				  0,
				 G_PARAM_READABLE);
	timer_client_props[TIMER_CLIENT_PROP_AUTO_START] = 
		g_param_spec_boolean("auto-start", "auto-start",
				     "Automatically restart or not",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_EXCLUSIVE] = 
		g_param_spec_boolean("exclusive", "exclusive",
				     "Exclusively keep the timer",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_EARLY_EVENT] = 
		g_param_spec_boolean("early-event", "early-event",
				     "Generate initial event or not",
				     FALSE,
				     G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_TICKS] = 
		g_param_spec_long("ticks", "ticks",
				  "The number of ticks for this client",
				  0, LONG_MAX,
				  1,
				  G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_QUEUE_SIZE] = 
		g_param_spec_long("queue-size", "queue-size",
				  "The size of event queue",
				  0, LONG_MAX,
				  128,
				  G_PARAM_READWRITE);
	timer_client_props[TIMER_CLIENT_PROP_FILTER] = 
		g_param_spec_uint("filter", "filter",
				  "The bit flag for event filter",
				  0, UINT_MAX,
				  0,
				  G_PARAM_READWRITE);

	timer_client_sigs[TIMER_CLIENT_SIG_EVENT] =
		g_signal_new("event",
		     G_OBJECT_CLASS_TYPE(klass),
		     G_SIGNAL_RUN_LAST,
		     0,
		     NULL, NULL,
		     alsatimer_sigs_marshal_VOID__STRING_LONG_LONG_UINT,
		     G_TYPE_NONE, 4,
		     G_TYPE_STRING, G_TYPE_LONG, G_TYPE_LONG, G_TYPE_UINT);

	g_object_class_install_properties(gobject_class,
					  TIMER_CLIENT_PROP_COUNT,
					  timer_client_props);
}

static void alsatimer_client_init(ALSATimerClient *self)
{
	return;
}

/**
 * alsatimer_client_open:
 * @self: A #ALSATimerClient
 * @path: A full path of a special file for ALSA timer character device
 * @exception: A #GError
 *
 * Open ALSA Timer character device.
 */
void alsatimer_client_open(ALSATimerClient *self, gchar *path,
			   GError **exception)
{
	ALSATimerClientPrivate *priv;
	int flag = 1;

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	priv->fd = open(path, O_RDONLY);
	if (priv->fd < 0) {
		raise(exception, errno);
		return;
	}

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_TREAD, &flag) < 0) {
		raise(exception, errno);
		close(priv->fd);
		return;
	}

	/* Select system timer as a default. */
	alsatimer_client_select_timer(self,
				      SNDRV_TIMER_CLASS_GLOBAL,
				      SNDRV_TIMER_SCLASS_NONE,
				      -1, SNDRV_TIMER_GLOBAL_SYSTEM, 0,
				      exception);
}

/*
	SNDRV_TIMER_CLASS_NONE = -1,
	SNDRV_TIMER_CLASS_SLAVE = 0,
	SNDRV_TIMER_CLASS_GLOBAL,
	SNDRV_TIMER_CLASS_CARD,
	SNDRV_TIMER_CLASS_PCM,
	SNDRV_TIMER_CLASS_LAST = SNDRV_TIMER_CLASS_PCM,
 */
/*
	SNDRV_TIMER_SCLASS_NONE = 0,
	SNDRV_TIMER_SCLASS_APPLICATION,
	SNDRV_TIMER_SCLASS_SEQUENCER,
	SNDRV_TIMER_SCLASS_OSS_SEQUENCER,
	SNDRV_TIMER_SCLASS_LAST = SNDRV_TIMER_SCLASS_OSS_SEQUENCER,
*/
/**
 * alsatimer_client_get_timer_list:
 * @self: A #ALSATimerClient
 * @list: (element-type guint)(array)(out caller-allocates):
 * @exception: A #GError
 */
void alsatimer_client_get_timer_list(ALSATimerClient *self, GArray *list,
				    GError **exception)
{
	ALSATimerClientPrivate *priv;
	struct snd_timer_id id = {0};
	struct snd_timer_ginfo info ={{0}};

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	id.dev_class = SNDRV_TIMER_CLASS_NONE;
	while (1) {
		if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_NEXT_DEVICE, &id) < 0) {
			raise(exception, errno);
			break;
		}

		if (id.dev_class == SNDRV_TIMER_CLASS_NONE)
			break;

		info.tid = id;
		if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_GINFO, &info) < 0) {
			raise(exception, errno);
			break;
		}
	}
}

/**
 * alsatimer_client_assign_timer:
 * @self: A #ALSATimerClient
 * @class: a numerical value for class
 * @subclass: a numerical value for subclass
 * @card: a numerical value for card
 * @device: a numerical value for device
 * @subdevice: a numerical value for subdevice
 * @exception: A #GError
 */
void alsatimer_client_select_timer(ALSATimerClient *self,
				   unsigned int class, unsigned int subclass,
				   unsigned int card,
				   unsigned int device, unsigned int subdevice,
				   GError **exception)
{
	ALSATimerClientPrivate *priv;
	struct snd_timer_select target = {{0}};

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	target.id.dev_class = class;
	target.id.dev_sclass = subclass;
	target.id.card = card;
	target.id.device = device;
	target.id.subdevice = subdevice;
	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_SELECT, &target) < 0) {
		raise(exception, errno);
		return;
	}

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_INFO, &priv->info) < 0) {
		raise(exception, errno);
		return;
	}

	priv->params.ticks = 1;
	priv->params.queue_size = 128;
	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_PARAMS, &priv->params) < 0) {
		raise(exception, errno);
		return;
	}

	/* Keep my id. */
	priv->id = target.id;
}

/**
 * alsatimer_client_get_status:
 * @self: A #ALSATimerClient
 * @status: (element-type uint) (array) (out caller-allocates):
 *		A #GArray for status
 * @exception: A #GError
 */
void alsatimer_client_get_status(ALSATimerClient *self, GArray *status,
				 GError **exception)
{
	ALSATimerClientPrivate *priv;
	struct snd_timer_status s = {{0}};

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_STATUS, &s) < 0) {
		raise(exception, errno);
		return;
	}

	/* NOTE: narrow-conversions are expected... */
	g_array_append_val(status, s.tstamp.tv_sec);
	g_array_append_val(status, s.tstamp.tv_nsec);
	g_array_append_val(status, s.lost);
	g_array_append_val(status, s.overrun);
	g_array_append_val(status, s.queue);
}

static gboolean prepare_src(GSource *gsrc, gint *timeout)
{
	/* Use blocking poll(2) to save CPU usage. */
	*timeout = -1;

	/* This source is not ready, let's poll(2) */
	return FALSE;
}

static const char *const ev_name[] = {
	[SNDRV_TIMER_EVENT_RESOLUTION]	= "resolution",
	[SNDRV_TIMER_EVENT_TICK]	= "tick",
	[SNDRV_TIMER_EVENT_START]	= "start",
	[SNDRV_TIMER_EVENT_STOP]	= "stop",
	[SNDRV_TIMER_EVENT_CONTINUE]	= "continue",
	[SNDRV_TIMER_EVENT_PAUSE]	= "pause",
	[SNDRV_TIMER_EVENT_EARLY]	= "early",
	[SNDRV_TIMER_EVENT_SUSPEND]	= "suspend",
	[SNDRV_TIMER_EVENT_RESUME]	= "resume",
	[SNDRV_TIMER_EVENT_MSTART]	= "master-start",
	[SNDRV_TIMER_EVENT_MSTOP]	= "master-stop",
	[SNDRV_TIMER_EVENT_MCONTINUE]	= "master-continue",
	[SNDRV_TIMER_EVENT_MPAUSE]	= "master-pause",
	[SNDRV_TIMER_EVENT_MSUSPEND]	= "master-suspend",
	[SNDRV_TIMER_EVENT_MRESUME]	= "master-resume",
};

static gboolean check_src(GSource *gsrc)
{
	TimerClientSource *src = (TimerClientSource *)gsrc;
	GIOCondition condition;

	ALSATimerClient *self = src->self;
	ALSATimerClientPrivate *priv =
				alsatimer_client_get_instance_private(self);

	struct snd_timer_tread *ev;
	ssize_t len;

	condition = g_source_query_unix_fd((GSource *)src, src->tag);
	if (!(condition & G_IO_IN))
		goto end;

	len = read(priv->fd, priv->buf, priv->len);
	if (len < 0)
		goto end;

	while (len > 0) {
		ev = (struct snd_timer_tread *)priv->buf;
		if (ev->event <= SNDRV_TIMER_EVENT_MRESUME) {
			g_signal_emit(self,
				      timer_client_sigs[TIMER_CLIENT_SIG_EVENT],
				      0, ev_name[ev->event], ev->tstamp.tv_sec,
				      ev->tstamp.tv_nsec, ev->val);
		}
		len -= sizeof(struct snd_timer_tread);
	}
end:
	return FALSE;
}

static gboolean dispatch_src(GSource *gsrc, GSourceFunc callback,
			     gpointer user_data)
{
	/* Just be sure to continue to process this source. */
	return TRUE;
}

static void finalize_src(GSource *gsrc)
{
	/* Do nothing paticular. */
	return;
}

static void listen_client(ALSATimerClient *self, GError **exception)
{
	static GSourceFuncs funcs = {
		.prepare	= prepare_src,
		.check		= check_src,
		.dispatch	= dispatch_src,
		.finalize	= finalize_src,
	};
	ALSATimerClientPrivate *priv =
				alsatimer_client_get_instance_private(self);
	GSource *src;

	/* Keep a memory so as to store 10 events. */
	priv->len = sizeof(struct snd_timer_tread) * 10;
	priv->buf = g_malloc0(priv->len);
	if (priv->buf == NULL) {
		raise(exception, ENOMEM);
		return;
	}

	/* Create a source. */
	src = g_source_new(&funcs, sizeof(TimerClientSource));
	if (src == NULL) {
		raise(exception, ENOMEM);
		g_free(priv->buf);
		priv->len = 0;
		return;
	}

	g_source_set_name(src, "ALSATimerClient");
	g_source_set_priority(src, G_PRIORITY_HIGH_IDLE);
	g_source_set_can_recurse(src, TRUE);
	((TimerClientSource *)src)->self = self;
	priv->src = (TimerClientSource *)src;

	/* Attach the source to context. */
	g_source_attach(src, g_main_context_default());
	((TimerClientSource *)src)->tag =
				g_source_add_unix_fd(src, priv->fd, G_IO_IN);
}

static void unlisten_client(ALSATimerClient *self)
{
	ALSATimerClientPrivate *priv;

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	g_source_destroy((GSource *)priv->src);
	g_free(priv->src);
	priv->src = NULL;
	g_free(priv->buf);
	priv->buf = NULL;
	priv->len = 0;
}

void alsatimer_client_start(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv;

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_PARAMS, &priv->params) < 0) {
		raise(exception, errno);
		return;
	}

	listen_client(self, exception);
	if (*exception != NULL)
		return;

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_START) < 0) {
		raise(exception, errno);
		unlisten_client(self);
	}
}

void alsatimer_client_stop(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv;

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_STOP) < 0) {
		raise(exception, errno);
	}

	unlisten_client(self);
}

void alsatimer_client_resume(ALSATimerClient *self, GError **exception)
{
	ALSATimerClientPrivate *priv;

	g_return_if_fail(ALSATIMER_IS_CLIENT(self));
	priv = alsatimer_client_get_instance_private(self);

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_PARAMS, &priv->params) < 0) {
		raise(exception, errno);
		return;
	}

	listen_client(self, exception);
	if (*exception != NULL)
		return;

	if (ioctl(priv->fd, SNDRV_TIMER_IOCTL_CONTINUE) < 0)
		raise(exception, errno);
}
