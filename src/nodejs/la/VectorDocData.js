/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
exports.view = {
	"title" : "Vector - array of doubles.",
	"className" : "Vector",
	"elementType": "number",

	"example1": "[1, 2, 3]",
    "input1": "4, 5",
	"output1": "'1, 2, 3'",
	"output2": "[1, 4, 5]",
    "output3": "[1]",

	"val1": "10",
	
	"sortCallback": "vectorCompareCb",
    "exampleSort": "[-2.0, 1.0, 3.0]",
    "inputSort": "function(arg1, arg2) { return Math.abs(arg1) - Math.abs(arg2); }",
    "outputSort": "[1.0, -2.0, 3.0]",
    "outputSortAsc": "[-2.0, 1.0, 3.0]",

	"skipSubVec": "",
	"skipInner": "",
	"skipSum": "",
	"skipGetMaxIdx": "",
	"skipSort": "",
	"skipOuter": "",
	"skipInner": "",
	"skipCosine": "",
	"skipPlus": "",
	"skipMinus": "",
	"skipMultiply": "",
	"skipNormalize": "",
	"skipDiag": "",
	"skipSpDiag": "",
	"skipNorm": "",
	"skipSparse": "",
	"skipToMat": "",
	"skipSave": "",
    "skipLoad": "",
    
    "defaultVal": "0.0",
}