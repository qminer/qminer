{
    'variables': {
        'LIN_ALG_BLAS%': 'NBLAS',
        'LIN_ALG_LAPACKE%': 'NLAPACKE',
        'LIN_ALG_EIGEN%': 'NEIGEN',
        'LIN_ALG_INCLUDE%': 'src/glib/base/',
        'LIN_EIGEN_INCLUDE%': 'src/third_party/eigen/',
        #full path to lapack or openblas libraries
        'LIN_ALG_LIB%': '',
        #64 bit indexing for BLAS
        'INDEX_64%': 'NINDEX_64',
        'INTEL%': 'NINTEL'
    },
    'target_defaults': {
        'default_configuration': 'Release',
        'configurations': {
            'Debug': {
                'defines': [
                    'DEBUG',
                ],
            },
            'Release': {
                'defines': [
                    'NDEBUG'
                ],
            }
        },
        'defines': [
            '<(LIN_ALG_BLAS)',
            '<(LIN_ALG_LAPACKE)',
            '<(LIN_ALG_EIGEN)',
            '<(INDEX_64)',
            '<(INTEL)'
        ],
        # hack for setting xcode settings based on example from
        # http://src.chromium.org/svn/trunk/o3d/build/common.gypi
        'target_conditions': [
            ['OS=="mac"', {
                'xcode_settings': {
                    'MACOSX_DEPLOYMENT_TARGET': '10.7',
                    'GCC_ENABLE_CPP_RTTI': 'YES',
                    'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                    'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ]
                },
            }],
        ],
        'conditions': [
            # operating system specific parameters
            ['OS == "linux"', {
                "link_settings": {
                    "libraries": [ '-lrt', '-fopenmp', '<(LIN_ALG_LIB)' ],
                },
                # GCC flags
                'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
                'cflags_cc': [ '-std=c++0x', '-frtti', '-fexceptions' ],
                'cflags': [ '-Wno-deprecated-declarations', '-fopenmp' ],
                # additional flags for ARM
                'conditions': [
                    ['target_arch == "arm"', {
                        "link_settings": {
                            "ldflags": [ '-Wl,--allow-multiple-definition' ]
                        },
                        'cflags!': [ ], # add -g if low on memory and gcc fails in debug mode
                        'cflags': [ '-fsigned-char' ], # add -g if you need symbols in release mode
                        'defines': [ "ARM" ]
                    }]
                ]
            }],
            ['OS == "win"', {
                # 'msbuild_toolset': 'v120', # works with v120 (--msvs_version=2013) and v140 (--msvs_version=2015)
                'msvs_settings': {
                    'VCCLCompilerTool': {
                        #'RuntimeTypeInfo': 'true',      # /GR  : this should work but doesn't get picked up
                        #'ExceptionHandling': 1,         # /EHsc: this should work but doesn't get picked up
                        'OpenMP': 'true',
                        "AdditionalOptions": [ "/EHsc /GR" ] # release mode displays D9025 warnings, which is a known issue https://github.com/nodejs/node-gyp/issues/335
                    },
                    'VCLinkerTool': {
                        'SubSystem' : 1, # Console
                        'AdditionalOptions': ['<(LIN_ALG_LIB)']
                    },
                },
            }],
            ['OS == "mac"', {
                "default_configuration": "Release",
                "configurations": {
                    "Debug": {
                        "defines": [
                            "DEBUG",
                        ],
                        "xcode_settings": {
                            "GCC_OPTIMIZATION_LEVEL": "0",
                            "GCC_GENERATE_DEBUGGING_SYMBOLS": "YES"
                        }
                    },
                    "Release": {
                        "defines": [
                            "NDEBUG"
                        ],
                        "xcode_settings": {
                            "GCC_OPTIMIZATION_LEVEL": "3",
                            "GCC_GENERATE_DEBUGGING_SYMBOLS": "NO",
                            "DEAD_CODE_STRIPPING": "YES",
                            "GCC_INLINES_ARE_PRIVATE_EXTERN": "YES"
                        }
                    }
                }
            }]
        ],
    },
    'targets': [
        {
            # node qminer module
            'target_name': 'qm',
            'sources': [
                # core qm module
                'src/nodejs/qm/qm_nodejs.h',
                'src/nodejs/qm/qm_nodejs.cpp',
                'src/nodejs/qm/qm_nodejs_streamaggr.h',
                'src/nodejs/qm/qm_nodejs_streamaggr.cpp',
                'src/nodejs/qm/qm_nodejs_store.h',
                'src/nodejs/qm/qm_nodejs_store.cpp',
                'src/nodejs/qm/qm_param.h',
                # la
                'src/nodejs/la/la_nodejs.h',
                'src/nodejs/la/la_nodejs.cpp',
                'src/nodejs/la/la_structures_nodejs.h',
                'src/nodejs/la/la_structures_nodejs.cpp',
                'src/nodejs/la/la_vector_nodejs.h',
                # analytics
                'src/nodejs/analytics/analytics.h',
                'src/nodejs/analytics/analytics.cpp',
                # fs
                'src/nodejs/fs/fs_nodejs.h',
                'src/nodejs/fs/fs_nodejs.cpp',
                # snap
                'src/nodejs/snap/snap_nodejs.h',
                'src/nodejs/snap/snap_nodejs.cpp',
                # ht
                'src/nodejs/ht/ht_nodejs.h',
                'src/nodejs/ht/ht_nodejs.cpp',
                # statistics
                'src/nodejs/statistics/stat_nodejs.h',
                'src/nodejs/statistics/stat_nodejs.cpp',
                # StreamStory
                'src/third_party/streamstory/streamstory_node.h',
                'src/third_party/streamstory/streamstory_node.cpp',
                # addon utilities
                'src/nodejs/nodeutil.h',
                'src/nodejs/nodeutil.cpp',
                # init functions
                'src/nodejs/modinit.h',
                'src/nodejs/modinit.cpp',
                'src/nodejs/nodeutil.h',
                'src/nodejs/nodeutil.hpp',
                'src/nodejs/analytics/analytics.h',
                'src/nodejs/fs/fs_nodejs.h',
                'src/nodejs/ht/ht_nodejs.h',
                'src/nodejs/la/la_nodejs.h',
                'src/nodejs/la/la_structures_nodejs.h',
                'src/nodejs/la/la_vector_nodejs.h',
                'src/nodejs/qm/qm_nodejs.h',
                'src/nodejs/qm/qm_nodejs_store.h',
                'src/nodejs/qm/qm_nodejs_streamaggr.h',
                'src/nodejs/qm/qm_param.h',
                'src/nodejs/snap/snap_nodejs.h',
                'src/nodejs/statistics/stat_nodejs.h'
            ],
            'include_dirs': [
                'src/nodejs/qm',
                'src/nodejs/la',
                'src/nodejs/analytics',
                'src/nodejs/fs',
                'src/nodejs/snap',
                'src/nodejs/ht',
                'src/nodejs/statistics',
                # StreamStory
                'src/third_party/streamstory/',
                'src/nodejs/',
                'src/qminer/',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
                'src/glib/concurrent/',
                'src/third_party/sole/',
                'src/third_party/Snap/snap-core',
                'src/third_party/Snap/snap-adv',
                'src/third_party/Snap/snap-exp',
                'src/third_party/Snap/qlib-core',
                '<(LIN_ALG_INCLUDE)',
                '<(LIN_EIGEN_INCLUDE)'
            ],
            'dependencies': [
                'glib',
                'snap_lib',
                'qminer',
            ],
        },
        {
            # qminer library
            'target_name': 'qminer',
            'type': 'static_library',
            'sources': [
                'src/qminer/qminer_core.h',
                'src/qminer/qminer_core.cpp',
                'src/qminer/qminer_storage.h',
                'src/qminer/qminer_storage.cpp',
                'src/qminer/qminer_ftr.h',
                'src/qminer/qminer_ftr.cpp',
                'src/qminer/qminer_aggr.h',
                'src/qminer/qminer_aggr.hpp',
                'src/qminer/qminer_aggr.cpp',
                # StreamStory
                'src/third_party/streamstory/streamstory.h',
                'src/third_party/streamstory/streamstory.cpp',
                # Geospatial
                'src/third_party/geospatial/geospatial_aggr.h',
                'src/third_party/geospatial/geospatial_aggr.cpp'
            ],
            'include_dirs': [
                'src/qminer',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
                'src/third_party/geospatial/',
                'src/third_party/sole/',
                '<(LIN_ALG_INCLUDE)',
                '<(LIN_EIGEN_INCLUDE)'
            ],
        },
        {
            # snap external library
            'target_name': 'snap_lib',
            'type': 'static_library',
            'sources': [
                'src/third_party/Snap/snap-core/Snap.cpp'
            ],
            'include_dirs': [
                'src/third_party/Snap/snap-core',
                'src/third_party/Snap/snap-adv',
                'src/third_party/Snap/snap-exp',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
                '<(LIN_ALG_INCLUDE)',
                '<(LIN_EIGEN_INCLUDE)'
            ],
        },
        {
            # glib library
            'target_name': 'glib',
            'type': 'static_library',
            'sources': [
                'src/glib/base/base.h',
                'src/glib/base/base.cpp',
                'src/glib/mine/mine.h',
                'src/glib/mine/mine.cpp',
                'src/glib/concurrent/thread.h',
                'src/glib/concurrent/thread.cpp',
                'src/third_party/sole/sole.cpp',
                'src/third_party/libsvm/svm.h',
                'src/third_party/libsvm/svm.cpp'
            ],
            'include_dirs': [
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
                'src/glib/concurrent/',
                'src/third_party/sole/',
                'src/third_party/libsvm/',
                '<(LIN_ALG_INCLUDE)',
                '<(LIN_EIGEN_INCLUDE)'
            ],
        },
        {
            # needed for publishing binaries with node-pre-gyp
            'target_name': 'action_after_build',
            'type': 'none',
            'dependencies': [ 'qm' ],
            'copies': [{
                'files': [ '<(PRODUCT_DIR)/qm.node' ],
                'destination': './out/'
            }]
        }
    ]
}
