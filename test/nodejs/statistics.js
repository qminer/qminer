/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
// 
// Sample unit test using standard assert JS library 
// 

var assert = require("../../src/nodejs/scripts/assert.js")
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

    describe('Vector std test: Testing vector is la.Vector([1, 2, 3])', function () {

        it('stat.std(vec) should return new value 2/3', function () {
            assert.eqtol(stat.std(vec), 1);
        })
        it('stat.std(mat, 1) should return new value 1', function () {
            assert.eqtol(stat.std(vec, 1), Math.sqrt(2/3));
        })
    })

    describe('Matrix std test: Testing matrix is la.matrix([[1, 2, 3], [2, 3, 4]])', function () {

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