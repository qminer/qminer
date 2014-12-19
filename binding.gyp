{
    'targets': [
        {
            # node qminer module
            'target_name': 'qm',
            'sources': [
                'src/nodejs/qm/qm_nodejs.h',
                'src/nodejs/qm/qm_nodejs.cpp',
                'src/nodejs/qm/qm_param.h',
                'src/nodejs/la/la_nodejs.h',
                'src/nodejs/la/la_nodejs.cpp',
                'src/nodejs/utils.h',
                'src/nodejs/utils.cpp'
            ],
            'include_dirs': [
                'src/nodejs/qm',
                'src/nodejs/la',
                'src/nodejs/',
                'src/qminer/',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/'
            ],
            'defines': [
                'MODULE_INCLUDE_LA'
            ],
            'dependencies': [
                'glib',
                'qminer'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid' ]}],
                ['OS == "mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    }
                }]
            ]
        }, {
            # node linear algebra module
            'target_name': 'la',
            'sources': [
                'src/nodejs/la/la_nodejs.h',
                'src/nodejs/la/la_nodejs.cpp',
                'src/nodejs/utils.h',
                'src/nodejs/utils.cpp'
            ],
            'include_dirs': [
                'src/nodejs/la',
                'src/nodejs/',
                'src/glib/base/',
                'src/glib/mine/',
            ],
            'dependencies': [
                'glib'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid' ]}],
                ['OS == "mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    }
                }]
            ]
        }, {
            # qminer library
            'target_name': 'qminer',
            'type': 'static_library',
            'sources': [
                'src/qminer/qminer_core.cpp',
                'src/qminer/qminer_gs.cpp',
                'src/qminer/qminer_ftr.cpp',
                'src/qminer/qminer_aggr.cpp',
                'src/qminer/qminer_snap.cpp',
                'src/qminer/qminer_op.cpp'
            ],        
            'include_dirs': [
                'src/qminer',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid' ]}],
                ['OS == "mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    }
                }]
            ]
        }, {
            # glib library
            'target_name': 'glib',
            'type': 'static_library',
            'sources': [
                'src/glib/base/base.cpp',
                'src/glib/mine/mine.cpp'
            ],        
            'include_dirs': [
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid' ]}],
                ['OS == "mac"', {
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    }
                }]
            ]
        }
    ],
    'target_defaults': {
        # GCC flags
        'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
        'cflags_cc': [ '-std=c++0x', '-frtti', '-fexceptions' ],
        'cflags': [ '-g', '-fexceptions', '-frtti', '-Wall', '-Wno-deprecated-declarations', '-fopenmp' ]    },    
}
