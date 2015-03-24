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
#include "elemset_bool.h"

G_DEFINE_TYPE(ALSACtlElemsetBool, alsactl_elemset_bool, ALSACTL_TYPE_ELEMSET)

static void ctl_elemset_bool_dispose(GObject *obj)
{
	G_OBJECT_CLASS(alsactl_elemset_bool_parent_class)->dispose(obj);
}

static void ctl_elemset_bool_finalize(GObject *gobject)
{
	G_OBJECT_CLASS(alsactl_elemset_bool_parent_class)->finalize(gobject);
}

static void alsactl_elemset_bool_class_init(ALSACtlElemsetBoolClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->dispose = ctl_elemset_bool_dispose;
	gobject_class->finalize = ctl_elemset_bool_finalize;
}

static void alsactl_elemset_bool_init(ALSACtlElemsetBool *self)
{
	return;
}
