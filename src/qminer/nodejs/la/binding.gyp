{
   'targets': [
      {
         'target_name': 'la',
         'sources': [
            '../utils.h', #not necessary for build, but useful for visual studio solution
            '../utils.cpp',
            'la_nodejs.h', #not necessary for build, but useful for visual studio solution
            'la_nodejs.cpp'
         ],
         'include_dirs': [
            '../',
            '../../../glib/',
            '../../../glib/base/',
            '../../../glib/mine/'
         ],
         'dependencies': [
            'glib'
         ],
         'cflags_cc!': [
            '-fno-rtti',
            '-fno-exceptions',
         ],
         'cflags_cc': [
            '-std=c++0x'
            '-frtti',
            '-fexceptions'
         ],
         'cflags': [
            '-g',
            '-fexceptions',
            '-frtti',
            '-Wall',
            '-Wno-deprecated-declarations',
            '-fopenmp',
         ],
         'xcode_settings': {
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [
               '-std=c++11', 
               '-stdlib=libc++' 
            ],
            'OTHER_LDFLAGS': [
               '-undefined dynamic_lookup'
            ]
         },         
         'dependencies': [
            'glib',
         ]
      },
      {
         'target_name': 'glib',
         'type': 'static_library',
         'include_dirs': [
            '.',
            '../../../glib/base/',
            '../../../glib/mine/',
            '../../../glib/misc/'
         ],
         'cflags_cc!': [
            '-fno-rtti',
            '-fno-exceptions',
         ],
         'cflags_cc': [
            '-frtti',
            '-fexceptions',
            '-std=c++0x'
         ],
         'cflags': [
            '-g',
            '-fexceptions',
            '-frtti',
            '-Wall',
            '-Wno-deprecated-declarations',
            '-fopenmp'
         ],
         'conditions': [
            ['OS == "linux"', {
               'libraries': [
                  '-lrt',
                  '-luuid'
               ]
            }],
            ['OS == "mac"', {
               'xcode_settings': {
                  'MACOSX_DEPLOYMENT_TARGET': '10.7',
                  'GCC_ENABLE_CPP_RTTI': 'YES',
                  'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                  'OTHER_CFLAGS': [
                     '-std=c++11', 
                     '-stdlib=libc++' 
                  ]
               }
            }]
         ],
         'sources': [
            '../../../glib/base/base.cpp',
            '../../../glib/mine/mine.cpp'
         ]
      }
   ]
}

