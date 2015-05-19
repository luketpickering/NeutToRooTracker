#!/bin/sh

#if it was sourced as . setup.sh then you can't scrub off the end... assume that
#we are in the correct directory.
if ! echo "${BASH_SOURCE}" | grep "/"; then
  SETUPDIR=$PWD
else
  SETUPDIR=${BASH_SOURCE%/*}
fi

#Looks a bit silly but allows you to either export it manually for the build
#or specify it in here during deployment
if [ ! "${NEUTCLASSLOC}" ]; then
  NEUTCLASSLOC=""
fi

if [ ! "${NEUTCLASSLOC}" ]; then
  echo "[WARN]: NEUTCLASSLOC has not been set. You will not be able to build \
neut2rootracker."
fi

echo "Adding directory \"${SETUPDIR}\" (Actually: $(readlink -f ${SETUPDIR})) \
to PATH, LD_LIBRARY_PATH. If this doesn't look right then try \"source ./setup.\
sh\""

export LD_LIBRARY_PATH=${SETUPDIR}:$LD_LIBRARY_PATH
export PATH=${SETUPDIR}:$PATH
