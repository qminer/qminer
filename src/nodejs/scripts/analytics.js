// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    exports = require('bindings')(pathPrefix + '/analytics.node');


    var la = require(__dirname + '/la.js')(pathPrefix);
    var qm = require('bindings')(pathPrefix + '/qm.node');
    var fs = require('bindings')(pathPrefix + '/fs.node');
    var qm_util = require(__dirname + '/qm_util.js');


    function createBatchModel(featureSpace, models) {
        this.featureSpace = featureSpace;
        this.models = models;
        // get targets
        this.target = [];
        for (var cat in this.models) { this.target.push(cat); }
        // serialize to stream
        this.save = function (sout) {
            // save list
            debugger
            sout.writeLine(this.models);
            // save feature space
            this.featureSpace.save(sout);
            // save models
            for (var cat in this.models) {
                this.models[cat].model.save(sout);
            }
            return sout;
        }

        this.predict = function (record) {
            var vec = this.featureSpace.ftrSpVec(record);
            var result = {};
            for (var cat in this.models) {
                result[cat] = this.models[cat].model.predict(vec);
            }
            return result;
        }

        this.predictLabels = function (record) {
            var result = this.predict(record);
            var labels = [];
            for (var cat in result) {
                if (result[cat] > 0.0) {
                    labels.push(cat);
                }
            }
            return labels;
        }

        this.predictTop = function (record) {
            var result = this.predict(record);
            var top = null;
            for (var cat in result) {
                if (top) {
                    if (top.weight > result[cat]) {
                        top.category = cat;
                        top.weight = result[cat];
                    }
                } else {
                    top = { category: cat, weight: result[cat] }
                }
            }
            return top.category;
        }
        return this;
    }

    //#- `batchModel = analytics.newBatchModel(rs, features, target)` -- learns a new batch model
    //#     using record set `rs` as training data and `features`; `target` is
    //#     a field descriptor JSON object for the records which we are trying to predict (obtained by calling store.field("Rating");
    //#     if target field string or string vector, the result is a SVM classification model,
    //#     and if target field is a float, the result is a SVM regression model; resulting 
    //#     model has the following functions:
    //#   - `strArr = batchModel.target` -- array of categories for which we have models
    //#   - `scoreArr = batchModel.predict(rec)` -- creates feature vector from record `rec`, sends it
    //#     through the model and returns the result as a dictionary where labels are keys and scores (numbers) are values.
    //#   - `labelArr = batchModel.predictLabels(rec)` -- creates feature vector from record `rec`, 
    //#     sends it through the model and returns the labels with positive weights as `labelArr`.
    //#   - `labelStr = batchModel.predictTop(rec)` -- creates feature vector from record `rec`, 
    //#     sends it through the model and returns the top ranked label `labelStr`.
    //#   - `batchModel.save(fout)` -- saves the model to `fout` output stream
    exports.newBatchModel = function (records, features, target, limitCategories) {
        console.log("newBatchModel", "Start");
        // prepare feature space
        console.log("newBatchModel", "  creating feature space");
        var featureSpace = new qm.FeatureSpace(records.store.base, features);
        // initialize features
        featureSpace.updateRecords(records);
        console.log("newBatchModel", "  number of dimensions = " + featureSpace.dim);
        // prepare spare vectors
        console.log("newBatchModel", "  preparing feature vectors");
        var sparseVecs = featureSpace.ftrSpColMat(records);
        // prepare target vectors
        var targets = {};
        // figure out if new category name, or update count
        function initCats(categories, catName) {
            if (categories[catName]) {
                categories[catName].count++;
            } else {
                // check if we should ignore this category
                if (limitCategories && !qm_util.isInArray(limitCategories, catName)) { return; }
                // check if we should ignore this category
                categories[catName] = {
                    name: catName,
                    type: "classification",
                    count: 1,
                    target: new la.Vector({ mxVals: records.length })
                };
            }
        }
        // initialize targets
        console.log("newBatchModel", "  preparing target vectors");
        if (target.type === "string_v") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var cats = records[i][target.name];
                for (var j = 0; j < cats.length; j++) {
                    initCats(targets, cats[j]);
                }
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var cats = la.copyVecToArray(records[i][target.name]);
                for (var cat in targets) {
                    targets[cat].target.push(qm_util.isInArray(cats, cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "string") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                initCats(targets, recCat);
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                for (var cat in targets) {
                    targets[cat].target.push((recCat === cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "float") {
            // initialized with +1 or -1 for each category
            targets[target.name] = {
                name: target.name,
                type: "regression",
                count: records.length,
                target: new la.Vector({ mxVals: records.length })

            };
            for (var i = 0; i < records.length; i++) {
                targets[target.name].target.push(records[i][target.name]);
            }
        }
        // training model for each category
        console.log("newBatchModel", "  training SVM");
        var models = {};
        for (var cat in targets) {
            if (targets[cat].count >= 50) {
                models[cat] = {
                    name: targets[cat].name,
                    type: targets[cat].type,
                };
                if (targets[cat].type === "classification") {
                    console.log("newBatchModel", "    ... " + cat + " (classification)");
                    models[cat].model = new exports.SVC({ c: 1, j: 10, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                } else if (targets[cat].type === "regression") {
                    console.log("newBatchModel", "    ... " + cat + " (regression)");
                    models[cat].model = new exports.SVR({ c: 1, eps: 1e-2, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                }
            }
        }
        // done
        console.log("newBatchModel", "Done");
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };

    //#- `batchModel = analytics.loadBatchModel(base, fin)` -- loads batch model frm input stream `fin`
    exports.loadBatchModel = function (base, sin) {
        var models = JSON.parse(sin.readLine());
        var featureSpace = new qm.FeatureSpace(base, sin);
        for (var cat in models) {
            models[cat].model = new exports.SVC(sin);
        }
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };


    return exports;
}