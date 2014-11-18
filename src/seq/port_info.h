#ifndef __ALSASEQ_PORT_INFO_H__
#define __ALSASEQ_PORT_INFO_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define ALSASEQ_TYPE_PORT_INFO	(alsaseq_port_info_get_type())

#define ALSASEQ_PORT_INFO(obj)						\
	(G_TYPE_CHECK_INSTANCE_CAST((obj),				\
				    ALSASEQ_TYPE_PORT_INFO,		\
				    ALSASeqPortInfo))
#define ALSASEQ_IS_PORT_INFO(obj)					\
	(G_TYPE_CHECK_INSTANCE_TYPE((obj),				\
				    ALSASEQ_TYPE_PORT_INFO))

#define ALSASEQ_PORT_INFO_CLASS(klass)					\
	(G_TYPE_CHECK_CLASS_CAST((klass),				\
				 ALSASEQ_TYPE_PORT_INFO,		\
				 ALSASeqClientClass))
#define ALSASEQ_IS_PORT_INFO_CLASS(klass)				\
	(G_TYPE_CHECK_CLASS_TYPE((klass),				\
				 ALSASEQ_TYPE_Seq))
#define ALSASEQ_PORT_INFO_GET_CLASS(obj) 				\
	(G_TYPE_INSTANCE_GET_CLASS((obj),				\
				   ALSASEQ_TYPE_PORT_INFO,		\
				   ALSASeqClientClass))

typedef struct _ALSASeqPortInfo		ALSASeqPortInfo;
typedef struct _ALSASeqPortInfoClass	ALSASeqPortInfoClass;
typedef struct _ALSASeqPortInfoPrivate	ALSASeqPortInfoPrivate;

struct _ALSASeqPortInfo
{
	GObject parent_instance;

	ALSASeqPortInfoPrivate *priv;
};

struct _ALSASeqPortInfoClass
{
    GObjectClass parent_class;
};

GType alsaseq_port_info_get_type(void) G_GNUC_CONST;

#endif
