#!/usr/bin/env bash
# a single fail results in global fail
set -e

cd build/Release
#pwd
#ls -l
qminer-test
