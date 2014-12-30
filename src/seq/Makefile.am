CLEANFILES =

AM_CPPFLAGS =					\
	 -I$(top_builddir)			\
	 -I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)				\
	$(GIO_CFLAGS)

lib_LTLIBRARIES =				\
	libalsaseq.la

libalsaseq_la_LIBADD =				\
	-lasound				\
	$(GLIB_LIBS)				\
	$(GIO_LIBS)

libalsaseq_la_SOURCES =				\
	client.h				\
	client.c				\
	port.h					\
	port.c

pkginclude_HEADERS = 				\
	client.h				\
	port.h

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =

if HAVE_INTROSPECTION
ALSASeq-1.0.gir: libalsaseq.la
ALSASeq_1_0_gir_PACKAGES =
ALSASeq_1_0_gir_EXPORT_PACKAGES = alsaseq
ALSASeq_1_0_gir_INCLUDES = GObject-2.0
ALSASeq_1_0_gir_CFLAGS =
ALSASeq_1_0_gir_LIBS = libalsaseq.la
ALSASeq_1_0_gir_FILES = $(libalsaseq_la_SOURCES)
ALSASeq_1_0_gir_SCANNERFLAGS =			\
	--identifier-prefix=ALSASeq		\
	--symbol-prefix=alsaseq
INTROSPECTION_GIRS += ALSASeq-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES +=					\
	$(gir_DATA)				\
	$(typelib_DATA)
endif
