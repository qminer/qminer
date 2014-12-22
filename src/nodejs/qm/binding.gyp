{
   'targets': [
      {
         'target_name': 'qm',
         'sources': [
            '../utils.h', 	#not necessary for build, but useful for visual studio solution
            '../utils.cpp',
            'qm_nodejs.h', 	#not necessary for build, but useful for visual studio solution
            'qm_nodejs.cpp',
            'qm_param.h',
            '../la/la_nodejs.h', #not necessary for build, but useful for visual studio solution
            '../la/la_nodejs.cpp'
        ],
	     'defines': [
          'MODULE_INCLUDE_LA'
        ],
         'include_dirs': [
            '../',
            '../../'
            '../../../glib/',
            '../../../glib/base/',
            '../../../glib/mine/',
            '../../../glib/misc/'
         ],
         'dependencies': [
            'glib',
            'qminer'
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
            }],
            ['OS == "mac"', {
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
               }
            }]
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
      },
      {
         'target_name': 'qminer',
         'type': 'static_library',
         'include_dirs': [
            '.',
            '../../',
            '../../../glib/base/',
            '../../../glib/mine/',
            '../../../glib/misc/'
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
            '../../qminer_core.cpp',
            '../../qminer_gs.cpp',
            '../../qminer_ftr.cpp',
            '../../qminer_aggr.cpp',
            '../../qminer_snap.cpp',
            '../../qminer_op.cpp'
         ]
      }
   ]
}

