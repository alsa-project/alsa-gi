#!/bin/bash

# NOTE:
# Current working directory should be in root of this repository.

export LD_LIBRARY_PATH="build/src:${LD_LIBRARY_PATH}"
export GI_TYPELIB_PATH="build/src:${GI_TYPELIB_PATH}"

if [[ $1 == 'timer' ]] ; then
	./sample/timer.py
elif [[ $1 == 'seq' ]] ; then
	./sample/seq.py
elif [[ $1 == 'ctl' ]] ; then
	./sample/ctl.py
fi
