#!/bin/bash

# NOTE:
# Current working directory should be in root of this repository.

export LD_LIBRARY_PATH=src/timer/.libs/:src/seq/.libs/:src/control/.libs/:/usr/lib:/lib
export GI_TYPELIB_PATH=src/timer/:src/seq/:src/control/:/usr/lib/girepository-1.0

if [[ $1 == 'timer' ]] ; then
	./sample/timer.py
elif [[ $1 == 'seq' ]] ; then
	./sample/sequencer.py
elif [[ $1 == 'ctl' ]] ; then
	./sample/control.py
fi
