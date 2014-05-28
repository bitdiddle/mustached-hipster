#!/bin/bash

# check
if [ -z "$1" ];
then
	echo "Usage: ./run.sh application"
	exit 0;
fi
# run
if [ "$(uname)" == "Darwin" ]; then
	$PIN_ROOT/pin -t obj-intel64/brtrace.dylib -- $1
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	$PIN_ROOT/pin -t obj-intel64/brtrace.so -- $1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    echo "missing"
fi