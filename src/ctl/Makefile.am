CLEANFILES =					\
	alsactl_sigs_marshal.*

AM_CPPFLAGS =					\
	 -I$(top_builddir)			\
	 -I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)				\
	$(GIO_CFLAGS)

lib_LTLIBRARIES =				\
	libalsactl.la

libalsactl_la_LIBADD =				\
	$(GLIB_LIBS)				\
	$(GIO_LIBS)

libalsactl_la_SOURCES =				\
	alsactl_sigs_marshal.h			\
	alsactl_sigs_marshal.c			\
	client.h				\
	client.c				\
	elem.h					\
	elem.c					\
	elem_int.h				\
	elem_int.c				\
	elem_bool.h				\
	elem_bool.c				\
	elem_enum.h				\
	elem_enum.c				\
	elem_byte.h				\
	elem_byte.c				\
	elem_iec60958.h				\
	elem_iec60958.c

pkginclude_HEADERS = 				\
	alsactl_sigs_marshal.h			\
	client.h				\
	elem.h					\
	elem_int.h				\
	elem_bool.h				\
	elem_enum.h				\
	elem_byte.h				\
	elem_iec60958.h

alsactl_sigs_marshal.list:
	$(AM_V_GEN)( find | grep \.c$$ | xargs cat | 			\
	sed -n -e 's/.*alsactl_sigs_marshal_\([A-Z]*__[A-Z_]*\).*/\1/p' |\
	sed -e 's/__/:/' -e 'y/_/,/' ) | sort -u > $@

alsactl_sigs_marshal.h: alsactl_sigs_marshal.list
	$(AM_V_GEN)( glib-genmarshal --header				\
		--prefix=alsactl_sigs_marshal $< ) > $@

alsactl_sigs_marshal.c: alsactl_sigs_marshal.list alsactl_sigs_marshal.h
	$(AM_V_GEN)( echo '#include "alsactl_sigs_marshal.h"';		\
		glib-genmarshal --body --prefix=alsactl_sigs_marshal $< ) > $@

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
ALSACtl-1.0.gir: libalsactl.la
ALSACtl_1_0_gir_PACKAGES =
ALSACtl_1_0_gir_EXPORT_PACKAGES = alsactl
ALSACtl_1_0_gir_INCLUDES = GObject-2.0
ALSACtl_1_0_gir_CFLAGS =
ALSACtl_1_0_gir_LIBS = libalsactl.la
ALSACtl_1_0_gir_FILES = $(libalsactl_la_SOURCES)
ALSACtl_1_0_gir_SCANNERFLAGS =			\
	--identifier-prefix=ALSACtl		\
	--symbol-prefix=alsactl
INTROSPECTION_GIRS += ALSACtl-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES +=					\
	$(gir_DATA)				\
	$(typelib_DATA)
endif
