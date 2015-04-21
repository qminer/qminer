console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js'); //adds assert.run function
var qm = require('qminer');

describe('Feature Space Tests', function () {
    var base = undefined;
    var Store = undefined;

    before(function () {
        qm.delLock();
        qm.config('qm.conf', true, 8080, 1024);
        var backward = require('../../src/nodejs/scripts/backward.js');
        backward.addToProcess(process); // adds process.isArg function

        base = qm.create('qm.conf', "", true); // 2nd arg: empty schema, 3rd arg: clear db folder = true
        // prepare test set
        base.createStore({
            "name": "FtrSpaceTest",
            "fields": [
              { "name": "Value", "type": "float" },
              { "name": "Category", "type": "string" },
              { "name": "Categories", "type": "string_v" },
              { "name": "Date", "type": "datetime" },
              { "name": "Text", "type": "string" }
            ],
            "joins": [],
            "keys": []
        });
        Store = base.store("FtrSpaceTest");
        Store.add({ Value: 1.0, Category: "a", Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Barclays dropped a bombshell on its investment bankers last week." });
        Store.add({ Value: 1.1, Category: "b", Categories: ["b", "w"], Date: "2014-10-11T00:11:22", Text: "Amid a general retreat by banks from bond trading and other volatile business lines, Barclays was particularly aggressive." });
        Store.add({ Value: 1.2, Category: "c", Categories: ["c", "e"], Date: "2014-10-12T00:11:22", Text: "In what CEO Antony Jenkins dubbed a “bold simplification,” Barclays will cut 7,000 jobs in its investment bank by 2016 and will trim the unit to 30% of the group’s risk-weighted assets." });
        Store.add({ Value: 1.3, Category: "a", Categories: ["a", "q"], Date: "2014-10-13T00:11:22", Text: "The bank is relegating £400 billion ($676 billion) in assets to its “non-core” unit, effectively quarantining them from the rest of the business." });
        Store.add({ Value: 1.4, Category: "b", Categories: ["b", "w"], Date: "2014-10-14T00:11:22", Text: "Just about every large lender these days has chucked a chunk of its toxic cast-offs into a so-called “bad bank,” but none with the same zeal as Barclays." });
        Store.add({ Value: 1.5, Category: "c", Categories: ["c", "e"], Date: "2014-10-15T00:11:22", Text: "The last time we drew up the league table for bad banks, UBS was on top." });
        Store.add({ Value: 1.6, Category: "a", Categories: ["a", "q"], Date: "2014-10-16T00:11:22", Text: "But Barclays has now taken the crown, with “non-core” assets accounting for nearly 30% of the bank’s total balance sheet." });
        Store.add({ Value: 1.7, Category: "b", Categories: ["b", "w"], Date: "2014-10-17T00:11:22", Text: "Details about a bad bank are typically relegated deep in the depths of a bank’s financial report, while it prominently trumpets the brilliant “adjusted” results of its unsullied core business." });
        Store.add({ Value: 1.8, Category: "c", Categories: ["c", "e"], Date: "2014-10-18T00:11:22", Text: "These assets still belong to the bank, and factor into its capital ratios and other important measures of financial soundness." });
        Store.add({ Value: 1.9, Category: "a", Categories: ["a", "q"], Date: "2014-10-19T00:11:22", Text: "But because selling everything at once would produce a huge loss, carving out an internal bad bank is the next best option." });
        Store.add({ Value: 2.0, Category: "b", Categories: ["b", "w"], Date: "2014-10-20T00:11:22", Text: "The Barbie doll is an icon that young girls have played with since 1959, when Barbie settled in as an American fixture in the lives of children, first in the United States and in more recent years, worldwide." });

    });
    after(function () {
        base.close();
    });

    describe('Constructor Tests', function () {
        it('should construct a new feature space using a base and feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.notEqual(ftr, null);
        })
        it.skip('should throw an exception, if feature extractor is not given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace(base);
            })
            console.log("should be seen on screen: base");
        })
        it.skip('should throw an exception, if only the feature extractor is given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace({ type: "numeric", source: "FtrSpaceTest", field: "Value" });
            })
        })
        it.skip('should throw an exception, if no parameters are given', function () {
            assert.throws(function () {
                var ftr = new qm.FeatureSpace();
                console.log("should be seen on screen: none");
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
            ftr.add({ type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] });
            assert.equal(ftr.dim, 4);
        })
        it('should throw an exception, if the added feature extractor is not valid', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.add({ type: "categorical", source: "FtrSpaceTest" });
            })
        })
        it('should throw an exception, if there is no parameter given', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                ftr.add();
            })
        })
    });

    describe('FtrVec Test', function () {
        it('should return a vector for the first record in store: one extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.ftrVec(Store[0]);
            assert.equal(vec.length, 1);
            assert.equal(vec[0], 1.0);
        })
        it('should return a vector for the first record in store: two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var vec = ftr.ftrVec(Store[0]);
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
            var vec = ftr.ftrVec(Store[10]);
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
    });

    describe('FtrSpVec Tests', function () {
        it('should return a sparse vector for the first record in store: one extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.ftrSpVec(Store[0]);
            assert.equal(vec.dim, 1);
            assert.equal(vec.at(0), 1.0);
        })
        it('should return a sparse vector for the first record in store: two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var vec = ftr.ftrSpVec(Store[0]);
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
                { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);
            var vec = ftr.ftrSpVec(Store[10]);
            assert.equal(vec.dim, 16);
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
        })
    });

    describe('InvFtrVec Tests', function () {
        it('should return the values of the first record (using the inverse function of ftrVec): one extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var vec = ftr.invFtrVec([1]);

            assert.equal(vec.length, 1);
            assert.equal(vec.at(0), 1.0);
        })
        it.skip('should return the values of the first record: one extractor (parameter is a vector)', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var ftrvec = ftr.ftrVec(Store[0]);

            var vec = ftr.invFtrVec(ftrvec);
            assert.equal(vec.length, 1);
            assert.equal(vec.at(0), 1.0);
        })
        it.skip('should return the values of the first record: two extractors', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var vec = ftr.invFtrVec([1, 1, 0, 0]);
            assert.equal(vec.length, 2);
            assert.equal(vec.at(0), 1.0);
            assert.equal(vec.at(1), "a");
        })
    });

    describe('InvFtr Tests', function () {
        it('should inverse the value using the first feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var val = ftr.invFtr(0, 1);

            assert.equal(val, 1.0);
        })
        it.skip('should inverse the value using the second feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
                { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                { type: "categorical", source: "FtrSpaceTest", field: "Category", value: ["a", "b", "c"] }
            ]);
            var val = ftr.invFtr(1, [0, 1, 0]);

            assert.equal(val, "b");
        })
    })

    describe('GetFtrExtractor Tests', function () {
        it('should return the name of the first feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var name = ftr.getFtrExtractor(0);
            assert.equal(name, "Numeric[Value]");
        })
        it('should return the name of the second feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFtrExtractor(1);
            assert.equal(name, "Categorical[Category]");
        })
        it('should throw an exception, if index is out of bound, idx = 2', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFtrExtractor(2);
            })
        })
        it('should throw an exception, if index is out of bound, idx = -1', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFtrExtractor(-1);
            })
        })
        it('should throw an exception, if the parameter is an feature extractor', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            assert.throws(function () {
                var name = ftr.getFtrExtractor({ type: "numeric", source: "FtrSpaceTest", field: "Value" });
            })
        })
    });

    describe('GetFtr Tests', function () {
        it('should return the name of the feature at first position', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", field: "Value" });
            var name = ftr.getFtr(0);
            assert.equal(name, "Numeric[Value]");
        })
        it('should return the name of the feature at second position', function () {
            var ftr = new qm.FeatureSpace(base, [
              { type: "numeric", source: "FtrSpaceTest", field: "Value" },
              { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFtr(1);
            assert.equal(name, "a");
        })
        it('should return the name of the feature at third position', function () {
            var ftr = new qm.FeatureSpace(base, [
             { type: "numeric", source: "FtrSpaceTest", field: "Value" },
             { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFtr(2);
            assert.equal(name, "b");
        })
        it('should throw an exception, if the index is out of bound, idx = 4', function () {
            var ftr = new qm.FeatureSpace(base, [
             { type: "numeric", source: "FtrSpaceTest", field: "Value" },
             { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            assert.throws(function () {
                var name = ftr.getFtr(4);
            })
        })
        it('should return the name of the first feature, if index < 0', function () {
            var ftr = new qm.FeatureSpace(base, [
                        { type: "numeric", source: "FtrSpaceTest", field: "Value" },
                        { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] }
            ]);
            var name = ftr.getFtr(-1);
            assert.equal(name, "Numeric[Value]");
        })
    });

    describe.skip('Filter Tests', function () {
        it('should return only the features of a vector for a given feature extractor id = 0', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.ftrVec(Store[0]);
            var out_vec = ftr.filter(in_vec, 0);

            in_vec.print();
            out_vec.print();

            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
        })
        it('should return only the features of a vector for the given feature extractor id = 1', function () {
            var ftr = new qm.FeatureSpace(base, [
               { type: "numeric", source: "FtrSpaceTest", field: "Value" },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", values: ["a", "b", "c"] },
               { type: "categorical", source: "FtrSpaceTest", field: "Category", hashDimension: 2 },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", values: ["a", "b", "c", "q", "w", "e"] },
               { type: "multinomial", source: "FtrSpaceTest", field: "Categories", hashDimension: 4 }
            ]);

            var in_vec = ftr.ftrVec(Store[0]);
            var out_vec = ftr.filter(in_vec, 4);

            in_vec.print();
            out_vec.print();

            assert.equal(out_vec.at(0), 1);
            assert.equal(out_vec.at(1), 0);
            assert.equal(out_vec.at(2), 0);
        })
    });

    describe.only('UpdateRecord Tests', function () {
        it('should update the feature space with a new record', function () {
            var ftr = new qm.FeatureSpace(base, { type: "numeric", source: "FtrSpaceTest", normalize: true, field: "Value" });
            assert.equal(ftr.ftrVec(Store[0]).at(0), 0);
            
            ftr.updateRecord({ Value: 1.1, Category: "a", Categories: ["a", "q"], Date: "2014-10-10T00:11:22", Text: "Barclays dropped a bombshell." });
            assert.equal(ftr.ftrVec(Store[0]).at(0), 0);

        })
    })
})