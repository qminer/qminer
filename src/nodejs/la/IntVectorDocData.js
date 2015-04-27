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
exports.view = {
    "title": "Vector - array of integers",
    "className": "IntVector",
    "elementType": "number",

    "example1": "[1, 2, 3]",
    "input1": "4, 5",
    "output1": "'1, 2, 3'",
    "output2": "[1, 4, 5]",
    "output3": "[1]",

    "sortCallback": "intVectorCompareCb",
    "exampleSort": "[-2, 1, 3]",
    "inputSort": "function(arg1, arg2) { return Math.abs(arg1) - Math.abs(arg2); }",
    "outputSort": "[1, -2, 3]",
    "outputSortAsc": "[-2, 1, 3]",

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
    "skipNorm": "skip.",
    "skipSparse": "skip.",
    "skipToMat": "skip.",
    "skipSave": "",
    "skipLoad": "",
    
    "defaultVal": "0",
}