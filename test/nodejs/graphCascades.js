/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var qm = require('../../index.js');
var assert = require("../../src/nodejs/scripts/assert.js");

var testPrecision = 0.05;
var numSamples = 40000;

describe("Graph cascades", function () {
    describe("s -> a -> b test", function () {
        it("should return the correct posterior after observing s", function () {
            var pred = {
                dag: {
                    s: [],
                    a: ['s'],
                    b: ['a']
                },
                enabledNodes: ['s', 'a', 'b'],
                nodeModels: {
                    a: [0.2, 0.5, 0.3],
                    b: [0.7, 0.1, 0.2],
                },
                observations: {
                    s: 1475870511000,
                    a: 1475870511001,
                    b: 1475870511002,
                },
                timeUnit: 1
            };

            var graphPred = new qm.analytics.GraphCascade(pred);
            console.log("should return the correct posterior after observing s", 1);
            var obs = pred.observations;
            console.log("should return the correct posterior after observing s",2);
            var obsArr = Object.keys(obs).map(function (x) { return { node: x, date: obs[x] } });
            obsArr = obsArr.filter(function (x) { return x.date > 0 });
            obsArr.sort(function (x, y) { return x.date - y.date });
            console.log("should return the correct posterior after observing s",3);
            var observSeen = 1; // just 's'
            for (var i = 0; i < observSeen; i++) {
                graphPred.observeNode(obsArr[i].node, obsArr[i].date);
            }
            console.log("should return the correct posterior after observing s",4);
            graphPred.computePosterior(obsArr[observSeen - 1].date, numSamples);
            console.log("should return the correct posterior after observing s",5);
            var psum = [0, 0, 0, 0, 0]; // sum is 0, 1, 2, 3, or 4
            for (var i = 0; i < 3; i++) {
                for (var j = 0; j < 3; j++) {
                    var sum = i + j;
                    var p = pred.nodeModels.a[i] * pred.nodeModels.b[j];
                    psum[sum] += p;
                }
            }
            console.log("should return the correct posterior after observing s",6);
            var cumsum = 0.0;
            // test that cdf jumps at the predicted places
            for (var i = 0; i < psum.length - 1; i++) {
                cumsum += psum[i];
                var quantiles = [cumsum - testPrecision, cumsum + testPrecision];
                // clip
                quantiles = quantiles.map(function (x) { return x > 1 ? 1.0 : (x < 0 ? 0 : x); });
                var posterior = graphPred.getPosterior({
                    quantiles: quantiles
                });
                // posterior.b should jump by 1
                assert(posterior.b[1] - posterior.b[0] == 1);
                // posterior.b after jump should be i+1 time units after start
                assert(posterior.b[1] - obs.s == i + 1);
            }
            console.log("should return the correct posterior after observing s",7);
        });
        it("should return the correct posterior after observing s and a", function () {
            var pred = {
                dag: {
                    s: [],
                    a: ['s'],
                    b: ['a']
                },
                enabledNodes: ['s', 'a', 'b'],
                nodeModels: {
                    a: [0.2, 0.5, 0.3],
                    b: [0.7, 0.1, 0.2],
                },
                observations: {
                    s: 1475870511000,
                    a: 1475870511001,
                    b: 1475870511002,
                },
                timeUnit: 1
            };

            var graphPred = new qm.analytics.GraphCascade(pred);
            console.log("should return the correct posterior after observing s and a", 1);
            var obs = pred.observations;
            var obsArr = Object.keys(obs).map(function (x) { return { node: x, date: obs[x] } });
            obsArr = obsArr.filter(function (x) { return x.date > 0 });
            obsArr.sort(function (x, y) { return x.date - y.date });

            var observSeen = 2; // s and a
            for (var i = 0; i < observSeen; i++) {
                graphPred.observeNode(obsArr[i].node, obsArr[i].date);
            }
            graphPred.computePosterior(obsArr[observSeen - 1].date, numSamples);

            var psum = pred.nodeModels.b;
            var cumsum = 0.0;
            // test that cdf jumps at the predicted places
            for (var i = 0; i < psum.length - 1; i++) {
                cumsum += psum[i];
                var quantiles = [cumsum - testPrecision, cumsum + testPrecision];
                // clip
                quantiles = quantiles.map(function (x) { return x > 1 ? 1.0 : (x < 0 ? 0 : x); });
                var posterior = graphPred.getPosterior({
                    quantiles: quantiles
                });
                // posterior.b should jump by 1
                assert(posterior.b[1] - posterior.b[0] == 1);
                // posterior.b after jump should be i+1 time units after a
                assert(posterior.b[1] - obs.a == i + 1);
            }
        });
    });
    describe("s -> (a, b) -> c test", function () {
        it("should return the correct posterior after observing s", function () {
            var pred = {
                dag: {
                    s: [],
                    a: ['s'],
                    b: ['s'],
                    c: ['a', 'b']
                },
                enabledNodes: ['s', 'a', 'b', 'c'],
                nodeModels: {
                    a: [0.2, 0.5, 0.3],
                    b: [0.7, 0.1, 0.2],
                    c: [0, 1, 0],
                },
                observations: {
                    s: 1475870511000,
                    a: 1475870511001,
                    b: 1475870511002,
                    b: 1475870511004,
                },
                timeUnit: 1
            };

            var graphPred = new qm.analytics.GraphCascade(pred);

            var obs = pred.observations;
            var obsArr = Object.keys(obs).map(function (x) { return { node: x, date: obs[x] } });
            obsArr = obsArr.filter(function (x) { return x.date > 0 });
            obsArr.sort(function (x, y) { return x.date - y.date });

            var observSeen = 1; // just 's'
            for (var i = 0; i < observSeen; i++) {
                graphPred.observeNode(obsArr[i].node, obsArr[i].date);
            }
            graphPred.computePosterior(obsArr[observSeen - 1].date, numSamples);

            var psum = [0, 0, 0]; // max is 0, 1 or 2
            for (var i = 0; i < 3; i++) {
                for (var j = 0; j < 3; j++) {
                    var tmax = Math.max(i, j);
                    var p = pred.nodeModels.a[i] * pred.nodeModels.b[j];
                    psum[tmax] += p;
                }
            }
            var cumsum = 0.0;
            // test that cdf jumps at the predicted places
            for (var i = 0; i < psum.length - 1; i++) {
                cumsum += psum[i];
                var quantiles = [cumsum - testPrecision, cumsum + testPrecision];
                // clip
                quantiles = quantiles.map(function (x) { return x > 1 ? 1.0 : (x < 0 ? 0 : x); });
                var posterior = graphPred.getPosterior({
                    quantiles: quantiles
                });
                // posterior.c should jump by 1
                assert(posterior.c[1] - posterior.c[0] == 1);
                // posterior.b after jump should be i+1 time units after start
                assert(posterior.c[1] - obs.s == i + 1 + 1); // 1 for internal c computation, i + 1 for max(a,b)
            }
        });
        it("should return the correct posterior after observing s and a", function () {
            var pred = {
                dag: {
                    s: [],
                    a: ['s'],
                    b: ['s'],
                    c: ['a', 'b']
                },
                enabledNodes: ['s', 'a', 'b', 'c'],
                nodeModels: {
                    a: [0.2, 0.5, 0.3],
                    b: [0.7, 0.1, 0.2],
                    c: [0, 1, 0],
                },
                observations: {
                    s: 1475870511000,
                    a: 1475870511001,
                    b: 1475870511002,
                    b: 1475870511004,
                },
                timeUnit: 1
            };

            var graphPred = new qm.analytics.GraphCascade(pred);

            var obs = pred.observations;
            var obsArr = Object.keys(obs).map(function (x) { return { node: x, date: obs[x] } });
            obsArr = obsArr.filter(function (x) { return x.date > 0 });
            obsArr.sort(function (x, y) { return x.date - y.date });

            var observSeen = 2; // 's' and 'a'
            for (var i = 0; i < observSeen; i++) {
                graphPred.observeNode(obsArr[i].node, obsArr[i].date);
            }
            graphPred.computePosterior(obsArr[observSeen - 1].date, numSamples);

            var posterior = graphPred.getPosterior({
                    quantiles: [0.01, 0.32, 0.34, 0.99]
            });
            // since t = 1, b can be s + 1 (p = 1/3) or s + 2 (p = 2/3).
            // c is always b + 1
            assert(posterior.c[0] == obs.s + 2);
            assert(posterior.c[1] == obs.s + 2);
            assert(posterior.c[2] == obs.s + 3);
            assert(posterior.c[3] == obs.s + 3);
        });
    });
});
