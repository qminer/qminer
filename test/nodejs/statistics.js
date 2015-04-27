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
 
// 
// Sample unit test using standard assert JS library 
// 

var assert = require("assert")
//var la = require("../../../build/Debug/la.node")
//var stat = require('../../../build/Debug/stat.node');
var la = require('qminer').la;
var stat = require('qminer').statistics;

describe('Import test', function(){
    it('if import of qminer.node succeeds, return true', function(){
      assert.equal(1,1);
    })
})

var vec = new la.Vector([1, 2, 3])
var mat = new la.Matrix([[1, 2, 3], [2, 3, 4]])

describe('Testing mean functionalities...', function () {

    describe('Vector mean test: Testing vector is la.vector([1, 2, 3])', function () {

        it('stat.mean(vec) should return number 2', function () {
            assert.equal(stat.mean(vec), 2);
        })
        it('stat.mean(vec, 2) should return number 2', function () {
            assert.equal(stat.mean(vec, 2), 2);
        })
    })

    describe('Matrix mean test: Testing matrix is la.matrix([[1, 2, 3], [2, 3, 4]]', function () {

        it('stat.mean(mat) should return new vector [1.5, 2.5, 3.5]', function () {
            assert.equal(stat.mean(mat).toString(), new la.Vector([1.5, 2.5, 3.5]).toString());
        })
        it('stat.mean(mat, 2) should return new vector [2, 3]', function () {
            assert.equal(stat.mean(mat, 2).toString(), new la.Vector([2, 3]).toString());
        })
    })
})

describe('Testing std functionalities...', function () {

    describe('Matrix mean test: Testing matrix is la.matrix([[1, 2, 3], [2, 3, 4]]', function () {

        it('stat.std(mat) should return new vector [0.707107, 0.707107, 0.707107]', function () {
            assert.equal(stat.std(mat).toString(), new la.Vector([0.707107, 0.707107, 0.707107]).toString());
        })
        it('stat.std(mat, 0, 1) should return new vector [0.707107, 0.707107, 0.707107]', function () {
            assert.equal(stat.std(mat, 0, 1).toString(), new la.Vector([0.707107, 0.707107, 0.707107]).toString());
        })
        it('stat.std(mat, 1, 1) should return new vector [0.5, 0.5, 0.5]', function () {
            assert.equal(stat.std(mat, 1, 1).toString(), new la.Vector([0.5, 0.5, 0.5]).toString());
        })
        it('stat.std(mat, 0, 2) should return new vector [1, 1]', function () {
            assert.equal(stat.std(mat, 0, 2).toString(), new la.Vector([1, 1]).toString());
        })
        it('stat.std(mat, 1, 2) should return new vector [0.816497, 0.816497]', function () {
            assert.equal(stat.std(mat, 1, 2).toString(), new la.Vector([0.816497, 0.816497]).toString());
        })
    })
})