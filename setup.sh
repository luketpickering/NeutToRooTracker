#!/bin/sh

#if it was sourced as . setup.sh then you can't scrub off the end... assume that
#we are in the correct directory.
if ! echo "${BASH_SOURCE}" | grep "/" --silent; then
  SETUPDIR=$(readlink -f $PWD)
else
  SETUPDIR=$(readlink -f ${BASH_SOURCE%/*})
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

if ! [[ ":$PATH:" == *":${SETUPDIR}/bin:"* ]]; then
  export PATH=${SETUPDIR}/bin:$PATH
fi
if ! [[ ":$LD_LIBRARY_PATH:" == *":${SETUPDIR}/lib:"* ]]; then
  export LD_LIBRARY_PATH=${SETUPDIR}/lib:$LD_LIBRARY_PATH
fi
