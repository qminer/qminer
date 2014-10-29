{
   'targets': [
      {
         'target_name': 'la',
         'sources': [
            'la_nodejs.cpp'
         ],
         'include_dirs': [
            './qminer/src/glib/',
            './qminer/src/glib/base/',
            './qminer/src/glib/mine/'
         ],
         'dependencies': [
            'glib'
         ],
         'cflags_cc!': [
            '-fno-rtti',
            '-fno-exceptions',
         ],
         'cflags_cc': [
            '-std=c++0x',
 	           '-frtti',
            '-fexceptions'
         ],
         'cflags': [
            '-g',
            '-fexceptions',
            '-frtti',
            '-std=c++0x',
            '-Wall',
            '-Wno-deprecated-declarations',
            '-fopenmp',
         ],
         'dependencies': [
            'glib',
         ]
      },
      {
         'target_name': 'glib',
         'type': 'static_library',
         'include_dirs': [
            '.',
            './qminer/src/glib/base/',
            './qminer/src/glib/mine/',
            './qminer/src/glib/misc/'
         ],
         'cflags_cc!': [
            '-fno-rtti',
            '-fno-exceptions',
         ],
         'cflags_cc': [
            '-std=c++0x',
            '-frtti',
            '-fexceptions'
         ],
         'cflags': [
            '-g',
            '-fexceptions',
            '-frtti',
            '-std=c++0x',
            '-Wall',
            '-Wno-deprecated-declarations',
            '-fopenmp',
         ],
         'conditions': [
            ['OS == "linux"', {
               'libraries': [
                  '-lrt',
                  '-luuid'
               ]
            }]
         ],
         'sources': [
            './qminer/src/glib/base/base.cpp',
            './qminer/src/glib/mine/mine.cpp'
         ]
      }
   ]
}

