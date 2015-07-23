@echo off
set REL_PATH=..\..\build
set QMINER_HOME=
rem // save current directory and change to target directory
pushd %REL_PATH%
rem // save value of CD variable (current directory)
set QMINER_HOME=%CD%
rem // restore original directory
popd
echo Qminer path : %QMINER_HOME%
set PATH=%PATH%;%QMINER_HOME%


del /q lock
qm config -overwrite
qm create