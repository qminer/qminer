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

    /**
    * beforeEach
    */
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

    /**
    * Aftereach
    */
    afterEach(function(){
        if (!base.isClosed()){
            base.close();
        }
    });//afterEach
    
    /**
    * Borderline tests
    */
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
    });//describe borderline testsi

    /**
    * Internal state tests
    */
    describe("Internal state tests", function(){
        it("Should return saveJsonState with Objects of proper values",
        function(){
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 0, 
                    "accuracy": 26
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 100, 
                    "accuracy": 26
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1000, 
                    "accuracy": 26
                }
            ];
            
            for (var recIdx = 0; recIdx<points.length; recIdx++){
                var rec = points[recIdx];
                var qrec = store.newRecord({
                    User:1,
                    Time:rec.time,
                    Location: [rec.latitude, rec.longitude],
                    Accuracy: rec.accuracy
                });
                spdAggr.onAdd(qrec);
            }
            state = spdAggr.saveStateJson();
            assert.equal(state.locations[0].time, points[0].time);
            assert.equal(state.locations[1].time, points[1].time);
            assert.equal(state.locations[2].time, points[2].time);
            assert.equal(state.locations[0].latitude, points[0].latitude);
            assert.equal(state.locations[1].latitude, points[1].latitude);
            assert.equal(state.locations[2].latitude, points[2].latitude);
            assert.equal(state.locations[0].longitude, points[0].longitude);
            assert.equal(state.locations[1].longitude, points[1].longitude);
            assert.equal(state.locations[2].longitude, points[2].longitude);
            assert.equal(state.locations[0].accuracy, points[0].accuracy);
            assert.equal(state.locations[1].accuracy, points[1].accuracy);
            assert.equal(state.locations[2].accuracy, points[2].accuracy);
        });//it saveJsonState, loadJson state
        
        it("Should saveJsonState and LoadJson state wihtou losing data",
        function(){
            //these points are chosen so to produce one finished staypoint
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 0, 
                    "accuracy": 21
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1000, 
                    "accuracy": 22
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1342444474215,//100000 s later 
                    "accuracy": 23
                },
                {
                    "latitude": 47.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13424544474215,//100000 s later 
                    "accuracy": 24
                },
                {
                    "latitude": 48.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13424654474215,//100000 s later 
                    "accuracy": 25
                },
                {
                    "latitude": 49.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13425654474215,//100000 s later 
                    "accuracy": 26
                }
            ];
            
            for (var recIdx = 0; recIdx<points.length; recIdx++){
                var rec = points[recIdx];
                var qrec = store.newRecord({
                    User:1,
                    Time:rec.time,
                    Location: [rec.latitude, rec.longitude],
                    Accuracy: rec.accuracy
                });
                spdAggr.onAdd(qrec);
            }
            state = spdAggr.saveStateJson();
            var aggr2 = store.addStreamAggr({
                type: "stayPointDetector",
                timeField: "Time",
                locationField: "Location",
                accuracyField: "Accuracy",
                params: { dT: 51, tT: 301 }
            });
            aggr2.loadStateJson(state);
            state2 = aggr2.saveStateJson();
            assert.deepEqual(state2,state);
            //console.log(state); 
        });//it saveJsonState, loadJson state
    });//describe internal state tests
});
