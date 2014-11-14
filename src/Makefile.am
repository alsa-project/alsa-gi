ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

if ENABLE_CARD
CARD_DIR=card
else
CARD_DIR=
endif

if ENABLE_SEQUENCER
SEQUENCER_DIR=seq
else
SEQUENCER_DIR=
endif

if ENABLE_RAWMIDI
RAWMIDI_DIR=rawmidi
else
RAWMIDI_DIR=
endif

if ENABLE_HWDEP
HWDEP_DIR=hwdep
else
HWDEP_DIR=
endif

if ENABLE_CONTROL
CONTROL_DIR=control
else
CONTROL_DIR=
endif

if ENABLE_PCM
PCM_DIR=pcm
else
PCM_DIR=
endif

if ENABLE_COMPRESS
COMPRESS_DIR=compress
else
COMPRESS_DIR=
endif

if ENABLE_TIMER
TIMER_DIR=timer
else
TIMER_DIR=
endif

SUBDIRS=			\
	$(CARD_DIR)		\
	$(SEQUENCER_DIR)	\
	$(RAWMIDI_DIR)		\
	$(HWDEP_DIR)		\
	$(CONTROL_DIR)		\
	$(PCM_DIR)		\
	$(COMPRESS_DIR)		\
	$(TIMER_DIR)

DIST_SUBDIRS=			\
	card			\
	seq			\
	rawmidi			\
	hwdep			\
	control			\
	pcm			\
	compress		\
	timer
