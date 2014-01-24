svn co http://v8.googlecode.com/svn/branches/3.18/ v8
svn co http://gyp.googlecode.com/svn/trunk v8/build/gyp --revision 1685
svn co http://src.chromium.org/svn/trunk/tools/third_party/python_26@89111 v8/third_party/python_26
svn co http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 v8/third_party/cygwin

cd v8
third_party\python_26\python build/gyp_v8 -Dtarget_arch=x64
"c:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.com" /build "Release|x64" build\All.sln
"c:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.com" /build "Debug|x64" build\All.sln
cd ..

copy /y v8\build\Release\lib\v8_base.x64.lib
copy /y v8\build\Release\lib\v8_snapshot.lib

md debug
cd debug
copy /y ..\v8\build\Debug\lib\v8_base.x64.lib
copy /y ..\v8\build\Debug\lib\v8_snapshot.lib
cd ..
