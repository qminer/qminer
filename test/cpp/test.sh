#!/usr/bin/env bash
# a single fail results in global fail
set -e

cd build/Release
qminer-test.exe
