#ifndef __ALSASEQ_CLIENT_INFO_H__
#define __ALSASEQ_CLIENT_INFO_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSASEQ_TYPE_CLIENT_INFO	(alsaseq_client_info_get_type())

#define ALSASEQ_CLIENT_INFO(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSASEQ_TYPE_CLIENT_INFO,	\
				    ALSASeqClientInfo))
#define ALSASEQ_IS_CLIENT_INFO(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSASEQ_TYPE_CLIENT_INFO))

#define ALSASEQ_CLIENT_INFO_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSASEQ_TYPE_CLIENT_INFO,	\
				 ALSASeqClientClass))
#define ALSASEQ_IS_CLIENT_INFO_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSASEQ_TYPE_Seq))
#define ALSASEQ_CLIENT_INFO_GET_CLASS(obj) 			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSASEQ_TYPE_CLIENT_INFO,	\
				   ALSASeqClientClass))

typedef struct _ALSASeqClientInfo		ALSASeqClientInfo;
typedef struct _ALSASeqClientInfoClass		ALSASeqClientInfoClass;
typedef struct _ALSASeqClientInfoPrivate	ALSASeqClientInfoPrivate;

struct _ALSASeqClientInfo
{
	GObject parent_instance;

	ALSASeqClientInfoPrivate *priv;
};

struct _ALSASeqClientInfoClass
{
    GObjectClass parent_class;
};

GType alsaseq_client_info_get_type(void) G_GNUC_CONST;

const gchar *alsaseq_client_info_get_name(const ALSASeqClientInfo *info);
gboolean
alsaseq_client_info_get_broadcast_filter(const ALSASeqClientInfo *info);
gboolean
alsaseq_client_info_get_error_bounce(const ALSASeqClientInfo *info);
const unsigned char *
alsaseq_client_info_get_event_filter(const ALSASeqClientInfo *info);
gint alsaseq_client_info_get_num_ports(const ALSASeqClientInfo *info);
gint alsaseq_client_info_get_event_lost(const ALSASeqClientInfo *info);

void alsaseq_client_info_set_name(const ALSASeqClientInfo *info,
				  const char *name);
void alsaseq_client_info_set_broadcast_filter(const ALSASeqClientInfo *info,
					      gboolean enable);
void alsaseq_client_info_set_error_bounce(const ALSASeqClientInfo *info,
					  gboolean enable);
void alsaseq_client_info_set_event_filter(const ALSASeqClientInfo *info,
					  unsigned char *filter);

void alsaseq_client_info_event_filter_clear(const ALSASeqClientInfo *info);
void alsaseq_client_info_event_filter_add(const ALSASeqClientInfo *info,
					  gint event_type);
void alsaseq_client_info_event_filter_del(const ALSASeqClientInfo *info,
					  gint event_type);
void alsaseq_client_info_event_filter_check(const ALSASeqClientInfo *info,
					    gint event_type);

#endif
