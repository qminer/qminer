#!/usr/bin/env bash
# a single fail results in global fail
set -e

ls -l build/Release
build/Release/qminer-test
