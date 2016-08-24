/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
exports.view = {
    "title": "Vector - array of boolean.",
    "className": "BoolVector",
    "elementType": "boolean",

    "example1": "[true, true, false]",
    "input1": "false, true",
    "output1": "'true, true, false'",
    "output2": "[true, false, true]",
    "output3": "[true]",

	"val1": "false",
	
    "sortCallback": "boolVectorCompareCb",
    "exampleSort": "[true, false, false]",
    "inputSort": "function(arg1, arg2) { return arg2; }",
    "outputSort": "[false, true, true]",
    "outputSortAsc": "[false, true, true]",
    
	"skipSubVec": "skip.",
    "skipInner": "skip.",
    "skipSum": "skip.",
    "skipGetMaxIdx": "skip.",
    "skipSort": "skip.",
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
    
    "defaultVal": "false",
}
