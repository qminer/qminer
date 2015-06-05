/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef mine_h
#define mine_h

#include <base.h>
#include <funrouter.h>

// utilitiesTFun
#include "cfyres.h"

// text processing
#include "stopword.h"
#include "stemming.h"
#include "StemmingTokenization.h"
#include "phrase.h"
#include "tokenizer.h"

#include "unicodebow.h"

// Bag-Of-Words
#include "bowbs.h"
#include "bowmd.h"
#include "bowclust.h"

// feature-generator
#include "ftrgen.h"

// Linear-Algebra
#include "bowlinalg.h"

// SVM
#include "svm.h"
#include "svmPrLoqo.h"
#include "svmbasic.h"
#include "strkernel.h"
#include "svmmodels.h"

// Kernel-Methods
#include "kernelmethods.h"
#include "semspace.h"

// some ML methods
#include "ml.h"

// clustering
#include "kmpp.h"
#include "hac.h"

// Markov Chains
#include "mc.h"

// cross-lingual
#include "crosslingual.h"

// Signal-Processing
#include "signalproc.h"

// Active-Learning
#include "bowactlearn.h"

// visualization
#include "vizmap.h"

// special datasets
#include "dmoz.h"

// hoeffding trees 
#include "hoeffding.h"

#endif
