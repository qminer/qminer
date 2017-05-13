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
                      { "name": "Speed", type: "float", "null": true },
                      { "name": "Distance", type: "float", "null": true },
                      { "name": "Accuracy", type: "byte", "null": true },
                      { "name": "Activities", type: "int_v", "null": true }
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
            speedField: "Speed",
            distanceField: "Distance",
            locationField: "Location",
            accuracyField: "Accuracy",
            activitiesField: "Activities",
            speedField: "Speed",
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
        it("should return empty array when no records pushed in", function () {
            result = spdAggr.saveJson(1);
            assert.equal(result.length, 0);
            result = spdAggr.saveJson(0);
            assert.equal(result.length, 0);
        });//it

        it("should reject older or same age records that were already provided",
        function () {
            console.log("HERE 0");
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1342434474215, 
                    "accuracy": 26,
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
                    Accuracy: 4,
                    Distance: null,
                    Activities: [1, 2, 3, 4]
                });
                spdAggr.onAdd(qrec);
            }
            result = spdAggr.saveJson(1);
            assert.equal(result.length,1);
            assert.equal(result[0].locations.length, 1);
            assert.equal(result[0].locationsNum, 1);
        });//it
    });//describe borderline testsi

    /**
    * Internal state tests
    */
    describe("Internal state tests", function(){
        it("Should return saveJsonState with Objects of proper values",
        function () {
            console.log("HERE 1");
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 0, 
                    "accuracy": 26,
                    "speed": 5.25,
                    "activities": [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, 17]
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 100, 
                    "accuracy": 26,
                    "speed": 5.25,
                    "activities": [22, 33, 44, 55, 66, 77, 88, 99,
                        10, 11, 12, 13, 14, 15, 16, 9,44]
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1000, 
                    "accuracy": 26,
                    "speed": 5.25,
                    "activities": [100, 99, 44, 55, 66, 77, 88, 99,
                        10, 11, 12, 13, 14, 99, 16,12,12]
                }
            ];
            
            for (var recIdx = 0; recIdx<points.length; recIdx++){
                var rec = points[recIdx];
                var qrec = store.newRecord({
                    User:1,
                    Time:rec.time,
                    Location: [rec.latitude, rec.longitude],
                    Activities: rec.activities,
                    Accuracy: rec.accuracy,
                    Distance: null,
                    Speed: rec.speed
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
            assert.equal(state.locations[0].speed, points[0].speed);
            assert.equal(state.locations[1].speed, points[1].speed);
            assert.equal(state.locations[2].speed, points[2].speed);
            assert.deepEqual(state.locations[0].activities,
                points[0].activities);
            assert.deepEqual(state.locations[1].activities,
                points[1].activities);
            assert.deepEqual(state.locations[2].activities,
                points[2].activities);
        });//it saveJsonState, loadJson state
        
        it("Should saveJsonState and LoadJson state wihtout losing data",
        function(){
            //these points are chosen so to produce one finished staypoint
            var points =[
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 0, 
                    "accuracy": 21,
                    "speed":1,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                        14, 15, 16]
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1000, 
                    "accuracy": 22,
                    "speed": 2,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                       14, 15, 16]
                },
                {
                    "latitude": 46.0423046, 
                    "longitude": 14.4875852,
                    "time": 1342444474215,//100000 s later 
                    "accuracy": 23,
                    "speed": 3,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                       14, 15, 16]
                },
                {
                    "latitude": 47.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13424544474215,//100000 s later 
                    "accuracy": 24,
                    "speed": 4,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                       14, 15, 16]
                },
                {
                    "latitude": 48.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13424654474215,//100000 s later 
                    "accuracy": 25,
                    "speed": 5,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                       14, 15, 16]
                },
                {
                    "latitude": 49.0423046,//lat +1 
                    "longitude": 14.4875852,
                    "time": 13425654474215,//100000 s later 
                    "accuracy": 26,
                    "speed": 6,
                    "activities": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                       14, 15, 16]
                }
            ];
            
            for (var recIdx = 0; recIdx<points.length; recIdx++){
                var rec = points[recIdx];
                var qrec = store.newRecord({
                    User:1,
                    Time:rec.time,
                    Location: [rec.latitude, rec.longitude],
                    Accuracy: rec.accuracy,
                    Speed: rec.speed,
                    Distance: null,
                    Activities: rec.activities
                });
                spdAggr.onAdd(qrec);
            }
            state = spdAggr.saveStateJson();
            var aggr2 = store.addStreamAggr({
                type: "stayPointDetector",
                timeField: "Time",
                locationField: "Location",
                speedField: "Speed",
                distanceField: "Distance",
                accuracyField: "Accuracy",
                activitiesField: "Activities",
                params: { dT: 51, tT: 301 }
            });
            aggr2.loadStateJson(state);
            state2 = aggr2.saveStateJson();

            assert.deepEqual(state2,state);
            //console.log(state); 
        });//it saveJsonState, loadJson state
    });//describe internal state tests
});//describe outer

