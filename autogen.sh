#!/bin/sh

set -u
set -e

dirs="card compress control hwdep pcm rawmidi seq timer"

if [ ! -e m4 ] ; then
	mkdir -p m4
fi

for i in $dirs; do
	dir="src/${i}"
	if [ ! -e $dir ] ; then
		mkdir $dir
	fi
done

gtkdocize --copy --docdir doc/reference
autoreconf --install
