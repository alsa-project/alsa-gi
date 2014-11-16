#ifndef __ALSASEQ_SYSTEM_INFO_H__
#define __ALSASEQ_SYSTEM_INFO_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSASEQ_TYPE_SYSTEM_INFO	(alsaseq_system_info_get_type())

#define ALSASEQ_SYSTEM_INFO(obj)				\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),			\
				    ALSASEQ_TYPE_SYSTEM_INFO,	\
				    ALSASeqSystemInfo))
#define ALSASEQ_IS_SYSTEM_INFO(obj)				\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),			\
				    ALSASEQ_TYPE_SYSTEM_INFO))

#define ALSASEQ_SYSTEM_INFO_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_CAST((klass),			\
				 ALSASEQ_TYPE_SYSTEM_INFO,	\
				 ALSASeqClientClass))
#define ALSASEQ_IS_SYSTEM_INFO_CLASS(klass)			\
	(G_TYPE_CHECK_CLASS_TYPE((klass),			\
				 ALSASEQ_TYPE_Seq))
#define ALSASEQ_SYSTEM_INFO_GET_CLASS(obj) 			\
	(G_TYPE_INSTANCE_GET_CLASS((obj),			\
				   ALSASEQ_TYPE_SYSTEM_INFO,	\
				   ALSASeqClientClass))

typedef struct _ALSASeqSystemInfo		ALSASeqSystemInfo;
typedef struct _ALSASeqSystemInfoClass		ALSASeqSystemInfoClass;
typedef struct _ALSASeqSystemInfoPrivate	ALSASeqSystemInfoPrivate;

struct _ALSASeqSystemInfo
{
	GObject parent_instance;

	ALSASeqSystemInfoPrivate *priv;
};

struct _ALSASeqSystemInfoClass
{
    GObjectClass parent_class;
};

GType alsaseq_system_info_get_type(void) G_GNUC_CONST;

gint alsaseq_system_info_get_max_queues(const ALSASeqSystemInfo *info);
gint alsaseq_system_info_get_max_clients(const ALSASeqSystemInfo *info);
gint alsaseq_system_info_get_max_ports(const ALSASeqSystemInfo *info);
gint alsaseq_system_info_get_max_channels(const ALSASeqSystemInfo *info);
gint alsaseq_system_info_get_cur_queues(const ALSASeqSystemInfo *info);
gint alsaseq_system_info_get_cur_clients(const ALSASeqSystemInfo *info);

#endif
