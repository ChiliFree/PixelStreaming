#!/bin/sh
sh setup.sh
pushd ../..
node cirrus.js platform=Linux
popd
