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

/* For error handling. */
G_DEFINE_QUARK("ALSACtlElemIec60958", alsactl_elem_iec60958)
#define raise(exception, errno)						\
	g_set_error(exception, alsactl_elem_iec60958_quark(), errno,	\
		    "%d: %s", __LINE__, strerror(errno))

G_DEFINE_TYPE(ALSACtlElemIec60958, alsactl_elem_iec60958, ALSACTL_TYPE_ELEM)

static void alsactl_elem_iec60958_class_init(
					ALSACtlElemIec60958Class *klass)
{
	return;
}

static void alsactl_elem_iec60958_init(ALSACtlElemIec60958 *self)
{
	return;
}
