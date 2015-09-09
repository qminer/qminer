/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

///////////////////////////////////////////////////////////
// Ridge Regression Unit Tests

var analytics = require('qminer').analytics;
var la = require('qminer').la;
var assert = require('../../src/nodejs/scripts/assert.js');

describe('Ridge Regression Tests', function () {
    
    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var reg = new analytics.RidgeReg(1.0);
            });
        })
    })
})
