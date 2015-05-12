#!/bin/sh
SETUPDIR=${BASH_SOURCE%/*}

echo $(readlink -f ${SETUPDIR})

export LD_LIBRARY_PATH=${SETUPDIR}:$LD_LIBRARY_PATH
export PATH=${SETUPDIR}:$PATH
