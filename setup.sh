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
if [ ! "${NEUT_ROOT}" ]; then
  NEUT_ROOT=""
fi

if [ ! "${NEUT_ROOT}" ]; then
  echo "[WARN]: NEUT_ROOT has not been set. You will not be able to build \
neut2rootracker."
  echo "[INFO]: Please \"$ export NEUT_ROOT=/path/to/neut\". This NEUT instance must be built, the libraries must exist."
  unset SETUPDIR
  return 1
fi

if ! [[ ":$PATH:" == *":${SETUPDIR}/bin:"* ]]; then
  export PATH=${SETUPDIR}/neut2rootracker/bin:$PATH
fi

unset SETUPDIR
