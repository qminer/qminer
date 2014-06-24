cd v8
third_party\python_26\python build/gyp_v8 -Dtarget_arch=x64
"c:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.com" /build "Release|x64" build\All.sln
"c:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.com" /build "Debug|x64" build\All.sln
cd ..

copy /y v8\build\Release\lib\v8_base.x64.lib
copy /y v8\build\Release\lib\v8_snapshot.lib

md debug
cd debug
copy /y ..\v8\build\Debug\lib\v8_base.x64.lib
copy /y ..\v8\build\Debug\lib\v8_snapshot.lib
cd ..
