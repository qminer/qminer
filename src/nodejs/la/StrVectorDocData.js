/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
exports.view = {
	"title" : "Vector - array of strings.",
	"className" : "StrVector",
	"elementType": "string",

	"example1": "['a', 'b', 'c']",
    "input1": "'d', 'e'",
    "output1": "'a, b, c'",
    "output2": "['a', 'd', 'e']",
    "output3": "['a']",
	
	"val1": "'xyz'",
	
	"sortCallback": "strVectorCompareCb",
    "exampleSort": "['asd', 'z', 'kkkk']",
    "inputSort": "function(arg1, arg2) { return arg1.length - arg2.length; }",
    "outputSort": "['z', 'asd', 'kkkk']",
    "outputSortAsc": "['asd', 'kkkk', 'z']",

	"skipSubVec": "",
	"skipInner": "skip.",
	"skipSum": "skip.",
	"skipGetMaxIdx": "skip.",
	"skipSort": "",
	"skipOuter": "skip.",
	"skipInner": "skip.",
	"skipCosine": "skip.",
	"skipPlus": "skip.",
	"skipMinus": "skip.",
	"skipMultiply": "skip.",
	"skipNormalize": "skip.",
	"skipDiag": "skip.",
	"skipSpDiag": "skip.",
    "skipSort": "skip.",
	"skipNorm": "skip.",
	"skipSparse": "skip.",
	"skipToMat": "skip.",
	"skipSave": "",
    "skipLoad": "",
    
    "defaultVal": "''",
}