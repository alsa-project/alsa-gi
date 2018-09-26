#ifndef __ALSA_GOBJECT_ALSASEQ_PORT_H__
#define __ALSA_GOBJECT_ALSASEQ_PORT_H__

#include <glib.h>
#include <glib-object.h>

#include "client.h"
#include "alsaseq_sigs_marshal.h"

G_BEGIN_DECLS

#define ALSASEQ_TYPE_PORT    (alsaseq_port_get_type())

#define ALSASEQ_PORT(obj)                           \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),              \
                                ALSASEQ_TYPE_PORT,  \
                                ALSASeqPort))
#define ALSASEQ_IS_PORT(obj)                        \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj),              \
                                ALSASEQ_TYPE_PORT))

#define ALSASEQ_PORT_CLASS(klass)                   \
    (G_TYPE_CHECK_CLASS_CAST((klass),               \
                             ALSASEQ_TYPE_PORT,     \
                             ALSASeqPortClass))
#define ALSASEQ_IS_PORT_CLASS(klass)                \
    (G_TYPE_CHECK_CLASS_TYPE((klass),               \
                             ALSASEQ_TYPE_PORT))
#define ALSASEQ_PORT_GET_CLASS(obj)                 \
    (G_TYPE_INSTANCE_GET_CLASS((obj),               \
                               ALSASEQ_TYPE_PORT,   \
                               ALSASeqPortClass))

typedef struct _ALSASeqPort         ALSASeqPort;
typedef struct _ALSASeqPortClass    ALSASeqPortClass;
typedef struct _ALSASeqPortPrivate  ALSASeqPortPrivate;

typedef struct _ALSASeqClient   ALSASeqClient;
struct _ALSASeqPort
{
    GObject parent_instance;

    ALSASeqClient *_client;

    ALSASeqPortPrivate *priv;
};

struct _ALSASeqPortClass
{
    GObjectClass parent_class;
};

GType alsaseq_port_get_type(void) G_GNUC_CONST;

typedef struct _ALSASeqClient    ALSASeqClient;

void alsaseq_port_update(ALSASeqPort *self, GError **exception);

#endif
