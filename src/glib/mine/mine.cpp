/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institut d.o.o.
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

#include "mine.h"

// utilities
#include "cfyres.cpp"

// text processing
#include "stopword.cpp"
#include "stemming.cpp"
#include "phrase.cpp"
#include "tokenizer.cpp"

// Bag-Of-Words
#include "bowbs.cpp"
#include "bowmd.cpp"
#include "bowclust.cpp"

// feature-generator
#include "ftrgen.cpp"

// Linear-Algebra
#include "bowlinalg.cpp"

// SVM
#include "svmPrLoqo.cpp"
#include "svmbasic.cpp"
#include "strkernel.cpp"
#include "svmmodels.cpp"

// Kernel-Methods
#include "kernelmethods.cpp"
#include "semspace.cpp"

// Active-Learning
#include "bowactlearn.cpp"

// visualization
#include "vizmap.cpp"

