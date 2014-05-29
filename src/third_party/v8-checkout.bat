svn co http://v8.googlecode.com/svn/branches/3.18/ v8
svn co http://gyp.googlecode.com/svn/trunk v8/build/gyp --revision 1685
svn co http://src.chromium.org/svn/trunk/tools/third_party/python_26@89111 v8/third_party/python_26
svn co http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 v8/third_party/cygwin

copy /y global-handles.cc v8\src\global-handles.cc