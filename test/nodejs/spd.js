/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var qm = require("qminer");
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for Kmeans

describe("SPD aggregate system running aggr tests", function () {
    var base = null;
    var store = null;
    var spdAggr = null;

    beforeEach(function(){
        base = new qm.Base({
            mode: "createClean",
            schema: [
                {
                    "name": "GPS",
                    "fields": [
                      { "name": "User", "type": "int" },
                      { "name": "Time", "type": "datetime" },
                      { "name": "Location", type: "float_pair" },
                      { "name": "Accuracy", type: "byte", "null": true }
                    ],
                    "joins": [],
                    "keys": []
                }
            ]
        });
        // used only for schema
        // will not be used to hold records (push will not be called)
        store = base.store("GPS");
        spdAggr = new qm.StreamAggr(base, {
            type: "stayPointDetector",
            store: store,
            userField: "User",
            timeField: "Time",
            locationField: "Location",
            accuracyField: "Accuracy",
            params: { dT: 50, tT: 300 }
        });
    });//beforeEach
    afterEach(function(){
        if (!base.isClosed()){
            base.close();
        }
    });//afterEach

    describe("Borderline usage (and arguments) tests", function () {
        it("should reject older or same age records that were already provided",
        function () {
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1342434474215, 
                    "accuracy": 26
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1342434474215, 
                    "accuracy": 26
                }
            ];
            
            for (var recIdx = 0; recIdx<points.length; recIdx++){
                var rec = points[recIdx];
                var qrec = store.newRecord({
                    User:1,
                    Time:rec.time,
                    Location: [rec.latitude, rec.longitude],
                    Accuracy: 4
                });
                spdAggr.onAdd(qrec);
            }
            result = spdAggr.saveJson(1);
            assert.equal(result.length,1);
            assert.equal(result[0].locations.length, 1);
            assert.equal(result[0].locationsNum, 1);
        });//it
        
        it("should return empty array when no records pushed in", function () {
            result = spdAggr.saveJson(1);
            assert.equal(result.length,0);
            result = spdAggr.saveJson(0);
            assert.equal(result.length,0);
        });//it

    });//describe
});
