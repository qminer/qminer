{
   'targets': [
      {
         'target_name': 'snap',
         'sources': [
			'../nodeutil.h', #not necessary for build, but useful for visual studio solution
			'../nodeutil.cpp',
            'snap_nodejs.cpp',
			'snap_nodejs.h', #not necessary for build, but useful for visual studio solution
			'../la/la_nodejs.h', #not necessary for build, but useful for visual studio solution
			'../la/la_nodejs.cpp'
         ],
		 'defines': [
			'MODULE_INCLUDE_LA'
		],
         'include_dirs': [
			'../../../glib/base/',
			'../../../glib/mine/',
			'../../../glib/misc/',
			'../../../third_party/snap/snap-core',
			'../../../third_party/snap/snap-adv',
			'../../../third_party/snap/snap-exp',
			'../../',
			'../'
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
			'glib'
         ]
      },
	  {
         'target_name': 'glib',
         'type': 'static_library',
         'include_dirs': [
			'../../../glib/base/',
			'../../../glib/mine/',
			'../../../glib/misc/',
			'../../../third_party/snap/snap-core',
			'../../../third_party/snap/snap-adv',
			'../../../third_party/snap/snap-exp'
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
            '../../../glib/base/base.cpp',
            '../../../glib/mine/mine.cpp',
			'../../../third_party/snap/snap-core/Snap.cpp'
         ]
      }
	]
}