/**
* TMD Averaging 
*/
describe("TMD averaging tests", function () {
    var base = null;
    var store = null;
    var spdAggr = null;

    /**
    * beforeEach
    */
    beforeEach(function () {
        base = new qm.Base({
            mode: "createClean",
            schema: [
                {
                    "name": "GPS",
                    "fields": [
                      { "name": "User", "type": "int" },
                      { "name": "Time", "type": "datetime" },
                      { "name": "Location", type: "float_pair" },
                      { "name": "Accuracy", type: "float", "null": true },
                      { "name": "Speed", type: "float", "null": true },
                      { "name": "Distance", type: "float", "null": true },
                      { "name": "Activities", type: "int_v", "null": true }
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
            speedField: "Speed",
            distanceField: "Distance",
            locationField: "Location",
            accuracyField: "Accuracy",
            activitiesField: "Activities",
            speedField: "Speed",
            params: { dT: 50, tT: 300 }
        });
    });//beforeEach

    /**
    * Aftereach
    */
    afterEach(function () {
        if (!base.isClosed()) {
            base.close();
        }
    });//afterEach

    it("should properly calculate sensor activity averages across geoActs",
    function () {
        var points = [
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 0,
                "accuracy": 26,
                "speed": 5.0,
                "activities": [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                       13, 14, 15,12]
            },
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 10000,
                "accuracy": 26,
                "speed": 6.5,
                "activities": [0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0,0]
            },
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 300000,
                "accuracy": 26,
                "speed": 8.0,
                "activities": [100, 100, 100, 100, 100, 100, 100, 100, 100,
                    100, 100, 100, 100, 100, 100, 100,100]
            }
        ];
        
        for (var recIdx = 0; recIdx < points.length; recIdx++) {
            var rec = points[recIdx];
            var qrec = store.newRecord({
                User: 1,
                Time: rec.time,
                Speed: rec.speed,
                Distance: null,
                Location: [rec.latitude, rec.longitude],
                Accuracy: rec.accuracy,
                Activities: rec.activities,
                Speed: rec.speed
            });
            spdAggr.onAdd(qrec);
        }
        result = spdAggr.saveJson(1);
        //current version of SPD will ignore the first point - it goes to path
        var acts1 = result[1].locations[0].activities;
        var acts2 = result[1].locations[1].activities;
        var avgs = acts1;
        for (var i = 0; i < acts2.length; i++) {
            avgs[i] = (acts1[i] + acts2[i]) / 2;
        }
        assert.deepEqual(avgs, result[1].activities);
    });//it

    it("should properly calculate UNKNOWN sensor act", function () {
        var points = [
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 0,
                "accuracy": 26,
                "activities": [100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            },
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 10000,
                "accuracy": 26,
                "activities": [100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0]
            },
            {
                "latitude": 46.0423046,
                "longitude": 14.4875852,
                "time": 300000,
                "accuracy": 26,
                "activities": [100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0]
            }
        ];

        for (var recIdx = 0; recIdx < points.length; recIdx++) {
            var rec = points[recIdx];
            var qrec = store.newRecord({
                User: 1,
                Time: rec.time,
                Location: [rec.latitude, rec.longitude],
                Accuracy: rec.accuracy,
                Activities: rec.activities,
                Speed: 0,
                Distance: null,
            });
            spdAggr.onAdd(qrec);
        }
        result = spdAggr.saveJson(1);
        //all geoActs should have [100, 0, 0, ...]
        for (var i = 0; i < result.length; i++) {
            var res = result[i];
            assert.deepEqual(res.activities, points[0].activities);
        }
    });//it

    it("should deal with states having empty activity vec", function () {
        var state = {
            "locations": [
                {
                    "time": 1487658300000,
                    "latitude": 46.037208557128906,
                    "longitude": 14.607879638671875,
                    "accuracy": 0,
                    "speed": 0.006789573316301387,
                    "distanceDiff": 577.1137318856179,
                    "timeDiff": 85000, "activities": []
                },
                {
                    "time": 1487658400000,
                    "latitude": 46.037208557128907,
                    "longitude": 14.607879638671877,
                    "accuracy": 10,
                    "speed": 0.006789573316301387,
                    "distanceDiff": 0.1137318856179,
                    "timeDiff": 400, "activities": []
                }]
        };
        spdAggr.loadStateJson(state);

        var points = [
            {
                "latitude": 46.037208557128907,
                "longitude": 14.607879638671877,
                "time": 1487658401000,
                "accuracy": 26,
                "activities": [100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0]
            }
        ];

        for (var recIdx = 0; recIdx < points.length; recIdx++) {
            var rec = points[recIdx];
            var qrec = store.newRecord({
                User: 1,
                Time: rec.time,
                Location: [rec.latitude, rec.longitude],
                Accuracy: rec.accuracy,
                Activities: rec.activities,
                Speed: 0,
                Distance: null,
            });
            spdAggr.onAdd(qrec);
        }
        result = spdAggr.saveJson(1);
        //all geoActs should have [100, 0, 0, ...]
        for (var i = 0; i < result.length; i++) {
            var res = result[i];
            assert.deepEqual(res.activities, points[0].activities);
        }
    });//it
});//describe outer sensor activity averaginng

