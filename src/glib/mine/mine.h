/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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

// some ML methods
#include "ml.h"

// clustering
#include "kmpp.h"
#include "hac.h"
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
