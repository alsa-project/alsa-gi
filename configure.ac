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


#
# GLIB 2.0 is required.
#
PKG_CHECK_MODULES(GLIB20, [ glib-2.0 >= 2.32.4 ], HAVE_GLIB20="yes", HAVE_GLIB20="no")
AS_IF([ test "x$HAVE_GLIB20" = "xno" ],
      [AC_MSG_ERROR([ *** Glib 2.0 support not found])])
AC_SUBST(GLIB20_CFLAGS)
AC_SUBST(GLIB20_LIBS)


#
# GLIB's gobject 2.0 is required.
#
PKG_CHECK_MODULES(GOBJECT, [ gobject-2.0 ], HAVE_GOBJECT20=yes, HAVE_GOBJECT20=yes)
AS_IF([ test "x$HAVE_GOBJECT20" = "xno" ],
      [AC_MSG_ERROR([ *** GObject 2.0 support not found])])
AC_SUBST(GOBJECT20_CFLAGS)
AC_SUBST(GOBJECT20_LIBS)
AM_PATH_GLIB_2_0([2.32.4], [], [], [gobject])


#
# Gobject Introspection support is required.
#
GOBJECT_INTROSPECTION_REQUIRE([1.32.1])


#
# GTK Documentation is required.
#
GTK_DOC_CHECK([1.18-2])


# ALSA library is required.
PKG_CHECK_MODULES(ASOUNDLIB, [ alsa >= 1.0.27 ], HAVE_ASOUNDLIB=yes, HAVE_ASOUNDLIB=no)
AS_IF([ test "x$HAVE_ASOUNDLIB" = "xno" ],
      [AC_MSG_ERROR([ *** ALSA library support not found])])
AC_SUBST(ASOUNDLIB_CFLAGS)
AC_SUBST(ASOUNDLIB_LIBS)


#
# ALSA Sequencer interface support
#
AC_ARG_ENABLE([sequencer],
	      AS_HELP_STRING([--disable-sequencer],
			     [Disable ALSA sequencer interface support]),
			     [case "${enableval}" in
				yes) sequencer=true  ;;
				no)  sequencer=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-sequencer) ;;
			     esac],
			     [sequencer=true])
AM_CONDITIONAL(ENABLE_SEQUENCER, test "x$sequencer" = "xtrue")


#
# ALSA timer interface support
#
AC_ARG_ENABLE([timer],
	      AS_HELP_STRING([--disable-timer],
			     [Disable ALSA timer interface support]),
			     [case "${enableval}" in
				yes) timer=true  ;;
				no)  timer=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-timer) ;;
			     esac],
			     [timer=true])
AM_CONDITIONAL(ENABLE_TIMER, test "x$timer" = "xtrue")


#
# ALSA rawmidi interface support
#
AC_ARG_ENABLE([rawmidi],
	      AS_HELP_STRING([--disable-rawmidi],
			     [Disable ALSA rawmidi interface support]),
			     [case "${enableval}" in
				yes) rawmidi=true  ;;
				no)  rawmidi=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-rawmidi) ;;
			     esac],
			     [rawmidi=false])
AM_CONDITIONAL(ENABLE_RAWMIDI, test "x$rawmidi" = "xtrue")


#
# ALSA hwdep interface support
#
AC_ARG_ENABLE([hwdep],
	      AS_HELP_STRING([--disable-hwdep],
			     [Disable ALSA hwdep interface support]),
			     [case "${enableval}" in
				yes) hwdep=true  ;;
				no)  hwdep=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-hwdep) ;;
			     esac],
			     [hwdep=false])
AM_CONDITIONAL(ENABLE_HWDEP, test "x$hwdep" = "xtrue")


#
# ALSA control interface support
#
AC_ARG_ENABLE([control],
	      AS_HELP_STRING([--disable-control],
			     [Disable ALSA control interface support]),
			     [case "${enableval}" in
				yes) control=true  ;;
				no)  control=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-control) ;;
			     esac],
			     [control=false])
AM_CONDITIONAL(ENABLE_CONTROL, test "x$control" = "xtrue")


#
# ALSA PCM interface support
#
AC_ARG_ENABLE([pcm],
	      AS_HELP_STRING([--disable-pcm],
			     [Disable ALSA PCM interface support]),
			     [case "${enableval}" in
				yes) pcm=true  ;;
				no)  pcm=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-pcm) ;;
			     esac],
			     [pcm=false])
AM_CONDITIONAL(ENABLE_PCM, test "x$pcm" = "xtrue")


#
# ALSA compress offload interface support
#
AC_ARG_ENABLE([compress],
	      AS_HELP_STRING([--disable-compress],
			     [Disable ALSA compress offload interface support]),
			     [case "${enableval}" in
				yes) compress=true  ;;
				no)  compress=false ;;
				*)   AC_MSG_ERROR(invalid value ${enableval} for --disable-compress) ;;
			     esac],
			     [compress=false])
AM_CONDITIONAL(ENABLE_COMPRESS, test "x$compress" = "xtrue")


AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/card/Makefile
  src/seq/Makefile
  src/timer/Makefile
  doc/Makefile
  doc/reference/Makefile
  doc/reference/version.xml
])

AC_OUTPUT

# ==========================================================================
echo "
 ---{ $PACKAGE_NAME $VERSION }---

	prefix:						${prefix}
	sysconfdir:					${sysconfdir}
	localstatedir:					${localstatedir}
	C Compiler:					${CC}
	CFLAGS:						${CFLAGS}

	GLIB-2.0:					${HAVE_GLIB20}
	GOBJECT-2.0:					${HAVE_GOBJECT20}
	GObject Introspection:				${found_introspection}

	ALSA library support:				${HAVE_ASOUNDLIB}
		card interface support:			true
		sequencer interface support:		${sequencer}
		rawmidi interface support:		${rawmidi}
		hwdep interface support:		${hwdep}
		control interface support:		${control}
		PCM interface support:			${pcm}
		compress offload interface support:	${compress}
"
