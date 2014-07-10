bash genJSdoc.sh
cd src/qminer/intellisense
python intellisense.py
python procintelli.py
cd ../../..
cp docjs/jsdocfinal.txt ../qminer.wiki/JavaScript.md
cd ..
cd qminer.wiki

@rem Do not use "echo off" to not affect any child calls.
@setlocal
git add JavaScript.md
git commit --message=AutoDoc

@rem Get the absolute path to the current directory, which is assumed to be the
@rem Git installation root.
@for /F "delims=" %%I in ("%~dp0") do @set git_install_root=%%~fI
@set PATH=%git_install_root%\bin;%git_install_root%\mingw\bin;%git_install_root%\cmd;%PATH%

@if not exist "%HOME%" @set HOME=%HOMEDRIVE%%HOMEPATH%
@if not exist "%HOME%" @set HOME=%USERPROFILE%

@set PLINK_PROTOCOL=ssh
@if not defined TERM set TERM=msys
git pull https://github.com/qminer/qminer.wiki.git
git push
@endlocal
cd ..\qminer