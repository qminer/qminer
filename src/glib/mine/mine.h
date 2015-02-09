/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
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
#include "phrase.h"
#include "tokenizer.h"

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

// clustering
#include "kmpp.h"
#include "hac.h"
#include "clust.h"

// Markov Chains
#include "mc.h"

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
