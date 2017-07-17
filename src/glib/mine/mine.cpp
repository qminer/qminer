/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mine.h"

// index structures
//#include "sphere.cpp"
//#include "btree.cpp"

// utilities
#include "cfyres.cpp"

// text processing
#include "stopword.cpp"
#include "stemming.cpp"
#include "stemmingtokenization.cpp"
#include "phrase.cpp"
#include "tokenizer.cpp"
#include "unicodebow.cpp"

// Bag-Of-Words
#include "bowbs.cpp"
#include "bowmd.cpp"
#include "bowclust.cpp"

// feature-generator
#include "ftrgen.cpp"

// Linear-Algebra
#include "bowlinalg.cpp"

// SVM
#include "svm.cpp"
#include "svmPrLoqo.cpp"
#include "svmbasic.cpp"
#include "strkernel.cpp"
#include "svmmodels.cpp"

// Kernel-Methods
#include "kernelmethods.cpp"
#include "semspace.cpp"

// regression methods
#include "regression.cpp"
#include "classification.cpp"

// clustering
#include "clustering.cpp"

// Anomaly Detection
#include "anomaly.cpp"

// cross-lingual
#include "crosslingual.cpp"

// Signal-Processing
#include "signalproc.cpp"
// Online quantiles
#include "quantiles.cpp"

// Active-Learning
#include "bowactlearn.cpp"

// visualization
#include "vizmap.cpp"

// special datasets
#include "dmoz.cpp"

// hoeffding trees 
#include "hoeffding.cpp"

// non-negative matrix factorization
#include "nmf.cpp"

// kernel-density-estimation
#include "kerneldensity.cpp"
