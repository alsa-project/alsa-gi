DOC_MODULE = alsaseq

DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.sgml

DOC_SOURCE_DIR =				\
	$(top_srcdir)/src/

SCAN_OPTIONS =					\
	--deprecated-guards="SAMPLE_DISABLE_DEPRECATED"

MKDB_OPTIONS =					\
	--name-space=alsaseq

HFILE_GLOB =					\
	$(top_srcdir)/src/seq/alsaseq*.h

CFILE_GLOB =					\
	$(top_srcdir)/src/seq/alsaseq*.c

AM_CPPFLAGS =					\
	-I$(top_builddir)			\
	-I$(top_srcdir)

AM_CFLAGS =					\
	$(GLIB_CFLAGS)

GTKDOC_LIBS =					\
	$(top_builddir)/src/seq/alsaseq.la	\
	$(GLIB_LIBS)

include $(srcdir)/gtk-doc.make

CLEANFILES +=					\
	$(DOC_MODULE)-sections.txt		\
	$(DOC_MODULE).types
