{
   'targets': [
      {
         'target_name': 'stat',
         'sources': [
            '../utils.h', #not necessary for build, but useful for visual studio solution
            '../utils.cpp',
            'stat_nodejs.cpp',
			'stat_nodejs.h',
			'../la/la_nodejs.cpp',
			'../la/la_nodejs.h'
         ],
		 'defines': [
           'MODULE_INCLUDE_LA'
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
            }]
         ],
         'sources': [
            '../../../glib/base/base.cpp',
            '../../../glib/mine/mine.cpp'
         ]
      }
   ]
}

