"c:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.com" /build "Debug|x64" libuv\uv.sln
"c:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.com" /build "Release|x64" libuv\uv.sln

copy /y libuv\x64\Release\libuv.lib

md debug
cd debug

copy /y ..\libuv\x64\Debug\libuv.lib

cd ..
