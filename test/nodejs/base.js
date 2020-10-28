/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('../../index.js');
var fs = qm.fs;

var DB_PATH = 'db-strict';
var ILLEGAL_NAME = '#(@*&$^#(YR@#&_$YR%Y%_#!  %*';

describe('Testing base with strict names ...', function () {
    var base = null;

    beforeEach(function () {
        base = new qm.Base({
            mode: 'createClean',
            dbPath: DB_PATH
        });
    });
    afterEach(function () {
        console.log('Closing base (strict) ...');
        if (!base.isClosed()) base.close();
    });

    describe('Testing store creation ...', function () {
        it('Should be created without an exception', function (done) {
            try {
                var store = base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });
                done();
            } catch (e) {
                done(e);
            }
        });

        it('Should throw an exception', function (done) {
            try {
                var store = base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": ILLEGAL_NAME, "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });
                done(new Error('An exception was not thrown!'));
            } catch (e) {
                done();
            }
        });
    });

    describe('Testing save and load ...', function () {
        it('Should throw an exception', function (done) {
            try {
                base.close();
                var base1 = new qm.Base({
                    mode: 'open',
                    dbPath: DB_PATH
                });


                var store = base1.createStore({
                    "name": "People1",
                    "fields": [
                        { "name": ILLEGAL_NAME, "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });
                base1.close();
                done(new Error('Was able to create a store with an illegal name!'));
            } catch (e) {
                base1.close();
                done();
            }
        });
    });
});

describe('Testing base without strict names ...', function () {
    var base = null;

    beforeEach(function () {
        console.log('Creating base (not strict) ...');
        base = new qm.Base({
            mode: 'createClean',
            dbPath: DB_PATH,
            strictNames: false
        });
    });
    afterEach(function () {
        console.log('Closing base (not strict) ...');
        if (!base.isClosed()) base.close();
    });

    describe('Testing store creation ...', function () {
        it('Should be created without an exception', function (done) {
            try {
                var store = base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": "Name", "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });
                done();
            } catch (e) {
                done(e);
            }
        });

        it('Should not throw an exception', function (done) {
            try {
                var store = base.createStore({
                    "name": "People",
                    "fields": [
                        { "name": ILLEGAL_NAME, "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });
                done();
            } catch (e) {
                done(e);
            }
        });
    });

    describe('Testing save and load ...', function () {
        it('Should not throw an exception', function (done) {
            try {
                base.close();
                var base1 = new qm.Base({
                    mode: 'open',
                    dbPath: DB_PATH
                });


                var store = base1.createStore({
                    "name": "People1",
                    "fields": [
                        { "name": ILLEGAL_NAME, "type": "string", "primary": true },
                        { "name": "Gender", "type": "string", "shortstring": true }
                    ]
                });

                base1.close();
                done();
            } catch (e) {
                done(e);
            }
        });
    });
});
