CLEANFILES =

AM_CPPFLAGS =					\
	 -I$(top_builddir)			\
	 -I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)				\
	$(GIO_CFLAGS)

lib_LTLIBRARIES =				\
	libalsacard.la

libalsacard_la_LIBADD =				\
	-lhinawa				\
	$(GLIB_LIBS)				\
	$(GIO_LIBS)

libalsacard_la_SOURCES =				\
	alsacard.h				\
	alsacard.c

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
ALSACard-1.0.gir: libalsacard.la
ALSACard_1_0_gir_PACKAGES =
ALSACard_1_0_gir_EXPORT_PACKAGES = alsacard
ALSACard_1_0_gir_INCLUDES = GObject-2.0
ALSACard_1_0_gir_CFLAGS =
ALSACard_1_0_gir_LIBS = libalsacard.la
ALSACard_1_0_gir_FILES = $(libalsacard_la_SOURCES)
ALSACard_1_0_gir_SCANNERFLAGS =			\
	--identifier-prefix=ALSACard		\
	--symbol-prefix=alsacard			\
	-lasound
INTROSPECTION_GIRS += ALSACard-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES +=					\
	$(gir_DATA)				\
	$(typelib_DATA)
endif
