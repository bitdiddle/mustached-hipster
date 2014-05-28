#!/bin/bash

# make ensure you set $PIN_ROOT
if [ -z "$PIN_ROOT" ];
then
	echo "please set \$PIN_ROOT"
	exit 0;
fi

# note -s CACHESIZE might need to be larger
if [ -z "$1" ];
then
	parameter=""
else
	parameter="-s $1"
fi

if [ "$(uname -m)" == 'x86_64' ]; then
	folder=obj-intel64
else
	folder=obj-ia32
fi
# run
if [ "$(uname)" == "Darwin" ]; then
	echo $PIN_ROOT/pin -t $folder/pincache.dylib $parameter -- tar zcf data1.tgz data1
	$PIN_ROOT/pin -t $folder/pincache.dylib $parameter -- tar zcf data1.tgz data1
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
	echo $PIN_ROOT/pin -t $folder/pincache.dylib $parameter -- tar zcf data1.tgz data1
	$PIN_ROOT/pin -t $folder/pincache.so $parameter -- tar zcf data1.tgz data1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
	echo $PIN_ROOT/pin -t $folder/pincache.dylib $parameter -- tar zcf data1.tgz data1
    $PIN_ROOT/pin -t $folder/pincache.dll $parameter -- tar zcf data1.tgz data1
fi