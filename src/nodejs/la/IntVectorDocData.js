/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
exports.view = {
    "title": "Vector - array of integers.",
    "className": "IntVector",
    "elementType": "number",

    "example1": "[1, 2, 3]",
    "input1": "4, 5",
    "output1": "'1, 2, 3'",
    "output2": "[1, 4, 5]",
    "output3": "[1]",

	"val1": "10",
	
    "sortCallback": "intVectorCompareCb",
    "exampleSort": "[-2, 1, 3]",
    "inputSort": "function(arg1, arg2) { return Math.abs(arg1) - Math.abs(arg2); }",
    "outputSort": "[1, -2, 3]",
    "outputSortAsc": "[-2, 1, 3]",

	"skipSubVec": "",
    "skipInner": "skip.",
    "skipSum": "",
    "skipGetMaxIdx": "",
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
    
    "defaultVal": "0",
}