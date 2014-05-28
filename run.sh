#!/bin/bash

# check
if [ -z "$1" ];
then
	echo "Usage: ./run.sh application"
	exit 0;
fi

if [ "$(uname -m)" == 'x86_64' ]; then
	folder=obj-intel64
else
	folder=obj-ia32
fi
# run
if [ "$(uname)" == "Darwin" ]; then
	$PIN_ROOT/pin -t $folder/brtrace.dylib -- $1
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	$PIN_ROOT/pin -t $folder/brtrace.so -- $1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    $PIN_ROOT/pin -t $folder/brtrace.dll -- $1
fi