CLEANFILES =					\
	alsatimer_sigs_marshal.*

AM_CPPFLAGS =					\
	 -I$(top_builddir)			\
	 -I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)				\
	$(GIO_CFLAGS)				\
	-Wall

lib_LTLIBRARIES =				\
	libalsatimer.la

libalsatimer_la_LIBADD =			\
	$(GLIB_LIBS)				\
	$(GIO_LIBS)

libalsatimer_la_SOURCES =			\
	alsatimer_sigs_marshal.h		\
	alsatimer_sigs_marshal.c		\
	client.h				\
	client.c

pkginclude_HEADERS = 				\
	alsatimer_sigs_marshal.h		\
	client.h

alsatimer_sigs_marshal.list:
	$(AM_V_GEN)( find | grep \.c$$ | xargs cat | 			\
	sed -n -e 's/.*alsatimer_sigs_marshal_\([A-Z]*__[A-Z_]*\).*/\1/p' |\
	sed -e 's/__/:/' -e 'y/_/,/' ) | sort -u > $@

alsatimer_sigs_marshal.h: alsatimer_sigs_marshal.list
	$(AM_V_GEN)( glib-genmarshal --header				\
		--prefix=alsatimer_sigs_marshal $< ) > $@

alsatimer_sigs_marshal.c: alsatimer_sigs_marshal.list alsatimer_sigs_marshal.h
	$(AM_V_GEN)( echo '#include "alsatimer_sigs_marshal.h"';	\
		glib-genmarshal --body --prefix=alsatimer_sigs_marshal $< ) > $@

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
ALSATimer-1.0.gir: libalsatimer.la
ALSATimer_1_0_gir_PACKAGES =
ALSATimer_1_0_gir_EXPORT_PACKAGES = alsatimer
ALSATimer_1_0_gir_INCLUDES = GObject-2.0
ALSATimer_1_0_gir_CFLAGS =
ALSATimer_1_0_gir_LIBS = libalsatimer.la
ALSATimer_1_0_gir_FILES = $(libalsatimer_la_SOURCES)
ALSATimer_1_0_gir_SCANNERFLAGS =		\
	--identifier-prefix=ALSATimer		\
	--symbol-prefix=alsatimer
INTROSPECTION_GIRS += ALSATimer-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES +=					\
	$(gir_DATA)				\
	$(typelib_DATA)
endif
