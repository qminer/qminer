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
var la = require('../../index.js').la;
var stat = require('../../index.js').statistics;

describe('Import test', function(){
    it('if import of qminer.node succeeds, return true', function(){
        assert.strictEqual(1,1);
    })
})

var vec0 = new la.Vector([]);
var vec1 = new la.Vector([1]);
var vec = new la.Vector([1, 2, 3]);
var mat0 = new la.Matrix([]);
var mat1 = new la.Matrix([[]]);
var mat2 = new la.Matrix([[1]]);
var mat3 = new la.Matrix([[1, 1]]);
var mat = new la.Matrix([[1, 2, 3], [2, 3, 4]]);

describe('Testing mean functionalities...', function () {

    describe('Edge cases mean test', () => {

        it('should throw exception if vector of length 0 is sent to mean function', () => {
            assert.strictEqual(vec0.length, 0);
            assert.throws(() => { stat.mean(vec0) });
        })

        it('should throw exception if matrix [0x0] is sent to mean function', () => {
            assert.strictEqual(mat0.cols, 0);
            assert.strictEqual(mat0.rows, 0);
            assert.throws(() => { stat.mean(mat0) });
            assert.throws(() => { stat.mean(mat0, 1) });
            assert.throws(() => { stat.mean(mat0, 2) });
        })

        it('should throw exception if matrix [0x1] is sent to mean function', () => {
            assert.strictEqual(mat1.cols, 0);
            assert.strictEqual(mat1.rows, 1);
            assert.doesNotThrow(() => { stat.mean(mat1) });
            assert.doesNotThrow(() => { stat.mean(mat1, 1) });
            assert.throws(() => { stat.mean(mat1, 2) });
        })

        it('should throw exception if matrix [1x1] is sent to mean function', () => {
            assert.strictEqual(mat2.cols, 1);
            assert.strictEqual(mat2.rows, 1);
            assert.doesNotThrow(() => { stat.mean(mat2) });
            assert.doesNotThrow(() => { stat.mean(mat2, 1) });
            assert.doesNotThrow(() => { stat.mean(mat2, 2) });
        })
    })

    describe('Vector mean test: Testing vector is la.vector([1, 2, 3])', function () {

        it('stat.mean(vec) should return number 2', function () {
            assert.strictEqual(stat.mean(vec), 2);
        })
        it('stat.mean(vec, 2) should return number 2', function () {
            assert.strictEqual(stat.mean(vec, 2), 2);
        })
    })

    describe('Matrix mean test: Testing matrix is la.matrix([[1, 2, 3], [2, 3, 4]]', function () {

        it('stat.mean(mat) should return new vector [1.5, 2.5, 3.5]', function () {
            assert.strictEqual(stat.mean(mat).toString(), new la.Vector([1.5, 2.5, 3.5]).toString());
        })
        it('stat.mean(mat, 2) should return new vector [2, 3]', function () {
            assert.strictEqual(stat.mean(mat, 2).toString(), new la.Vector([2, 3]).toString());
        })
    })
})

describe('Testing std functionalities...', function () {

    describe('Edge cases std test:', function () {

        it('std of empty vetor should throw error', function () {
            assert.throws(() => { stat.std(vec0) });
            assert.throws(() => { stat.std(vec0, 1) });
            assert.throws(() => { stat.std(vec0, 0, 2) });
            assert.throws(() => { stat.std(vec0, 1, 2) });
        })

        it('std of vector of length 1 should throw error when flag is 0', function () {
            assert.throws(() => { stat.std(vec1), 0, 1 });
            assert.throws(() => { stat.std(vec1, 0, 2) });
            assert.doesNotThrow(() => { stat.std(vec1, 1) });
            assert.doesNotThrow(() => { stat.std(vec1, 1, 1) });
            assert.doesNotThrow(() => { stat.std(vec1, 1, 2) });
        })

        it('std of matrix [0x0] should return empty vetor', function () {
            assert.throws(() => { stat.std(mat0) });
            assert.throws(() => { stat.std(mat0, 1) });
            assert.throws(() => { stat.std(mat0, 0, 2) });
            assert.throws(() => { stat.std(mat0, 1, 2) });
        })

        it('std of matrix [0x1] should throw error when flag is 0 or if mat is column based', function () {
            assert.throws(() => { stat.std(mat1), 0, 1 });
            assert.throws(() => { stat.std(mat1, 0, 2) });
            assert.doesNotThrow(() => { stat.std(mat1, 1) });
            assert.doesNotThrow(() => { stat.std(mat1, 1, 1) });
            assert.throws(() => { stat.std(mat1, 1, 2) });
        })

        it('std of matrix [1x1] should throw error when flag is 0', function () {
            assert.throws(() => { stat.std(mat2), 0, 1 });
            assert.throws(() => { stat.std(mat2, 0, 2) });
            assert.doesNotThrow(() => { stat.std(mat2, 1) });
            assert.doesNotThrow(() => { stat.std(mat2, 1, 1) });
            assert.doesNotThrow(() => { stat.std(mat2, 1, 2) });
        })

        it('std of matrix [2x1] should throw error when flag is 0 and if mat is row based ', function () {
            assert.throws(() => { stat.std(mat3), 0, 1 });
            assert.doesNotThrow(() => { stat.std(mat3, 0, 2) });
            assert.doesNotThrow(() => { stat.std(mat3, 1) });
            assert.doesNotThrow(() => { stat.std(mat3, 1, 1) });
            assert.doesNotThrow(() => { stat.std(mat3, 1, 2) });
        })
    })

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
            assert.strictEqual(stat.std(mat).toString(), new la.Vector([0.707107, 0.707107, 0.707107]).toString());
        })
        it('stat.std(mat, 0, 1) should return new vector [0.707107, 0.707107, 0.707107]', function () {
            assert.strictEqual(stat.std(mat, 0, 1).toString(), new la.Vector([0.707107, 0.707107, 0.707107]).toString());
        })
        it('stat.std(mat, 1, 1) should return new vector [0.5, 0.5, 0.5]', function () {
            assert.strictEqual(stat.std(mat, 1, 1).toString(), new la.Vector([0.5, 0.5, 0.5]).toString());
        })
        it('stat.std(mat, 0, 2) should return new vector [1, 1]', function () {
            assert.strictEqual(stat.std(mat, 0, 2).toString(), new la.Vector([1, 1]).toString());
        })
        it('stat.std(mat, 1, 2) should return new vector [0.816497, 0.816497]', function () {
            assert.strictEqual(stat.std(mat, 1, 2).toString(), new la.Vector([0.816497, 0.816497]).toString());
        })
    })
})