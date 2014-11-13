AC_PREREQ(2.65)

AC_INIT([alsa-gir], [1.0.0], [o-takashi@sakamocchi.jp])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.13 foreign -Wno-portability])
AM_SILENT_RULES([yes])

PKG_PROG_PKG_CONFIG
AC_PROG_CC
LT_INIT

# Based on glib-2.0 and gobject introspection
AM_PATH_GLIB_2_0([2.32.4], [], [], [gobject])
GTK_DOC_CHECK([1.18-2])
GOBJECT_INTROSPECTION_REQUIRE([1.32.1])

# ALSA library is always required.
PKG_CHECK_MODULES(ASOUNDLIB, [ alsa >= 1.0.28 ])
AC_SUBST(ASOUNDLIB_CFLAGS)
AC_SUBST(ASOUNDLIB_LIBS)
AC_SUBST(HAVE_ALSA)

# ALSA card interface ssupport
AC_ARG_ENABLE([card],
	      AS_HELP_STRING([--disable-card],
			     [Disable ALSA card interface ssupport]))
AS_IF([test "x$enable_card" != "xno"],
      [AC_DEFINE([HAVE_CARD], 1,
		 ["Have ALSA card interface ssupport."])])
AM_CONDITIONAL(HAVE_CARD, test "x$enable_card" = "xyes")

# ALSA Sequencer interface ssupport
AC_ARG_ENABLE([sequencer],
	      AS_HELP_STRING([--disable-sequencer],
			     [Disable ALSA sequencer interface support]))
AS_IF([test "x$enable_sequencer" != "xno"],
      [AC_DEFINE([HAVE_SEQUENCER], 1,
		 ["Have ALSA sequencer interface support."])])
AM_CONDITIONAL(HAVE_SEQUENCER, test "x$enable_sequencer" = "xyes")


# ALSA rawmidi interface support
AC_ARG_ENABLE([rawmidi],
	      AS_HELP_STRING([--disable-rawmidi],
			     [Disable ALSA rawmidi interface support]))
AS_IF([test "x$enable_rawmidi" != "xno"],
      [AC_DEFINE([HAVE_RAWMIDI], 1,
		 ["Have ALSA rawmidi interface support."])])
AM_CONDITIONAL(HAVE_RAWMIDI, test "x$enable_rawmidi" = "xyes")

# ALSA hwdep interface ssupport
AC_ARG_ENABLE([hwdep],
	      AS_HELP_STRING([--disable-hwdep],
			     [Disable ALSA hwdep interface ssupport]))
AS_IF([test "x$enable_hwdep" != "xno"],
      [AC_DEFINE([HAVE_HWDEP], 1,
		 ["Have ALSA hwdep interface ssupport."])])
AM_CONDITIONAL(HAVE_HWDEP, test "x$enable_hwdep" = "xyes")

# ALSA control interface ssupport
AC_ARG_ENABLE([control],
	      AS_HELP_STRING([--disable-control],
			     [Disable ALSA control interface ssupport]))
AS_IF([test "x$enable_control" != "xno"],
      [AC_DEFINE([HAVE_CONTROL], 1,
		 ["Have ALSA control interface ssupport."])])
AM_CONDITIONAL(HAVE_CONTROL, test "x$enable_control" = "xyes")

# ALSA PCM interface ssupport
AC_ARG_ENABLE([pcm],
	      AS_HELP_STRING([--disable-pcm],
			     [Disable ALSA PCM interface ssupport]))
AS_IF([test "x$enable_pcm" != "xno"],
      [AC_DEFINE([HAVE_PCM], 1,
		 ["Have ALSA pcm interface ssupport."])])
AM_CONDITIONAL(HAVE_PCM, test "x$enable_pcm" = "xyes")

# ALSA compress interface ssupport
AC_ARG_ENABLE([compress],
	      AS_HELP_STRING([--disable-compress],
			     [Disable ALSA compress offload interface ssupport]))
AS_IF([test "x$enable_compress" != "xno"],
      [AC_DEFINE([HAVE_COMPRESS], 1,
		 ["Have ALSA compress offload interface ssupport."])])
AM_CONDITIONAL(HAVE_COMPRESS, test "x$enable_compress" = "xyes")

# ALSA timer support
AC_ARG_ENABLE([timer],
	      AS_HELP_STRING([--disable-timer],
			     [Disable ALSA timer interface ssupport]))
AS_IF([test "x$enable_timer" != "xno"],
      [AC_DEFINE([HAVE_TIMER], 1,
		 ["Have ALSA timer interface ssupport."])])
AM_CONDITIONAL(HAVE_TIMER, test "x$enable_timer" = "xyes")

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/seq/Makefile
  doc/Makefile
  doc/reference/Makefile
  doc/reference/version.xml
])

AC_OUTPUT
