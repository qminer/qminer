@echo off
set REL_PATH=..\..\bin
set QMINER_HOME=
rem // save current directory and change to target directory
pushd %REL_PATH%
rem // save value of CD variable (current directory)
set QMINER_HOME=%CD%
rem // restore original directory
popd

mocha *.js