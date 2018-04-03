#!/usr/bin/env bash
# a single fail results in global fail
set -e

if [ -d "build/Release" ]; then
  ls -l build/Release
  build/Release/qminer-test
fi
if [ -d "build/Debug" ]; then
  ls -l build/Debug
  build/Debug/qminer-test
fi
