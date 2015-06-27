{
    'variables': { 'LIN_ALG_BLAS%': 'NBLAS',
		   'LIN_ALG_LAPACKE%': 'NLAPACKE',	
   		   'LIN_ALG_INCLUDE%': 'src/glib/base/',
		   #full path to lapack or openblas libraries
		   'LIN_ALG_LIB%': '',
		   #64 bit indexing for BLAS
		   'INDEX_64%': 'NINDEX_64',
		   'INTEL%': 'NINTEL'
 
    },
    'target_defaults': {
        # GCC flags
        'cflags_cc!': [ '-fno-rtti', '-fno-exceptions' ],
        'cflags_cc': [ '-std=c++0x', '-frtti', '-fexceptions' ],
        'cflags': [ '-g', '-fexceptions', '-frtti', '-Wall', '-Wno-deprecated-declarations', '-fopenmp' ]
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
                'src/nodejs/qm/qm_param.h',
                # la
                'src/nodejs/la/la_nodejs.h',
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
                # addon utilities
                'src/nodejs/nodeutil.h',
                'src/nodejs/nodeutil.cpp',
                # init functions
                'src/nodejs/modinit.h',
                'src/nodejs/modinit.cpp'
            ],
            'include_dirs': [
                'src/nodejs/qm',
                'src/nodejs/la',
                'src/nodejs/analytics',
                'src/nodejs/fs',
                'src/nodejs/snap',
                'src/nodejs/ht',
                'src/nodejs/statistics',
                'src/nodejs/',
                'src/qminer/',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
                'src/third_party/Snap/snap-core',
                'src/third_party/Snap/snap-adv',
                'src/third_party/Snap/snap-exp',
                'src/third_party/Snap/qlib-core',
		'<(LIN_ALG_INCLUDE)'
            ],
            'defines': ['<(LIN_ALG_BLAS)',
			'<(LIN_ALG_LAPACKE)',
			'<(INDEX_64)',
			'<(INTEL)'
            ],
            'dependencies': [
                'glib',
                'snap_lib',
                'qminer'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid', '-fopenmp', '<(LIN_ALG_LIB)' ]}],
                ['OS == "mac"', {
                    "default_configuration": "Release",
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    },
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
                }],
                ['OS == "win"', {
                    'configurations': {
                        'Debug': {
                            'msvs_settings': {
                                'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
                                },
                            },
                        },
                        'Release': {
                            'msvs_settings': {
                                'VCCLCompilerTool': {
				    #'AdditionalIncludeDirectories': '<(LIN_ALG_INCLUDE)',	
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
				    'AdditionalDependencies': ['<(LIN_ALG_LIB)']	
                                },
                            },
                            'defines' : ['NDEBUG']
                        },
                    }
                }]
            ]
        },{
            # qminer library
            'target_name': 'qminer',
            'type': 'static_library',
            'sources': [
                'src/qminer/qminer_core.h',
                'src/qminer/qminer_core.cpp',
                'src/qminer/qminer_gs.h',
                'src/qminer/qminer_gs.cpp',
                'src/qminer/qminer_ftr.h',
                'src/qminer/qminer_ftr.cpp',
                'src/qminer/qminer_aggr.h',
                'src/qminer/qminer_aggr.cpp',
                'src/qminer/qminer_snap.h',
                'src/qminer/qminer_snap.cpp',
                'src/qminer/qminer_op.h',
                'src/qminer/qminer_op.cpp'
            ],        
            'include_dirs': [
                'src/qminer',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
	        '<(LIN_ALG_INCLUDE)'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid', '-fopenmp', '<(LIN_ALG_LIB)' ]}],
                ['OS == "mac"', {
                    "default_configuration": "Release",
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    },
                    "configurations": {
                        "Debug": {
                            "defines": [
                                "DEBUG"
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
                }],
                ['OS == "win"', {
                    'configurations': {
                        'Debug': {
                            'msvs_settings': {
                                'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
				    'AdditionalOptions': ['<(LIN_ALG_LIB)']					
                                },
                            },
                        },
                        'Release': {
                            'msvs_settings': {
                               'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
				    'AdditionalOptions': ['<(LIN_ALG_LIB)']
                                },
                            },
                            'defines' : ['NDEBUG']
                        },
                    }
                }]
            ]
        }, {
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
                'src/third_party/Snap/qlib-core',
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
		'<(LIN_ALG_INCLUDE)'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid', '-fopenmp', '<(LIN_ALG_LIB)' ]}],
                ['OS == "mac"', {
                    "default_configuration": "Release",
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    },
                    "configurations": {
                        "Debug": {
                            "defines": [
                                "DEBUG"
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
                }],
                ['OS == "win"', {
                    'configurations': {
                        'Debug': {
                            'msvs_settings': {
                               'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
				    'AdditionalOptions' : ['<(LIN_ALG_LIB)']
                                },
                            },
                        },
                        'Release': {
                            'msvs_settings': {
                               'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
				    'AdditionalOptions' : ['<(LIN_ALG_LIB)']
                                },
                            },
                            'defines' : ['NDEBUG']
                        },
                    }
                }]
            ]
        }, {
            # glib library
            'target_name': 'glib',
            'type': 'static_library',
            'sources': [
                'src/glib/base/base.h',
                'src/glib/base/base.cpp',
                'src/glib/mine/mine.h',
                'src/glib/mine/mine.cpp'
            ],        
            'include_dirs': [
                'src/glib/base/',
                'src/glib/mine/',
                'src/glib/misc/',
		'<(LIN_ALG_INCLUDE)'
            ],
            'defines': ['<(LIN_ALG_BLAS)',
			'<(LIN_ALG_LAPACKE)',
			'<(INDEX_64)',
			'<(INTEL)'
            ],
            'conditions': [
                # operating system specific parameters
                ['OS == "linux"', { 'libraries': [ '-lrt', '-luuid', '-fopenmp', '<(LIN_ALG_LIB)' ]}],
                ['OS == "mac"', {
                    "default_configuration": "Release",
                    'xcode_settings': {
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'OTHER_CFLAGS': [ '-std=c++11', '-stdlib=libc++' ],
                        'OTHER_LDFLAGS': [ '-undefined dynamic_lookup' ]
                    },
                    "configurations": {
                        "Debug": {
                            "defines": [
                                "DEBUG"
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
                }],
                ['OS == "win"', {
                    'configurations': {
                        'Debug': {
                            'msvs_settings': {
                               'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
				    'AdditionalOptions' : ['<(LIN_ALG_LIB)']
                                },
                            },
                        },
                        'Release': {
                            'msvs_settings': {
                               'VCCLCompilerTool': {
                                    'RuntimeTypeInfo': 'true',      # /GR
				    'OpenMP': 'true'	
                                },
                                'VCLinkerTool': {
                                    'SubSystem' : 1, # Console
      				    #'AdditionalDependencies' : ['<(LIN_ALG_LIB)']
			            'AdditionalOptions' : ['<(LIN_ALG_LIB)']
                                },
                            },
                            'defines' : ['NDEBUG']
                        },
                    }
                }]
            ]
        }
    ]
}