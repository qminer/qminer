/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('qminer');

describe('Feature Space Tests', function () {
    var base = undefined;
    var Store = undefined;

    beforeEach(function () {
        qm.delLock();
        var backward = require('../../src/nodejs/scripts/backward.js');
        backward.addToProcess(process); // adds process.isArg function

        base = new qm.Base({mode: "createClean"});
		// prepare test set
        base.createStore({
            "name": "FtrSpaceTest",
            "fields": [
              { "name": "Value", "type": "float" },
              { "name": "Category", "type": "string" },
              { "name": "Values", "type": "float_v" },
              { "name": "Categories", "type": "string_v" },
              { "name": "Date", "type": "datetime" },
              { "name": "Text", "type": "string" }
            ],
            "joins": [],
            "keys": []
        });
        Store = base.store("FtrSpaceTest");
        Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Barclays dropped a bombshell on its investment bankers last week." });
        Store.push({ Value: 1.1, Category: "b", Values: [1.1, 1.9], Categories: ["b", "w"], Date: "2014-10-11T00:11:22", Text: "Amid a general retreat by banks from bond trading and other volatile business lines, Barclays was particularly aggressive." });
        Store.push({ Value: 1.2, Category: "c", Values: [1.2, 1.8], Categories: ["c", "e"], Date: "2014-10-12T00:11:22", Text: "In what CEO Antony Jenkins dubbed a “bold simplification,” Barclays will cut 7,000 jobs in its investment bank by 2016 and will trim the unit to 30% of the group’s risk-weighted assets." });
        Store.push({ Value: 1.3, Category: "a", Values: [1.3, 1.7], Categories: ["a", "q"], Date: "2014-10-13T00:11:22", Text: "The bank is relegating £400 billion ($676 billion) in assets to its “non-core” unit, effectively quarantining them from the rest of the business." });
        Store.push({ Value: 1.4, Category: "b", Values: [1.4, 1.6], Categories: ["b", "w"], Date: "2014-10-14T00:11:22", Text: "Just about every large lender these days has chucked a chunk of its toxic cast-offs into a so-called “bad bank,” but none with the same zeal as Barclays." });
        Store.push({ Value: 1.5, Category: "c", Values: [1.5, 1.5], Categories: ["c", "e"], Date: "2014-10-15T00:11:22", Text: "The last time we drew up the league table for bad banks, UBS was on top." });
        Store.push({ Value: 1.6, Category: "a", Values: [1.6, 1.4], Categories: ["a", "q"], Date: "2014-10-16T00:11:22", Text: "But Barclays has now taken the crown, with “non-core” assets accounting for nearly 30% of the bank’s total balance sheet." });
        Store.push({ Value: 1.7, Category: "b", Values: [1.7, 1.3], Categories: ["b", "w"], Date: "2014-10-17T00:11:22", Text: "Details about a bad bank are typically relegated deep in the depths of a bank’s financial report, while it prominently trumpets the brilliant “adjusted” results of its unsullied core business." });
        Store.push({ Value: 1.8, Category: "c", Values: [1.8, 1.2], Categories: ["c", "e"], Date: "2014-10-18T00:11:22", Text: "These assets still belong to the bank, and factor into its capital ratios and other important measures of financial soundness." });
        Store.push({ Value: 1.9, Category: "a", Values: [1.9, 1.1], Categories: ["a", "q"], Date: "2014-10-19T00:11:22", Text: "But because selling everything at once would produce a huge loss, carving out an internal bad bank is the next best option." });
        Store.push({ Value: 2.0, Category: "b", Values: [2.0, 1.0], Categories: ["b", "w"], Date: "2014-10-20T00:11:22", Text: "The Barbie doll is an icon that young girls have played with since 1959, when Barbie settled in as an American fixture in the lives of children, first in the United States and in more recent years, worldwide." });

    });
    afterEach(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new feature space using a base and feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.notEqual(ftr, null);
        })
        it('should throw an exception, if feature extractor is not given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace(base);
            })
        })
        it('should throw an exception, if only the feature extractor is given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace({ type: "numeric", source: "FtrSpaceTest", field: "Value" });
            })
        })
        it('should throw an exception, if base is not a base', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace(10, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            })
        })
        it('should throw an exception, if json is not a json', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace(base, 10);
            })
        })
        it('should throw an exception, if no parameters are given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace();
            })
        })

        it('shoud construct a new feature space using a base and feature extractor, the extractor gets the store object as source', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: Store.name, field: "Value" });
            assert.notEqual(ftr, null);
        })
    });

    describe('Dim Tests', function () {
        it('should return the dimension of the feature space', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.notEqual(ftr, null);
            assert.equal(ftr.dim, 1);
        })
        it('should return the dimension of the feature space, where it takes two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.notEqual(ftr, null);
            assert.equal(ftr.dim, 4);
        })
        it('should return dimension of the feature space, where it takes multiple extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            assert.notEqual(ftr, null);
            assert.equal(ftr.dim, 16);
        })
    });

    describe('Dims Tests', function () {
        it('should return the dimensions of the feature space, for every extractor: 1 extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var arr = ftr.dims;
            assert.equal(arr.length, 1);
            assert.equal(arr[0], 1);
        })
        it('should return the dimensions of the feature space, for every extractor: 2 extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var arr = ftr.dims;
            assert.equal(arr.length, 2);
            assert.equal(arr[0], 1);
            assert.equal(arr[1], 3);
        })
        it('should return the dimensions of the feature space, for every extractor: 5 extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            var arr = ftr.dims;
            assert.equal(arr.length, 5);
            assert.equal(arr[0], 1);
            assert.equal(arr[1], 3);
            assert.equal(arr[2], 2);
            assert.equal(arr[3], 6);
            assert.equal(arr[4], 4);
        })
    });

    describe('Add Tests', function () {
        it('should put a new feature extractor in the feature space', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.equal(ftr.dim, 1);
            ftr.addFeatureExtractor({ type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            assert.equal(ftr.dim, 4);
        })
        it('should throw an exception, if the added feature extractor is not valid', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.addFeatureExtractor({ type: "categorical", source: "FtrSpaceTest" });
            })
        })
        it('should throw an exception, if there is no parameter given', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.addFeatureExtractor();
            })
        })
    });

    describe('FtrVec Test', function () {
        it('should return a vector for the first record in store: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 1);
            assert.equal(vec[0], 1.0);
        })
        it('should return a vector for the first record in store: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 1);
            assert.ok(0 <= vec[0] <= 1.0);
        })
        it('should return a vector for the first record in store: numeric', function () {
        	var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 1);
            assert.equal(vec[0], 1.0);

            var vec2 = ftr.extractVector(Store[1]);
            assert.equal(vec2.length, 1);
            assert.equal(vec2[0], 1.1);

            // test that we get the same for a JSON input
            var recJson = Store[0].toJSON();
            var vec3 = ftr.extractVector(recJson);
            assert.eqtol(vec3.minus(vec).norm(), 0);


        })
        it('should return a vector for the first record in store: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            var vec = ftr.extractVector(Store[0]);

            assert.equal(vec.length, 3);
            assert.equal(vec[0], 1);
            assert.equal(vec[1], 0);
            assert.equal(vec[2], 0);
        })
        it('should return a vector for the first record in store: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            var vec = ftr.extractVector(Store[0]);

            assert.equal(vec.length, 6);
            assert.equal(vec[0], 1);
            assert.equal(vec[1], 0);
            assert.equal(vec[2], 0);
            assert.equal(vec[3], 1);
            assert.equal(vec[4], 0);
            assert.equal(vec[5], 0);
        })
        it('should return a vector for the first record in store transformed using the logarithm', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "multinomial",
                source: "FtrSpaceTest",
                field: "Categories",
                values: ["a", "b", "c", "q", "w", "e"],
                transform: 'log'
            });
            var vec = ftr.extractVector(Store[0]);

            var expected = [1, 0, 0, 1, 0, 0];

            assert.equal(vec.length, expected.length);
            for (var i = 0; i < expected.length; i++) {
                assert.equal(vec[i], Math.log(expected[i] + 1));
            }
        })
        it('should return a vector for the first record in store: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", valueField: "Values", values: ["a", "b", "c", "q", "w", "e"] });
            var vec = ftr.extractVector(Store[0]);

            assert.equal(vec.length, 6);
            assert.equal(vec[0], 1)
            assert.equal(vec[1], 0);
            assert.equal(vec[2], 0);
            assert.equal(vec[3], 2);
            assert.equal(vec[4], 0);
            assert.equal(vec[5], 0);
        })
        it('should return a vector for the first record in store: text', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text", ngrams: [1, 4] });
            var vec = ftr.extractVector(Store[0]);

            assert.equal(vec.length, 0);
        })
        it('should return a vector for the first record in store: pair', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "pair", source: "FtrSpaceTest",
                first: { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                second: { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
            });

            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 0);
        })
        it('should return a vector for the first record in store: dateWindow', function () {
            var ftr = new qm.FeatureSpace(base, { type: "dateWindow", source: "FtrSpaceTest", field: "Date", window: 1, unit: "12hours" });
            var vec = ftr.extractVector(Store[0]);
            vec.print();
        })
        it('should return a vector for the first record in store: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 1);
            assert.equal(vec[0], 2);
        })
        it('should return a vector for the first record in store: two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var vec = ftr.extractVector(Store[0]);
            assert.equal(vec.length, 4);
            assert.equal(vec[0], 1.0);
            assert.equal(vec[1], 1);
            assert.equal(vec[2], 0);
            assert.equal(vec[3], 0);
        })
        it('should return a vector for the last record in store: multiple extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            var vec = ftr.extractVector(Store[10]);
            assert.equal(vec.length, 16);
            assert.equal(vec[0], 2.0);
            assert.equal(vec[1], 0);
            assert.equal(vec[2], 1);
            assert.equal(vec[3], 0);
            assert.equal(vec[6], 0);
            assert.equal(vec[7], 1);
            assert.equal(vec[8], 0);
            assert.equal(vec[9], 0);
            assert.equal(vec[10], 1);
            assert.equal(vec[11], 0);
        })
        it('should throw an exception, if no parameter is given', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            assert.throws(function () {
                var vec = ftr.extractVector();
            })
        })
        it('should throw an exception, if the parameter is not a record of type store', function () {
            base.createStore({
                "name": "Mobile",
                "fields": [
                  { "name": "TeaMobile", "type": "float" },
                  { "name": "AvtoMobile", "type": "string" },
                  { "name": "PerpetumMobile", "type": "string_v" },
                ],
                "joins": [],
                "keys": []
            });
            base.store('Mobile').push({ TeaMobile: 10, AvtoMobile: "car", PerpetumMobile: ["more", "cars"] });
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.extractVector(base.store('Mobile')[0]);
            });
        })
    });

    describe('FtrSpVec Tests', function () {
        it('should return a sparse vector for the first record in store: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 1);
            assert.equal(vec.at(0), 1.0);
        })
        it('should return a sparse vector for the first record in store: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 1);
            assert.ok(0 <= vec.at(0) <= 1.0);
        })
        it('should return a sparse vector for the first record in store: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 1);
            assert.equal(vec.at(0), 1.0);

            // test that we get the same for a JSON input
            var recJson = Store[0].toJSON();
            var vec3 = ftr.extractSparseVector(recJson);
            debugger
            assert.eqtol(vec3.full().minus(vec.full()).norm(), 0);
        })
        it('should return a sparse vector for the first record in store: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 3);
            assert.equal(vec.at(0), 1);
            assert.equal(vec.at(1), 0);
            assert.equal(vec.at(2), 0);
        })
        it('should return a sparse vector for the first record in store: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 6);
            assert.equal(vec.at(0), 1);
            assert.equal(vec.at(3), 1);
        })
        it('should return a sparse vector for the first record in store: text', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text" });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 0);
        })
        it('should return a sparse vector for the first record in store: pair', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "pair", source: "FtrSpaceTest",
                first: { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                second: { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
            });

            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 0);
        })
        it('should return a sparse vector for the first record in store: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 1);
            assert.equal(vec.at(0), 2);
        })
        it('should return a sparse vector for the first record in store: two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var vec = ftr.extractSparseVector(Store[0]);
            assert.equal(vec.dim, 4);
            assert.equal(vec.at(0), 1);
            assert.equal(vec.at(1), 1);
            assert.equal(vec.at(3), 0);
        })
        it('should return a sparse vector for the last record in store: multiple extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", valueField: "Values", values: ["a", "b", "c", "q", "w", "e"] }
            ]);
            var vec = ftr.extractSparseVector(Store[10]);
            assert.equal(vec.dim, 22);
            assert.equal(vec.at(0), 2.0);
            assert.equal(vec.at(1), 0);
            assert.equal(vec.at(2), 1);
            assert.equal(vec.at(3), 0);
            assert.equal(vec.at(6), 0);
            assert.equal(vec.at(7), 1);
            assert.equal(vec.at(8), 0);
            assert.equal(vec.at(9), 0);
            assert.equal(vec.at(10), 1);
            assert.equal(vec.at(11), 0);
            assert.equal(vec.at(10), 1);
            assert.equal(vec.at(17), 2);
            assert.equal(vec.at(20), 1);
        })
        it('should throw an exception, if no parameter is given', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            assert.throws(function () {
                var vec = ftr.extractSparseVector();
            })
        })
        it('should throw an exception, if the parameter is not a record of type store', function () {
            base.createStore({
                "name": "Mobile",
                "fields": [
                  { "name": "TeaMobile", "type": "float" },
                  { "name": "AvtoMobile", "type": "string" },
                  { "name": "PerpetumMobile", "type": "string_v" },
                ],
                "joins": [],
                "keys": []
            });
            base.store('Mobile').push({ TeaMobile: 10, AvtoMobile: "car", PerpetumMobile: ["more", "cars"] });
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.extractSparseVector(base.store('Mobile')[0]);
            });
        })
    });

    describe('InvertFeatureVector Tests', function () {
        it('should return the values of the first record: numeric, parameter is an array', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.invertFeatureVector([1]);

            assert.equal(vec.length, 1);
            assert.equal(vec[0], 1.0);
        })
        it('should return the values of the first record: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var ftrvec = ftr.extractVector(Store[0]);

            var vec = ftr.invertFeatureVector(ftrvec);
            assert.equal(vec.length, 1);
            assert.equal(vec[0], 1.0);
        })
        it('should invert a feature for extractor type: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            var ftrvec = ftr.extractVector(Store[0]);
            
            var inv = ftr.invertFeatureVector(ftrvec);
            
            assert.notNull(inv);
            assert.equal(1, inv.length);	// there is only 1 feature exractor defined
            
            var invFtr = inv[0];
            
            assert.notNull(invFtr);
            assert.deepEqual(invFtr, { a: 1, b: 0, c: 0 });
        })
        it('should invert a feature for extractor type: categorical, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 4, value: ["a", "b", "c"] });
            
            var ftrvec = ftr.extractVector(Store[0]);
            var inv = ftr.invertFeatureVector(ftrvec);
            var range = ftr.getFeatureRange(0);
            
            assert.notNull(inv);
            
            // extract the label
            var hashCategory = null;
            for (var i = 0; i < ftrvec.length; i++) {
            	if (ftrvec[i] > 0) {
            		hashCategory = range[i];
            	}
            }
            
            assert.notNull(hashCategory);
            assert.equal(1, inv.length);	// there is only 1 feature exractor defined
            
            var ftrInv = inv[0];            
            for (var key in ftrInv) {
            	if (key == hashCategory) {
            		assert.equal(1, ftrInv[key]);
            	}
            	else {
            		assert.equal(0, ftrInv[key]);
            	}
            }
        })
        it('should invert the numerical and categorical feature', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);

            var inv = ftr.invertFeatureVector([1, 1, 0, 0]);
            
            assert.equal(2, inv.length);
            
            var numInv = inv[0];
            var catInv = inv[1];
            
            assert.equal(numInv, 1);
            assert.deepEqual(catInv, { a: 1, b: 0, c: 0 });
        })
    });

    describe('InvertFeature Tests', function () {
        it('should inverse the value for extractor type: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var val = ftr.invertFeature(0, 1);

            assert.equal(val, 1.0);
        })
        it('should inverse the value for extractor type: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            ftr.updateRecords(Store.allRecords);
            var val = ftr.invertFeature(0, 0.8);
            assert.eqtol(val, 1.8);
        })
        it('should invert a feature vector for extractor type: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            
            var val = ftr.invertFeature(0, [1, 0, 0]);
            assert.deepEqual(val, { a: 1, b: 0, c: 0 });
        })
        it('should invert a feature vector for extractor type: categorical, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 4 });
            
            var inv = ftr.invertFeature(0, [1, 0, 0, 0]);
            var range = ftr.getFeatureRange(0);
            
            assert.equal(inv[range[0]], 1);
            assert.equal(inv[range[1]], 0);
            assert.equal(inv[range[2]], 0);
            assert.equal(inv[range[3]], 0);
        })
        it('should get the value by using the first extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", value: ["a", "b", "c"] }
            ]);
            var val = ftr.invertFeature(0, 5.0);

            assert.equal(val, 5);
        })
    })
    
    describe('Range tests', function () {
    	it('Should return [a, b, c]', function () {
    		var origRange = ['a', 'b', 'c'];
    		var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: origRange });
    		
    		var range = ftr.getFeatureRange(0);
    		for (var i = 0; i < origRange.length; i++) {
    			assert.equal(origRange[i], range[i]);
    		}
    	});
    });

    describe('GetFeatureExtractor Tests', function () {
        it('should return the name of the first feature extractor: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Constant");
        })
        it('should return the name of the first feature extractor: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Random");
        })
        it('should return the name of the first feature extractor: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Numeric[Value]");
        })
        it('should return the name of the first feature extractor: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Categorical[Category]");
        })
        it('should return the name of the first feature extractor: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Multinomial[Categories]");
        })
        it('should return the name of the first feature extractor: text', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text", ngrams: [1, 4] });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "BagOfWords[Text]");
        })
        it('should return the name of the first feature extractor: pair', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "pair", source: "FtrSpaceTest",
                first: { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                second: { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
            });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "Pair[Categorical[Category],Multinomial[Categories]]");
        })
        it('should return the name of the first feature extractor: dateWindow', function () {
            var ftr = new qm.FeatureSpace(base, { type: "dateWindow", source: "FtrSpaceTest", field: "Date", window: 3, unit: "12hours" });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "DateWnd[Date]");
        })
        it('should return the name of the first feature extractor: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var name = ftr.getFeatureExtractor(0);
            assert.equal(name, "TestFunc");
        })
        it('should return the name of the second feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFeatureExtractor(1);
            assert.equal(name, "Categorical[Category]");
        })
        it('should throw an exception, if index is out of bound, idx = 2', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFeatureExtractor(2);
            })
        })
        it('should throw an exception, if index is out of bound, idx = -1', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFeatureExtractor(-1);
            })
        })
        it('should throw an exception, if the parameter is a feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                var name = ftr.getFeatureExtractor({ type: "numeric", source: "FtrSpaceTest", field: "Value" });
            })
        })
    });

    describe('GetFeature Tests', function () {
        it('should return the name of the feature of extractor type: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var name = ftr.getFeature(0);
            assert.equal(name, "Constant");
        })
        it('should return the name of the feature of extractor type: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var name = ftr.getFeature(0);
            assert.equal(name, "Random");
        })
        it('should return the name of the feature of extractor type: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var name = ftr.getFeature(0);
            assert.equal(name, "Numeric[Value]");
        })
        it('should return the name of the feature of extractor type: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value", normalize: true });
            var name = ftr.getFeature(0);
            assert.equal(name, "Numeric[Value]");
        })
        it('should return the name of the features of extractor type: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            assert.equal(ftr.getFeature(0), "a");
            assert.equal(ftr.getFeature(1), "b");
            assert.equal(ftr.getFeature(2), "c");
        })
        it('should return the name of the features of extractor type: categorical, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 3 });
            assert.equal(ftr.getFeature(0), "0");
            assert.equal(ftr.getFeature(1), "1");
            assert.equal(ftr.getFeature(2), "2");
        })
        it('should return the name of the features of extractor type: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            assert.equal(ftr.getFeature(0), "a");
            assert.equal(ftr.getFeature(1), "b");
            assert.equal(ftr.getFeature(2), "c");
            assert.equal(ftr.getFeature(3), "q");
            assert.equal(ftr.getFeature(4), "w");
            assert.equal(ftr.getFeature(5), "e");
        })
        it('should return the name of the features of extractor type: multinomial, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 3 });
            assert.equal(ftr.getFeature(0), "0");
            assert.equal(ftr.getFeature(1), "1");
            assert.equal(ftr.getFeature(2), "2");
        })
        it('should return the name of the features of extractor type: text', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text", hashDimension: 3, ngrams: [1, 4] });
            assert.equal(ftr.getFeature(0), "0");
            assert.equal(ftr.getFeature(1), "1");
            assert.equal(ftr.getFeature(2), "2");
        })
        it('should throw an exception for extractor type: pair', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "pair", source: "FtrSpaceTest",
                first: { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                second: { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
            });
            assert.throws(function () {
                var type = ftr.getFeature(0);
            });
        })
        it('should reutrn the name of the feature of extractor type: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            assert.equal(ftr.getFeature(0), "TestFunc[0]");
        })
        it('should return the name of the feature at second position', function () {
            var ftr = new qm.FeatureSpace(base, [
              { type: "numeric", source: "FtrSpaceTest", field: "Value" },
              { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFeature(1);
            assert.equal(name, "a");
        })
        it('should return the name of the feature at third position', function () {
            var ftr = new qm.FeatureSpace(base, [
             { type: "numeric", source: "FtrSpaceTest", field: "Value" },
             { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFeature(2);
            assert.equal(name, "b");
        })
        it('should throw an exception, if the index is out of bound, idx = 4', function () {
            var ftr = new qm.FeatureSpace(base, [
             { type: "numeric", source: "FtrSpaceTest", field: "Value" },
             { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFeature(4);
            })
        })
        it('should return the name of the first feature if idx < 0', function () {
            var ftr = new qm.FeatureSpace(base, [
                        { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                        { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFeature(-1);
            assert.equal(name, "Numeric[Value]");
        })
    });

    describe('Filter Tests', function () {
        it('should return only the features of a sparse vector for extractor type: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a dense vector for extractor type: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 1);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a sparse vector for extractor type: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.ok(0 <= out_vec.at(0) <= 1);
        })
        it('should return only the features of a dense vector for extractor type: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 1);
            assert.ok(0 <= out_vec.at(0) <= 1);
        })
        it('should return only the features of a sparse vector for extractor type: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a dense vector for extractor type: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 1);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a sparse vector for extractor type: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value", normalize: true });
            ftr.updateRecords(Store.allRecords);

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 0);
        })
        it('should return only the features of a dense vector for extractor type: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value", normalize: true });
            ftr.updateRecords(Store.allRecords);

            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 1);
            assert.equal(out_vec.at(0), 0);
        })
        it('should return only the features of a sparse vector for extractor type: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
        })
        it('should return only the features of a dense vector for extractor type: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });

            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 3);
            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
        })
        it('should return only the features of a sparse vector for extractor type: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 2);
            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
            assert.equal(out_vec.at(3), 1);
            assert.equal(out_vec.at(4), 0);
            assert.equal(out_vec.at(5), 0);
        })
        it('should return only the features of a dense vector for extractor type: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });

            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 6);
            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
            assert.equal(out_vec.at(3), 1);
            assert.equal(out_vec.at(4), 0);
            assert.equal(out_vec.at(5), 0);
        })
        it('should return only the features of a sparse vector for extractor type: text', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 2);
        })
        it('should return only the features of a dense vector for extractor type: text', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 1);
            assert.equal(out_vec.at(0), 2);
        })
        it('should return only the features of a sparse vector for a given feature extractor id = 0', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a sparse vector for the given feature extractor id = 1', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 1);

            assert.equal(out_vec.nnz, 1);
            assert.equal(out_vec.at(1), 1);
        })
        it('should return only the features of a sparse vector for the given extractor id = 3, offset = false', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.extractSparseVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 3, false);

            assert.equal(out_vec.nnz, 2);
            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(3), 1);
        })

        it('should return only the features of a vector for a given extractor id = 0', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            assert.equal(out_vec.length, 16);
            assert.equal(out_vec.at(0), 1);
        })
        it('should return only the features of a vector for a given extractor id = 1, offset = false', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.extractVector(Store[0]);
            var out_vec = ftr.filter(in_vec, 1, false);

            assert.equal(out_vec.length, 3);
            assert.equal(out_vec.at(0), 1);
        })
        it('should throw an exception if the vector length is less than the start index of the extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            var in_vec = new qm.la.Vector([1, 0, 0, 1]);
            assert.throws(function () {
                var out_vec = ftr.filter(in_vec, 3, false);
            })

        })
    });

    describe('UpdateRecord Tests', function () {
        it('should update the feature space with a new record: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
        })
        it('should update the feature space with a new record: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            ftr.updateRecord(Store[0]);
            assert.ok(0 <= ftr.extractVector(Store[0]).at(0) <= 1);
        })
        it('should update the feature space with a new record: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });

            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);

            ftr.updateRecord(Store[1]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[1]).at(0), 1.1);

            ftr.updateRecord(Store[2]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 1.1);
            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1.2);
        })
        it('should update the feature space with a new record: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });

            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);

            ftr.updateRecord(Store[1]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 0);
            assert.equal(ftr.extractVector(Store[1]).at(0), 1);

            ftr.updateRecord(Store[2]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 0.5);
            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[3]).at(0), 1);
        })
        it('should update the feature space with a new record: numeric, normalize (new syntax)', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: "scale", field: "Value" });

            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);

            ftr.updateRecord(Store[1]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 0);
            assert.equal(ftr.extractVector(Store[1]).at(0), 1);

            ftr.updateRecord(Store[2]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 0.5);
            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[3]).at(0), 1);
        })
        it('should update the feature space with a new record: numeric, no normalize (new syntax)', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: "none", field: "Value" });

            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);

            ftr.updateRecord(Store[1]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[1]).at(0), 1.1);

            ftr.updateRecord(Store[2]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 1.1);
            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1.2);
        })
        it('should update the feature space with a new record: numeric, normalize variance (new syntax)', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: "var", field: "Value" });

            ftr.updateRecord(Store[0]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 0);

            ftr.updateRecord(Store[1]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), -0.707106781, 0.0000001);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 0.707106781, 0.0000001);

            ftr.updateRecord(Store[2]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), -1, 0.0000001);
            assert.eqtol(ftr.extractVector(Store[1]).at(0), 0, 0.0000001);
            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1, 0.0000001);

            for (var i = 3; i < 11; i++){
            	ftr.updateRecord(Store[i].toJSON()); // test if updates work on JSON arguments
			}
            var expected = [
				-1.507556723,
				-1.206045378,
				-0.904534034,
				-0.603022689,
				-0.301511345,
				0,
				0.301511345,
				0.603022689,
				0.904534034,
				1.206045378,
				1.507556723
			];
             for (var i = 0; i < 11; i++){
				assert.eqtol(ftr.extractVector(Store[i]).at(0), expected[i], 0.0000001);
			}
        })
        it('should update the feature space with a new record: categorical', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            );
            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[0]).at(1), 0);
            assert.equal(ftr.extractVector(Store[0]).at(2), 0);
        })
        it('should update the feature space with a new record: categorical, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 3 }
            );
            ftr.updateRecord(Store[0]);
            assert.equal(ftr.extractVector(Store[0]).at(0), 0);
            assert.equal(ftr.extractVector(Store[0]).at(1), 1);
            assert.equal(ftr.extractVector(Store[0]).at(2), 0);
        })
        it('should update the feature space with a new record: multinomial', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories" }
            );

            assert.equal(ftr.dim, 0);
            ftr.updateRecord(Store[0]);
            assert.equal(ftr.dim, 2);
            ftr.updateRecord(Store[1]);
            assert.equal(ftr.dim, 4);
            ftr.updateRecord(Store[2]);
            assert.equal(ftr.dim, 6);
            ftr.updateRecord(Store[3]);
            assert.equal(ftr.dim, 6);
        })
        it('should update the feature space with a new record: multinomial', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] }
            );

            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 1);

            ftr.updateRecord(Store[0]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 1);
        })
        it('should update the feature space with a new record: multinomial', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", valueField: "Values", values: ["a", "b", "c", "q", "w", "e"] }
            );

            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 2);

            ftr.updateRecord(Store[0]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 2);
        })
        it('should update the feature space with a new record: multinomial, normalize', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
            );
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 1 / Math.sqrt(2));

            ftr.updateRecord(Store[0]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 1 / Math.sqrt(2));
        })
        it('should update the feature space with a new record: multinomial, normalize', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", valueField: "Values", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
            );
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1 / Math.sqrt(5));
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 2 / Math.sqrt(5));

            ftr.updateRecord(Store[0]);
            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1 / Math.sqrt(5));
            assert.eqtol(ftr.extractVector(Store[0]).at(3), 2 / Math.sqrt(5));
        })
        it('should update the feature space with a new record: text', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text", ngrams: [1, 1] });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            assert.equal(ftr.extractVector(Store[11]).length, 2);
            assert.equal(ftr.extractVector(Store[11]).at(0), 1);
            assert.equal(ftr.extractVector(Store[11]).at(1), 0);
            assert.equal(ftr.extractVector(Store[12]).length, 2);
            assert.equal(ftr.extractVector(Store[12]).at(0), 0);
            assert.equal(ftr.extractVector(Store[12]).at(1), 1);
        })
        it('should update the feature space with a new record: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            ftr.updateRecord(Store[0]);
            ftr.updateRecord(Store[1]);

            assert.equal(ftr.extractVector(Store[0]).length, 1);
            assert.equal(ftr.extractVector(Store[0]).at(0), 2);

            assert.equal(ftr.extractVector(Store[1]).length, 1);
            assert.equal(ftr.extractVector(Store[1]).at(0), 2);
        })
        it('should return the correct value based on the last update', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            ftr.updateRecord(Store[0]);
            ftr.updateRecord(Store[1]);

            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1);
        })
        it('should update the feature space with a new record, numeric, categorical and multinomial', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
            ]);

            assert.eqtol(ftr.extractVector(Store[0]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(1), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(4), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[0]).at(7), 1 / Math.sqrt(2));


            ftr.updateRecord(Store[0]);
            ftr.updateRecord(Store[1]);
            ftr.updateRecord(Store[2]);

            assert.eqtol(ftr.extractVector(Store[0]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[0]).at(1), 1);
            assert.eqtol(ftr.extractVector(Store[0]).at(4), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[0]).at(7), 1 / Math.sqrt(2));

            assert.eqtol(ftr.extractVector(Store[1]).at(0), 1 / 2);
            assert.eqtol(ftr.extractVector(Store[1]).at(2), 1);
            assert.eqtol(ftr.extractVector(Store[1]).at(5), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[1]).at(8), 1 / Math.sqrt(2));

            assert.eqtol(ftr.extractVector(Store[2]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[2]).at(3), 1);
            assert.eqtol(ftr.extractVector(Store[2]).at(6), 1 / Math.sqrt(2));
            assert.eqtol(ftr.extractVector(Store[2]).at(9), 1 / Math.sqrt(2));
        })
    });

    describe('Tokenizer Tests', function () {
        it('should update the feature space, text: default settings', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", field: "Text" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabeth a a" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 3);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[11]).at(1), 0);
            assert.eqtol(ftr.extractVector(Store[11]).at(2), 0);

            assert.equal(ftr.extractVector(Store[12]).length, 3);
            assert.eqtol(ftr.extractVector(Store[12]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[12]).at(1), 1);
            assert.eqtol(ftr.extractVector(Store[12]).at(2), 0);

            assert.equal(ftr.extractVector(Store[13]).length, 3);
            assert.eqtol(ftr.extractVector(Store[13]).at(0), Math.log(2) / Math.sqrt(Math.log(2) * Math.log(2) + Math.log(4 / 3) * Math.log(4 / 3)));
            assert.eqtol(ftr.extractVector(Store[13]).at(1), Math.log(4 / 3) / Math.sqrt(Math.log(2) * Math.log(2) + Math.log(4 / 3) * Math.log(4 / 3)));
            assert.eqtol(ftr.extractVector(Store[13]).at(2), 0);

            assert.equal(ftr.extractVector(Store[14]).length, 3);
            assert.eqtol(ftr.extractVector(Store[14]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[14]).at(1), Math.log(4 / 3) / Math.sqrt(Math.log(4) * Math.log(4) + Math.log(4 / 3) * Math.log(4 / 3)));
            assert.eqtol(ftr.extractVector(Store[14]).at(2), Math.log(4) / Math.sqrt(Math.log(4) * Math.log(4) + Math.log(4 / 3) * Math.log(4 / 3)));
        })
        it('should update the feature space, text: normalize-false, other default settings', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", normalize: false, field: "Text" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabeth a a" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 3);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), 0);
            assert.eqtol(ftr.extractVector(Store[11]).at(2), 0);

            assert.equal(ftr.extractVector(Store[12]).length, 3);
            assert.eqtol(ftr.extractVector(Store[12]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[12]).at(2), 0);

            assert.equal(ftr.extractVector(Store[13]).length, 3);
            assert.eqtol(ftr.extractVector(Store[13]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[13]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[13]).at(2), 0);

            assert.equal(ftr.extractVector(Store[14]).length, 3);
            assert.eqtol(ftr.extractVector(Store[14]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[14]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[14]).at(2), Math.log(4));
        })
        it('should update the feature space, text: normalize-false, ngrams[2, 3]', function () {
            var ftr = new qm.FeatureSpace(base, { type: "text", source: "FtrSpaceTest", normalize: false, field: "Text", ngrams: [2, 3] });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet Beta, Gamma" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabet Beta Omega" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabeth a a Omicron, Omicron" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 12);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(4));
        })
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "none" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "none" }
            });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet Beta, Gamma" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabet Beta Omega" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabeth a a Omicron, Omicron" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 8);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[12]).at(3), Math.log(4 / 3));

            assert.eqtol(ftr.extractVector(Store[13]).at(4), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(5), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(6), 2 * Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(6), 2 * Math.log(4));

        })
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "en" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "en" }
            });

            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha Alphabeth a a" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 3);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), 0);
            assert.eqtol(ftr.extractVector(Store[11]).at(2), 0);

            assert.equal(ftr.extractVector(Store[12]).length, 3);
            assert.eqtol(ftr.extractVector(Store[12]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[12]).at(2), 0);

            assert.equal(ftr.extractVector(Store[13]).length, 3);
            assert.eqtol(ftr.extractVector(Store[13]).at(0), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[13]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[13]).at(2), 0);

            assert.equal(ftr.extractVector(Store[14]).length, 3);
            assert.eqtol(ftr.extractVector(Store[14]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[14]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[14]).at(2), Math.log(4));
        })
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "si" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "si" }
            });

            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Abeceda" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Miloš zna abecedo" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Kaj pa to Miloš Abeceda?" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Miloš in Abeceda" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 4);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[12]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(3), Math.log(4));
        })
        // TODO check if everything works allright (no idea how it should parse for es)
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "es" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "es" }
            });

            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "¿Quién se robó mi teléfono?" });         // who stole my phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Se robó el teléfono!" });                // he stole the phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Por cuánto usted vende el teléfono?" }); // for how much do you sell the phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "mamá tiene un nuevo teléfono." });       // mom has a new phone

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 8);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(3), Math.log(1));


            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[12]).at(2), 0);
            assert.eqtol(ftr.extractVector(Store[12]).at(3), Math.log(1));

            assert.eqtol(ftr.extractVector(Store[13]).at(4), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(5), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(6), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(7), Math.log(4));
        })
        // TODO check if everything works allright (no idea how it should parse for de)
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "de" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "de" }
            });

            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Die mein Handy gestohlen?" });           // who stole my phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "er habe das Handy!" });                  // he stole the phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Für wie viel wissen Sie das Telefon verkaufen?" }); // for how much do you sell the phone
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Mutter hat ein neues Handy." });       // mom has a new phone

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 12);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(4 / 3));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[12]).at(3), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(4), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(4 / 3));

            assert.eqtol(ftr.extractVector(Store[13]).at(5), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(6), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(7), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(8), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(9), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(10), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(11), Math.log(4));
        })
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: ["an", "a", "butterfly"] }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: ["an", "a", "butterfly"] }
            });

            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Have you seen the butterfly?" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "An elephant and a butterfly." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Is you is or is you ain't my baby?" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "The butterfly dagger!" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 12);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(3), Math.log(2));


            assert.eqtol(ftr.extractVector(Store[12]).at(4), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(5), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[13]).at(6), 3 * Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(7), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(8), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(9), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(10), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(11), Math.log(4));
        })
        it('should update the feature space, text: normalize-false, tokenizer: { type: "simple", stopwords: "none", uppercase: false }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "none", uppercase: false }
            });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Alpha alphabet" });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "alpha Alphabeth a a" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 6);
            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(1), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[13]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(3), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(4), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(5), 2 * Math.log(4));
        })
        it('should update the feature space, text: normalize: false, tokenizer: { type : "simple", stopwords: "none", stemmer: "false" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "none", stemmer: false }
            });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Jimmy Carr bought a car." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "My dad sold a lot of cars." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "The union of dads are buying cars." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Car Cars Car, buy them!" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(3), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(4), Math.log(2));

            assert.eqtol(ftr.extractVector(Store[12]).at(5), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(6), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(7), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(8), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(9), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[12]).at(10), Math.log(4 / 3));

            assert.eqtol(ftr.extractVector(Store[13]).at(11), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(12), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(13), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(14), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(15), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(16), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[14]).at(17), Math.log(4));
        })
        //TODO stemmer changes BUY into BUI and JIMMY into JIMMI, but not MY into MI
        it('should update the feature space, text: normalize: false, tokenizer: { type : "simple", stopwords: "none", stemmer: "true" }', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "text", source: "FtrSpaceTest", normalize: false, field: "Text",
                tokenizer: { type: "simple", stopwords: "none", stemmer: true }
            });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Jimmy Carr bought a car." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "My dad sold a lot of cars." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "The union of dads are buying cars." });
            Store.push({ Value: 1.0, Category: "a", Values: [1.0, 2.0], Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Car Cars Car, buy them!" });

            ftr.updateRecord(Store[11]);
            ftr.updateRecord(Store[12]);
            ftr.updateRecord(Store[13]);
            ftr.updateRecord(Store[14]);

            assert.equal(ftr.extractVector(Store[11]).length, 15);

            assert.eqtol(ftr.extractVector(Store[11]).at(0), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(1), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(2), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[11]).at(3), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[11]).at(4), Math.log(1));

            assert.eqtol(ftr.extractVector(Store[12]).at(5), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(6), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[12]).at(7), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(8), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[12]).at(9), Math.log(2));

            assert.eqtol(ftr.extractVector(Store[13]).at(10), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(11), Math.log(4));
            assert.eqtol(ftr.extractVector(Store[13]).at(12), Math.log(4));

            assert.eqtol(ftr.extractVector(Store[14]).at(13), Math.log(2));
            assert.eqtol(ftr.extractVector(Store[14]).at(14), Math.log(4));
        })
    });

    describe('UpdateRecords Tests', function () {
        it('should update the feature space by adding the whole store: constant', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest" });
            var rs = Store.allRecords;
            ftr.updateRecords(rs);
            for (var i = 0; i < 11; i++) {
                assert.equal(ftr.extractVector(Store[i]).length, 1);
                assert.equal(ftr.extractVector(Store[i]).at(0), 1);
            }
        })
        it('should update the feature space by adding the whole store: constant, number 10', function () {
            var ftr = new qm.FeatureSpace(base, { type: "constant", source: "FtrSpaceTest", const: 10 });
            var rs = Store.allRecords;
            ftr.updateRecords(rs);
            for (var i = 0; i < 10; i++) {
                assert.equal(ftr.extractVector(Store[i]).length, 1);
                assert.equal(ftr.extractVector(Store[i]).at(0), 10);
            }
        })
        it('should update the feature space by adding the whole store: random', function () {
            var ftr = new qm.FeatureSpace(base, { type: "random", source: "FtrSpaceTest" });
            var rs = Store.allRecords;
            ftr.updateRecords(rs);
            for (var i = 0; i < 10; i++) {
                assert.equal(ftr.extractVector(Store[i]).length, 1);
                assert.ok(0 <= ftr.extractVector(Store[i]).at(0) <= 1);
            }
        })
        it('should update the feature space by adding the whole store: numeric', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            for (var i = 0; i < 11; i++) {
                assert.eqtol(ftr.extractVector(Store[i]).at(0), 1 + i / 10);
            };
        })
        it('should update the feature space by adding the whole store: numeric, normalize', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            for (var i = 0; i < 11; i++) {
                assert.eqtol(ftr.extractVector(Store[i]).at(0), i / 10);
            };
        })
        it('should update the feature spave by adding the whole store: numeric, normalize, min, max', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value", min: 1.5, max: 2.5 });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            for (var i = 0; i < 6; i++) {
                assert.eqtol(ftr.extractVector(Store[i]).at(0), 0);
            };
            for (var i = 6; i < 11; i++) {
                assert.eqtol(ftr.extractVector(Store[i]).at(0), (i-5) / 10);
            }
        })
        it('should update the feature space by adding the whole store: categorical', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            assert.equal(ftr.extractVector(Store[0]).length, 3);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[0]).at(1), 0);
            assert.equal(ftr.extractVector(Store[0]).at(2), 0);

        })
        it('should update the feature space by adding the whole store: categorical, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base, { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 4 });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            assert.equal(ftr.extractVector(Store[0]).length, 4);
        })
        it('should update the feature space by adding the whole store: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            var rs = Store.allRecords;

            ftr.updateRecords(rs);
            assert.equal(ftr.extractVector(Store[0]).length, 6);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[0]).at(1), 0);
            assert.equal(ftr.extractVector(Store[0]).at(2), 0);
            assert.equal(ftr.extractVector(Store[0]).at(3), 1);
            assert.equal(ftr.extractVector(Store[0]).at(4), 0);
            assert.equal(ftr.extractVector(Store[0]).at(5), 0);
        });
        it('should update the feature space by adding the whole store as a JSON array: multinomial', function () {
            var ftr = new qm.FeatureSpace(base, { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] });
            var rs = Store.allRecords.toJSON().records;

            ftr.updateRecords(rs);
            assert.equal(ftr.extractVector(Store[0]).length, 6);
            assert.equal(ftr.extractVector(Store[0]).at(0), 1);
            assert.equal(ftr.extractVector(Store[0]).at(1), 0);
            assert.equal(ftr.extractVector(Store[0]).at(2), 0);
            assert.equal(ftr.extractVector(Store[0]).at(3), 1);
            assert.equal(ftr.extractVector(Store[0]).at(4), 0);
            assert.equal(ftr.extractVector(Store[0]).at(5), 0);
        });

        it('should update the feature space by adding the whole store: multinomial, normalize', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", normalize: true, values: ["a", "b", "c", "q", "w", "e"] }
            );
            var rs = Store.allRecords;
            ftr.updateRecords(rs);

            assert.eqtol(ftr.extractVector(Store[0]).at(0), (1 / Math.sqrt(2)));
            assert.eqtol(ftr.extractVector(Store[0]).at(3), (1 / Math.sqrt(2)));

            assert.eqtol(ftr.extractVector(Store[10]).at(1), (1 / Math.sqrt(2)));
            assert.eqtol(ftr.extractVector(Store[10]).at(4), (1 / Math.sqrt(2)));
        })
        it('should update the feature space by adding the whole store: multinomial, hashDimension', function () {
            var ftr = new qm.FeatureSpace(base,
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
                );
            var rs = Store.allRecords;
            ftr.updateRecords(rs);

            assert.equal(ftr.extractVector(Store[0]).length, 4);
        })
        it('should update the feature space by adding the whole store: jsfunc', function () {
            var ftr = new qm.FeatureSpace(base, {
                type: "jsfunc", source: "FtrSpaceTest", name: "TestFunc", dim: 1,
                fun: function (rec) { return rec.Categories.length; }
            });
            var rs = Store.allRecords;
            ftr.updateRecords(rs);
            for (var i = 0; i < 11; i++) {
                assert.equal(ftr.extractVector(Store[i]).length, 1);
                assert.eqtol(ftr.extractVector(Store[i]).at(0), 2);
            }
        })
        it('should update the feature space by adding a whole record space, numeric, multinomial', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" },
                { type: "multinomial", source: "FtrSpaceTest", normalize: true, field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
            ]);
            var rs = Store.allRecords;
            ftr.updateRecords(rs);

            assert.eqtol(ftr.extractVector(Store[0]).at(0), 0);
            assert.eqtol(ftr.extractVector(Store[0]).at(1), (1 / Math.sqrt(2)));
            assert.eqtol(ftr.extractVector(Store[0]).at(4), (1 / Math.sqrt(2)));

            assert.eqtol(ftr.extractVector(Store[10]).at(0), 1);
            assert.eqtol(ftr.extractVector(Store[10]).at(2), (1 / Math.sqrt(2)));
            assert.eqtol(ftr.extractVector(Store[10]).at(5), (1 / Math.sqrt(2)));
        })
    });

    describe('extractSparseMatrix Tests', function () {
        it('should return a sparse matrix gained from the numeric feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            var rs = Store.allRecords;
            var mat = ftr.extractSparseMatrix(rs);

            //assert.equal(mat.rows, 1);
            assert.equal(mat.cols, 11);
            assert.eqtol(mat.at(0, 0), 1);
            assert.eqtol(mat.at(0, 5), 1.5);
            assert.eqtol(mat.at(0, 10), 2);

            // test that we get the same for a JSON array input
            var rsJson = Store.allRecords.toJSON().records;
            var mat2 = ftr.extractSparseMatrix(rsJson);
            assert.eqtol(mat.minus(mat2).frob(), 0);

        })
        it('should return a bigger space matrix gained from the numeric and categorical feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var rs = Store.allRecords;
            var mat = ftr.extractSparseMatrix(rs);

            //assert.equal(mat.rows, 4);
            assert.equal(mat.cols, 11);

            assert.eqtol(mat.at(0, 0), 1);
            assert.eqtol(mat.at(1, 0), 1);
            assert.eqtol(mat.at(3, 0), 0);

            assert.eqtol(mat.at(0, 5), 1.5);
            assert.eqtol(mat.at(3, 5), 1);

            assert.eqtol(mat.at(0, 10), 2);
            assert.eqtol(mat.at(2, 10), 1);
        })
    });

    describe('FtrColMat Tests', function () {
        it('should return a dense matrix gained from the numeric feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            var rs = Store.allRecords;
            var mat = ftr.extractMatrix(rs);

            assert.equal(mat.rows, 1);
            assert.equal(mat.cols, 11);
            assert.eqtol(mat.at(0, 0), 1);
            assert.eqtol(mat.at(0, 5), 1.5);
            assert.eqtol(mat.at(0, 10), 2);

            // test that we get the same for a JSON array input
            var rsJson = Store.allRecords.toJSON().records;
            var mat2 = ftr.extractMatrix(rsJson);
            assert.eqtol(mat.minus(mat2).frob(), 0);

        })
        it('should return a dense matrix gained from the numeric and categorical feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var rs = Store.allRecords;
            var mat = ftr.extractMatrix(rs);

            assert.equal(mat.rows, 4);
            assert.equal(mat.cols, 11);

            assert.eqtol(mat.at(0, 0), 1);
            assert.eqtol(mat.at(1, 0), 1);
            assert.eqtol(mat.at(3, 0), 0);

            assert.eqtol(mat.at(0, 5), 1.5);
            assert.eqtol(mat.at(3, 5), 1);

            assert.eqtol(mat.at(0, 10), 2);
            assert.eqtol(mat.at(2, 10), 1);
        })
    });
})
