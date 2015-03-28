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
#include "elem_iec60958.h"

G_DEFINE_TYPE(ALSACtlElemIec60958, alsactl_elem_iec60958, ALSACTL_TYPE_ELEM)

static void ctl_elem_iec60958_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elem_iec60958_parent_class)->dispose(obj);
}

static void ctl_elem_iec60958_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elem_iec60958_parent_class)->finalize(gobject);
}

static void alsactl_elem_iec60958_class_init(
					ALSACtlElemIec60958Class *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elem_iec60958_dispose;
	gobject_class->finalize = ctl_elem_iec60958_finalize;
}

static void alsactl_elem_iec60958_init(ALSACtlElemIec60958 *self)
{
	return;
}