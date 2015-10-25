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
  echo "[INFO]: Please \"export NEUTCLASSLOC=/path/to/root/src/neutclass\". This NEUT instance must be built, the libraries must exist."
  unset SETUPDIR
  return 1
fi

if [ ! -e ${NEUTCLASSLOC}/neutvect.so ]; then
  echo "Could not find neutvect.so in \${NEUTCLASSLOC}/, Has NEUT been built?"
  return 2
fi 

if ! [[ ":$PATH:" == *":${SETUPDIR}/bin:"* ]]; then
  export PATH=${SETUPDIR}/neut2rootracker/bin:$PATH
fi
if ! [[ ":$LD_LIBRARY_PATH:" == *":${SETUPDIR}/lib:"* ]]; then
  export LD_LIBRARY_PATH=${SETUPDIR}/neut2rootracker/lib:$LD_LIBRARY_PATH
fi

unset SETUPDIR
