/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    var sget = require('sget');
    var qm = require('bindings')(pathPrefix + '/qm.node');
    var fs = qm.fs;
    var la = qm.la;
    var assert = require('assert');

    exports = qm.analytics;

    var la = require(__dirname + '/la.js')(pathPrefix);
    var qm_util = require(__dirname + '/qm_util.js');


    exports.preprocessing = new function() {
        this.binerize = function (y, cat) {
            var target = new la.Vector();
            for (var i = 0; i < y.length; i++) {
                target.push(y[i] == cat ? 1 : -1);
            }
            return target;
        };

        this.applyModel = function (model, X) {
            var target = new la.Vector();
            for (var i = 0; i < X.cols; i++) {
                target.push(model.decision_function(X[i]));
            }
            return target;
        }
    };

    exports.OneVsAll = function (model, modelParam, cats) {
        // remember parameters
        this.model = model;
        this.modelParam = modelParam;
        this.cats = cats;
        // trained models
        this.models = [ ];

        // apply all models to the given vector and return distance to the class boundary
        this.decision_function = function(x) {
            // evaluate all models
            var scores = [ ];
            for (var cat = 0; cat < this.cats; cat++) {
                scores.push(this.models[cat].decision_function(x));
            }
            return scores;
        }

        // return the most likely category
        this.predict = function(x) {
            // evaluate all models
            var scores = this.decision_function(x)
            // select maximal one
            var maxScore = scores[0], maxCat = 0;
            for (var cat = 1; cat < this.cats; cat++) {
                if (scores[cat] > maxScore) {
                    maxScore = scores[cat];
                    maxCat = cat;
                }
            }
            // done!
            return maxCat;
        }

        // X = feature matrix
        // y = target label from 0..cats
        this.fit = function(X, y) {
            this.models = [ ];
            // make model for each category
            for (var cat = 0; cat < this.cats; cat++) {
                console.log("Fitting label", (cat + 1), "/", this.cats);
                // prepare targert vector for current category
                var target = exports.preprocessing.binerize(y, cat);
                // get the model
                var catModel = new this.model(this.modelParam);
                this.models.push(catModel.fit(X, target));
            }
            console.log("Done!");
            return this;
        }
    };

    exports.metrics = new function() {
        // For evaluating provided categories (precision, recall, F1).
        this.ClassifcationScore = function (yTrue, yPred) {
            this.scores = {
                count: 0, predictionCount: 0,
                TP: 0, TN: 0, FP: 0, FN: 0,
                all: function () { return this.TP + this.FP + this.TN + this.FN; },
                precision: function () { return (this.FP == 0) ? 1 : this.TP / (this.TP + this.FP); },
                recall: function () { return this.TP / (this.TP + this.FN); },
                f1: function () { return 2 * this.precision() * this.recall() / (this.precision() + this.recall()); },
                accuracy: function () { return (this.TP + this.TN) / this.all(); }
            };

            // adds prediction to the current statistics. `correct` corresponds to the correct
            // label(s), `predicted` correspond to predicted lable(s). Labels can be either integers
            // or integer array (when there are zero or more then one lables).
            this.push = function (correct, predicted) {
                var catCorrect = (correct > 0);
                var catPredicted = (predicted > 0);
                // update counts for correct categories
                if (catCorrect) { this.scores.count++; }
                // update counts for how many times category was predicted
                if (catPredicted) { this.scores.predictionCount++; }
                // update true/false positive/negative count
                if (catCorrect && catPredicted) {
                    // both predicted and correct say true
                    this.scores.TP++;
                } else if (catCorrect) {
                    // this was only correct but not predicted
                    this.scores.FN++;
                } else if (catPredicted) {
                    // this was only predicted but not correct
                    this.scores.FP++;
                } else {
                    // both predicted and correct say false
                    this.scores.TN++;
                }
            };

            // initialize if we are passed the data
            if (arguments.length >= 2) {
                for (var i = 0; i < yTrue.length; i++) {
                    this.push(yTrue[i], yPred[i]);
                }
            }
        };

        this.accuracyScore = function (yTrue, yPred) {
            return new this.ClassifcationScore(yTrue, yPred).scores.accuracy();
        };

        this.precisionScore = function (yTrue, yPred) {
            return new this.ClassifcationScore(yTrue, yPred).scores.precision();
        };

        this.recallScore = function (yTrue, yPred) {
            return new this.ClassifcationScore(yTrue, yPred).scores.recall();
        };

        this.f1Score = function (yTrue, yPred) {
            return new this.ClassifcationScore(yTrue, yPred).scores.accuracy();
        };

        // used for computing ROC curve and other related measures such as AUC;
        this.PredictionCurve = function (yTrue, yPred) {
            // count of all the positive and negative examples
    		this.allPositives = 0;
    		this.allNegatives = 0;
    		// store of predictions and ground truths
    		this.grounds = new la.Vector();
    		this.predictions = new la.Vector();

            // add new measurement with ground score (1 or -1) and predicted value
            this.push = function (ground, predict) {
                // remember the scores
                this.grounds.push(ground)
                this.predictions.push(predict);
                // update counts
                if (ground > 0) {
                    this.allPositives++;
                } else {
                    this.allNegatives++;
                }
            };

            // initialize if we are given data
            if (arguments.length >= 2) {
                for (var i = 0; i < yTrue.length; i++) {
                    this.push(yTrue[i], yPred[i]);
                }
            }

            // get ROC parametrization sampled on `sample' points
    		this.roc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    			// sort according to predictions
    			var perm = this.predictions.sortPerm(false);
    			// maintaining the results as we go along
    			var TP = 0, FP = 0, ROC = [[0, 0]];
    			// for figuring out when to dump a new ROC sample
    			var next = Math.floor(perm.perm.length / sample);
    			// go over the sorted results
    			for (var i = 0; i < perm.perm.length; i++) {
    				// get the ground
    				var ground = this.grounds[perm.perm[i]];
    				// update TP/FP counts according to the ground
    				if (ground > 0) { TP++ } else { FP++; }
    				// see if time to do next save
    				next = next - 1;
    				if (next <= 0) {
    					// add new datapoint to the curve
    					ROC.push([FP/this.allNegatives, TP/this.allPositives]);
    					// setup next timer
    					next = Math.floor(perm.perm.length / sample);
    				}
    			}
    			// add the last point
    			ROC.push([1,1]);
    			// return ROC
    			return ROC;
    		}

            // get AUC of the current curve
    		this.auc = function (sample) {
    			// default sample size is 10
    			sample = sample || 10;
    	        // get the curve
    	        var curve = this.curve(sample);
    	        // compute the area
    	        var result = 0;
    	        for (var i = 1; i < curve.length; i++) {
    	            // get edge points
    	            var left = curve[i-1];
    	            var right = curve[i];
    	            // first the rectangle bellow
    	            result = result + (right[0] - left[0]) * left[1];
    	            // an then the triangle above
    	            result = result + (right[0] - left[0]) * (right[1] - left[1]) / 2;
    	        }
    	        return result;
    	    }

            // get precision recall curve sampled on `sample' points
            this.precisionRecallCurve = function (sample) {
                // default sample size is 10
                sample = sample || 10;
                // sort according to predictions
                var perm = this.predictions.sortPerm(false);
                // maintaining the results as we go along
                var TP = 0, FP = 0, TN = this.allNegatives, FN = this.allPositives
                var curve = [[0, 1]];
                // for figuring out when to dump a new ROC sample
                var next = Math.floor(perm.perm.length / sample);
                // go over the sorted results
                for (var i = 0; i < perm.perm.length; i++) {
                    // get the ground
                    var ground = this.grounds[perm.perm[i]];
                    // update TP/FP counts according to the ground
                    if (ground > 0) { TP++; FN--; } else { FP++; TN--; }
                    // see if time to do next save
                    next = next - 1;
                    if (next <= 0) {
                        // do the update
                        if ((TP + FP) > 0 && (TP + FN) > 0) {
                            // compute current precision and recall
                            var recall = TP / (TP + FN);
                            var precision = TP / (TP + FP);
                            // add to the curve
                            curve.push([recall, precision]);
                        }
                        // setup next timer
                        next = Math.floor(perm.perm.length / sample);
                    }
                }
                // add the last point
                var recall = TP / (TP + FN);
                var precision = TP / (TP + FP);
                curve.push([recall, precision]);
                // return ROC
                return curve;
            };

            // get break-even point, which is number where precision and recall intersect
            this.breakEvenPoint = function () {
                // sort according to predictions
                var perm = this.predictions.sortPerm(false);
                // maintaining the results as we go along
                var TP = 0, FP = 0, TN = this.allNegatives, FN = this.allPositives;
                var minDiff = 1.0, bep = -1.0;
                // go over the sorted results
                for (var i = 0; i < perm.perm.length; i++) {
                    // get the ground
                    var ground = this.grounds[perm.perm[i]];
                    // update TP/FP counts according to the ground
                    if (ground > 0) { TP++; FN--; } else { FP++; TN--; }
                    // do the update
                    if ((TP + FP) > 0 && (TP + FN) > 0 && TP > 0) {
                        // compute current precision and recall
                        var precision = TP / (TP + FP);
                        var recall = TP / (TP + FN);
                        // see if we need to update current bep
                        var diff = Math.abs(precision - recall);
                        if (diff < minDiff) { minDiff = diff; bep = (precision + recall) / 2; }
                    }
                }
                return bep;
            }
        };

        this.rocCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.rocAucScore = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).roc(sample);
        };

        this.precisionRecallCurve = function (yTrue, yPred, sample) {
            return new this.PredictionCurve(yTrue, yPred).precisionRecallCurve(sample);
        };

        this.breakEventPointScore = function (yTrue, yPred) {
            return new this.PredictionCurve(yTrue, yPred).breakEvenPoint();
        };
    };

    //// deprecated//////////////////////////////////////////////////
    function defarg(arg, defaultval) {
        return arg == undefined ? defaultval : arg;
    }

    function createBatchModel(featureSpace, models) {
        this.featureSpace = featureSpace;
        this.models = models;
        // get targets
        this.target = [];
        for (var cat in this.models) { this.target.push(cat); }
        // serialize to stream
        this.save = function (sout) {
            // save list
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
            var vec = this.featureSpace.extractSparseVector(record);
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

    //!- `batchModel = analytics.newBatchModel(rs, features, target)` -- learns a new batch model
    //!     using record set `rs` as training data and `features`; `target` is
    //!     a field descriptor JSON object for the records which we are trying to predict
	//!     (obtained by calling store.field("Rating");
    //!     if target field string or string vector, the result is a SVM classification model,
    //!     and if target field is a float, the result is a SVM regression model; resulting
    //!     model has the following functions:
    //!   - `strArr = batchModel.target` -- array of categories for which we have models
    //!   - `scoreArr = batchModel.predict(rec)` -- creates feature vector from record `rec`, sends it
    //!     through the model and returns the result as a dictionary where labels are keys and scores (numbers) are values.
    //!   - `labelArr = batchModel.predictLabels(rec)` -- creates feature vector from record `rec`,
    //!     sends it through the model and returns the labels with positive weights as `labelArr`.
    //!   - `labelStr = batchModel.predictTop(rec)` -- creates feature vector from record `rec`,
    //!     sends it through the model and returns the top ranked label `labelStr`.
    //!   - `batchModel.save(fout)` -- saves the model to `fout` output stream
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
        var sparseVecs = featureSpace.extractSparseMatrix(records);
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

    //!- `batchModel = analytics.loadBatchModel(base, fin)` -- loads batch model frm input stream `fin`
    exports.loadBatchModel = function (base, sin) {
        var models = JSON.parse(sin.readLine());
        var featureSpace = new qm.FeatureSpace(base, sin);
        for (var cat in models) {
            models[cat].model = new exports.SVC(sin);
        }
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };

	//!- `result = analytics.crossValidation(rs, features, target, folds)` -- creates a batch
    //!     model for records from record set `rs` using `features; `target` is the
    //!     target field and is assumed discrete; the result is a results object
    //!     with the following API:
    //!     - `result.target` -- an object with categories as keys and the following
    //!       counts as members of these keys: `count`, `TP`, `TN`, `FP`, `FN`,
    //!       `all()`, `precision()`, `recall()`, `accuracy()`.
    //!     - `result.confusion` -- confusion matrix between categories
    //!     - `result.report()` -- prints basic report on to the console
    //!     - `result.reportCSV(fout)` -- prints CSV output to the `fout` output stream
    exports.crossValidation = function (records, features, target, folds, limitCategories) {
        // create empty folds
        var fold = [];
        for (var i = 0; i < folds; i++) {
            fold.push(new la.IntVector());
        }
        // split records into folds
        records.shuffle(1);
        var fold_i = 0;
        for (var i = 0; i < records.length; i++) {
            fold[fold_i].push(records[i].$id);
            fold_i++; if (fold_i >= folds) { fold_i = 0; }
        }
        // do cross validation
        var cfyRes = null;
        for (var fold_i = 0; fold_i < folds; fold_i++) {
            // prepare train and test record sets
            var train = new la.IntVector();
            var test = new la.IntVector();
            for (var i = 0; i < folds; i++) {
                if (i == fold_i) {
                    test.pushV(fold[i]);
                } else {
                    train.pushV(fold[i]);
                }
            }
            var trainRecs = records.store.newRecSet(train);
            var testRecs = records.store.newRecSet(test);
            console.log("Fold " + fold_i + ": " + trainRecs.length + " training and " + testRecs.length + " testing");
            // create model for the fold
            var model = exports.newBatchModel(trainRecs, features, target, limitCategories);
            // prepare test counts for each target
            if (!cfyRes) { cfyRes = new exports.classifcationScore(model.target); }
            // evaluate predictions
            for (var i = 0; i < testRecs.length; i++) {
                var correct = testRecs[i][target.name];
                var predicted = model.predictLabels(testRecs[i]);
                cfyRes.count(correct, predicted);
            }
            // report
            cfyRes.report();
        }
        return cfyRes;
    };

    //!- `alModel = analytics.newActiveLearner(query, qRecSet, fRecSet, ftrSpace, settings)` -- initializes the
    //!    active learning. The algorihm is run by calling `model.startLoop()`. The algorithm has two stages: query mode, where the algorithm suggests potential
    //!    positive and negative examples based on the query text, and SVM mode, where the algorithm keeps
    //!   selecting examples that are closest to the SVM margin (every time an example is labeled, the SVM
    //!   is retrained.
    //!   The inputs are: query (text), record set `qRecSet`, record set `fRecSet`,  the feature space `ftrSpace` and a
    //!   `settings`JSON object. The settings object specifies:`textField` (string) which is the name
    //!    of the field in records that is used to create feature vectors, `nPos` (integer) and `nNeg` (integer) set the number of positive and negative
    //!    examples that have to be identified in the query mode before the program enters SVM mode.
    //!   We can set two additional parameters `querySampleSize` and `randomSampleSize` which specify the sizes of subsamples of qRecSet and fRecSet, where the rest of the data is ignored in the active learning.
    //!   Final parameters are all SVM parameters (c, j, batchSize, maxIterations, maxTime, minDiff, verbose).
    exports.newActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        return new exports.ActiveLearner(query, qRecSet, fRecSet, ftrSpace, stts);
    }

    exports.ActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        var settings = defarg(stts, {});
        settings.nPos = defarg(stts.nPos, 2);
        settings.nNeg = defarg(stts.nNeg, 2);
        settings.textField = defarg(stts.textField, "Text");
        settings.querySampleSize = defarg(stts.querySampleSize, -1);
        settings.randomSampleSize = defarg(stts.randomSampleSize, -1);
        settings.c = defarg(stts.c, 1.0);
        settings.j = defarg(stts.j, 1.0);
        settings.batchSize = defarg(stts.batchSize, 100);
        settings.maxIterations = defarg(stts.maxIterations, 100000);
        settings.maxTime = defarg(stts.maxTime, 1); // 1 second for computation by default
        settings.minDiff = defarg(stts.minDiff, 1e-6);
        settings.verbose = defarg(stts.verbose, false);

        // compute features or provide them
        settings.extractFeatures = defarg(stts.extractFeatures, true);

        if (!settings.extractFeatures) {
            if (stts.uMat == null) { throw 'settings uMat not provided, extractFeatures = false'; }
            if (stts.uRecSet == null) { throw 'settings uRecSet not provided, extractFeatures = false'; }
            if (stts.querySpVec == null) { throw 'settings querySpVec not provided, extractFeatures = false'; }
        }

        // QUERY MODE
        var queryMode = true;
        // bow similarity between query and training set

        var querySpVec;
        var uRecSet;
        var uMat;

        if (settings.extractFeatures) {
            var temp = {}; temp[settings.textField] = query;
            var queryRec = qRecSet.store.newRecord(temp); // record
            querySpVec = ftrSpace.extractSparseVector(queryRec);
            // use sampling?
            var sq = qRecSet;
            if (settings.querySampleSize >= 0 && qRecSet != undefined) {
                sq = qRecSet.sample(settings.querySampleSize);
            }
            var sf = fRecSet;
            if (settings.randomSampleSize >= 0 && fRecSet != undefined) {
                sf = fRecSet.sample(settings.randomSampleSize);
            }
            // take a union or just qset or just fset if some are undefined
            uRecSet = (sq != undefined) ? ((sf != undefined) ? sq.setunion(sf) : sq) : sf;
            if (uRecSet == undefined) { throw 'undefined record set for active learning!';}
            uMat = ftrSpace.extractSparseMatrix(uRecSet);

        } else {
            querySpVec = stts.querySpVec;
            uRecSet = stts.uRecSet;
            uMat = stts.uMat;
        }


        querySpVec.normalize();
        uMat.normalizeCols();

        var X = new la.SparseMatrix();
        var y = new la.Vector();
        var simV = uMat.multiplyT(querySpVec); //similarities (q, recSet)
        var sortedSimV = simV.sortPerm(); //ascending sort
        var simVs = sortedSimV.vec; //sorted similarities (q, recSet)
        var simVp = sortedSimV.perm; //permutation of sorted similarities (q, recSet)
        //// counters for questions in query mode
        var nPosQ = 0; //for traversing simVp from the end
        var nNegQ = 0; //for traversing simVp from the start


        // SVM MODE
        var svm;
        var posIdxV = new la.IntVector(); //indices in recordSet
        var negIdxV = new la.IntVector(); //indices in recordSet

        var posRecIdV = new la.IntVector(); //record IDs
        var negRecIdV = new la.IntVector(); //record IDs

        var classVec = new la.Vector({ "vals": uRecSet.length }); //svm scores for record set
        var resultVec = new la.Vector({ "vals": uRecSet.length }); // non-absolute svm scores for record set


        //!   - `rs = alModel.getRecSet()` -- returns the record set that is being used (result of sampling)
        this.getRecSet = function () { return uRecSet };

        //!   - `idx = alModel.selectedQuestionIdx()` -- returns the index of the last selected question in alModel.getRecSet()
        this.selectedQuestionIdx = -1;

        //!   - `bool = alModel.getQueryMode()` -- returns true if in query mode, false otherwise (SVM mode)
        this.getQueryMode = function () { return queryMode; };

        //!   - `numArr = alModel.getPos(thresh)` -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
        this.getPos = function (threshold) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!threshold) { threshold = 0; }
            var posIdxArray = [];
            for (var recN = 0; recN < uRecSet.length; recN++) {
                if (resultVec[recN] >= threshold) {
                    posIdxArray.push(recN);
                }
            }
            return posIdxArray;
        };

        this.debug = function () { debugger; }

        this.getTop = function (limit) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!limit) { limit = 20; }
            var idxArray = [];
            var marginArray = [];
            var sorted = resultVec.sortPerm(false);
            for (var recN = 0; recN < uRecSet.length && recN < limit; recN++) {
                idxArray.push(sorted.perm[recN]);
                var val = sorted.vec[recN];
                val = val == Number.POSITIVE_INFINITY ? Number.MAX_VALUE : val;
                val = val == Number.NEGATIVE_INFINITY ? -Number.MAX_VALUE : val;
                marginArray.push(val);
            }
            return { posIdx: idxArray, margins: marginArray };
        };

        //!   - `objJSON = alModel.getSettings()` -- returns the settings object
        this.getSettings = function () { return settings; }

        // returns record set index of the unlabeled record that is closest to the margin
        //!   - `recSetIdx = alModel.selectQuestion()` -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
        this.selectQuestion = function () {
            if (posRecIdV.length >= settings.nPos && negRecIdV.length >= settings.nNeg) { queryMode = false; }
            if (queryMode) {
                if (posRecIdV.length < settings.nPos && nPosQ + 1 < uRecSet.length) {
                    nPosQ = nPosQ + 1;
                    console.log("query mode, try to get pos");
                    this.selectedQuestionIdx = simVp[simVp.length - 1 - (nPosQ - 1)];
                    return this.selectedQuestionIdx;
                }
                if (negRecIdV.length < settings.nNeg && nNegQ + 1 < uRecSet.length) {
                    nNegQ = nNegQ + 1;
                    // TODO if nNegQ == rRecSet.length, find a new sample
                    console.log("query mode, try to get neg");
                    this.selectedQuestionIdx = simVp[nNegQ - 1];
                    return this.selectedQuestionIdx;
                }
            }
            else {
                ////call svm, get record closest to the margin
                svm = new exports.SVC(settings);
                svm.fit(X, y);//column examples, y float vector of +1/-1, default svm paramvals

                // mark positives
                for (var i = 0; i < posIdxV.length; i++) {
                    classVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                }
                // mark negatives
                for (var i = 0; i < negIdxV.length; i++) {
                    classVec[negIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[negIdxV[i]] = Number.NEGATIVE_INFINITY;
                }
                var posCount = posIdxV.length;
                var negCount = negIdxV.length;
                // classify unlabeled
                for (var recN = 0; recN < uRecSet.length; recN++) {
                    if (classVec[recN] !== Number.POSITIVE_INFINITY) {
                        var svmMargin = svm.predict(uMat.getCol(recN));
                        if (svmMargin > 0) {
                            posCount++;
                        } else {
                            negCount++;
                        }
                        classVec[recN] = Math.abs(svmMargin);
                        resultVec[recN] = svmMargin;
                    }
                }
                var sorted = classVec.sortPerm();
                console.log("svm mode, margin: " + sorted.vec[0] + ", npos: " + posCount + ", nneg: " + negCount);
                this.selectedQuestionIdx = sorted.perm[0];
                return this.selectedQuestionIdx;
            }

        };
        // asks the user for class label given a record set index
        //!   - `alModel.getAnswer(ALAnswer, recSetIdx)` -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
        //!      The user input should be either "y" (indicating that recSet[recSetIdx] is a positive example), "n" (negative example).
        this.getAnswer = function (ALanswer, recSetIdx) {
            //todo options: ?newQuery
            if (ALanswer === "y") {
                posIdxV.push(recSetIdx);
                posRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(1.0);
            } else {
                negIdxV.push(recSetIdx);
                negRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(-1.0);
            }
            // +k query // rank unlabeled according to query, ask for k most similar
            // -k query // rank unlabeled according to query, ask for k least similar
        };
        //!   - `alModel.startLoop()` -- starts the active learning loop in console
        this.startLoop = function () {
            while (true) {
                var recSetIdx = this.selectQuestion();
                var ALanswer = sget(uRecSet[recSetIdx].Text + ": y/(n)/s? Command s stops the process").trim();
                if (ALanswer == "s") { break; }
                if (posIdxV.length + negIdxV.length == uRecSet.length) { break; }
                this.getAnswer(ALanswer, recSetIdx);
            }
        };
        //!   - `alModel.saveSvmModel(fout)` -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
        this.saveSvmModel = function (outputStream) {
            // must be in SVM mode
            if (queryMode) {
                console.log("AL.save: Must be in svm mode");
                return;
            }
            svm.save(outputStream);
        };

        this.getWeights = function () {
            return svm.weights;
        }
        //this.saveLabeled
        //this.loadLabeled
    };


	//////////// RIDGE REGRESSION
	// solve a regularized least squares problem
	//!- `ridgeRegressionModel = new analytics.RidgeRegression(kappa, dim, buffer)` -- solves a regularized ridge
	//!  regression problem: min|X w - y|^2 + kappa |w|^2. The inputs to the algorithm are: `kappa`, the regularization parameter,
	//!  `dim` the dimension of the model and (optional) parameter `buffer` (integer) which specifies
	//!  the length of the window of tracked examples (useful in online mode). The model exposes the following functions:
	exports.RidgeRegression = function (kappa, dim, buffer) {
	    var X = [];
	    var y = [];
	    buffer = typeof buffer !== 'undefined' ? buffer : -1;
	    var w = new la.Vector({ "vals": dim });
	    //!   - `ridgeRegressionModel.add(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set
	    this.add = function (x, target) {
	        X.push(x);
	        y.push(target);
	        if (buffer > 0) {
	            if (X.length > buffer) {
	                this.forget(X.length - buffer);
	            }
	        }
	    };
	    //!   - `ridgeRegressionModel.addupdate(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set and retrains the model
	    this.addupdate = function (x, target) {
	        this.add(x, target);
	        this.update();
	    }
	    //!   - `ridgeRegressionModel.forget(n)` -- deletes first `n` (integer) examples from the training set
	    this.forget = function (ndeleted) {
	        ndeleted = typeof ndeleted !== 'undefined' ? ndeleted : 1;
	        ndeleted = Math.min(X.length, ndeleted);
	        X.splice(0, ndeleted);
	        y.splice(0, ndeleted);
	    };
	    //!   - `ridgeRegressionModel.update()` -- recomputes the model
	    this.update = function () {
	        var A = this.getMatrix();
	        var b = new la.Vector(y);
	        w = this.compute(A, b);
	    };
	    //!   - `vec = ridgeRegressionModel.getModel()` -- returns the parameter vector `vec` (dense vector)
	    this.getModel = function () {
	        return w;
	    };
	    this.getMatrix = function () {
	        if (X.length > 0) {
	            var A = new la.Matrix({ "cols": X[0].length, "rows": X.length });
	            for (var i = 0; i < X.length; i++) {
	                A.setRow(i, X[i]);
	            }
	            return A;
	        }
	    };
	    //!   - `vec2 = ridgeRegressionModel.compute(mat, vec)` -- computes the model parameters `vec2`, given
	    //!    a row training example matrix `mat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |mat' vec2 - vec|^2 + kappa |vec2|^2.
	    //!   - `vec2 = ridgeRegressionModel.compute(spMat, vec)` -- computes the model parameters `vec2`, given
	    //!    a row training example sparse matrix `spMat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |spMat' vec2 - vec|^2 + kappa |vec2|^2.
	    this.compute = function (A, b) {
	        var I = la.eye(A.cols);
	        var coefs = (A.transpose().multiply(A).plus(I.multiply(kappa))).solve(A.transpose().multiply(b));
	        return coefs;
	    };
	    //!   - `num = model.predict(vec)` -- predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.
	    this.predict = function (x) {
	        return w.inner(x);
	    };
	};


    /**
     * StreamStory.
     * @class
     * @param {opts} HierarchMarkovParam - parameters. TODO typedef and describe
     */
    exports.HierarchMarkov = function (opts) {
    	// constructor
    	if (opts == null) throw 'Missing parameters!';
    	if (opts.base == null) throw 'Missing parameter base!';

    	// create model and feature space
    	var mc;
    	var obsFtrSpace;
    	var controlFtrSpace;

    	if (opts.hmcConfig != null && opts.obsFields != null &&
    			opts.contrFields != null && opts.base != null) {

    		mc = opts.sequenceEndV != null ? new exports.HMC(opts.hmcConfig, opts.sequenceEndV) : new exports.HMC(opts.hmcConfig);

    		obsFtrSpace = new qm.FeatureSpace(opts.base, opts.obsFields);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, opts.contrFields);
    	}
    	else if (opts.hmcFile != null) {
    		var fin = new fs.FIn(opts.hmcFile);
    		mc = new exports.HMC(fin);
    		obsFtrSpace = new qm.FeatureSpace(opts.base, fin);
    		controlFtrSpace = new qm.FeatureSpace(opts.base, fin);
    	}
    	else {
    		throw 'Parameters missing: ' + JSON.stringify(opts);
    	}

    	function getFtrNames(ftrSpace) {
    		var names = [];

    		var dims = ftrSpace.dims;
    		for (var i = 0; i < dims.length; i++) {
				names.push(ftrSpace.getFeature(i));
			}

    		return names;
    	}

    	function getObsFtrCount() {
			return obsFtrSpace.dims.length;
		}

    	function getObsFtrNames() {
    		return getFtrNames(obsFtrSpace);
    	}

    	function getControlFtrNames() {
    		return getFtrNames(controlFtrSpace);
    	}

    	function getFtrDescriptions(stateId) {
    		var observations = [];
    		var controls = [];

			var coords = mc.fullCoords(stateId);
			var obsFtrNames = getObsFtrNames();
			var invObsCoords = obsFtrSpace.invertFeatureVector(coords);
			for (var i = 0; i < invObsCoords.length; i++) {
				observations.push({name: obsFtrNames[i], value: invObsCoords.at(i)});
			}

			var controlCoords = mc.fullCoords(stateId, false);
			var contrFtrNames = getControlFtrNames();
			var invControlCoords = controlFtrSpace.invertFeatureVector(controlCoords);
			for (var i = 0; i < invControlCoords.length; i++) {
				controls.push({name: contrFtrNames[i], value: invControlCoords.at(i)});
			}

			return {
				observations: observations,
				controls: controls
			};
    	}

    	function getFtrCoord(stateId, ftrIdx) {
    		if (ftrIdx < obsFtrSpace.dims.length) {
    			return obsFtrSpace.invertFeatureVector(mc.fullCoords(stateId))[ftrIdx];
    		} else {
    			return controlFtrSpace.invertFeatureVector(mc.fullCoords(stateId, false))[ftrIdx - obsFtrSpace.dims.length];
    		}
    	}

    	// public methods
    	var that = {
    		/**
    		 * Creates a new model out of the record set.
    		 */
    		fit: function (opts) {
    			var recSet = opts.recSet;
    			var batchEndV = opts.batchEndV;
    			var timeField = opts.timeField;

    			log.info('Updating feature space ...');
    			obsFtrSpace.updateRecords(recSet);
    			controlFtrSpace.updateRecords(recSet);

    			var obsColMat = obsFtrSpace.extractMatrix(recSet);
    			var contrColMat = controlFtrSpace.extractMatrix(recSet);
    			var timeV = recSet.getVector(timeField);

    			log.info('Creating model ...');
    			mc.fit({
    				observations: obsColMat,
    				controls: contrColMat,
    				times: timeV,
    				batchV: batchEndV
    			});
    			log.info('Done!');

    			return that;
    		},

    		/**
    		 * Adds a new record. Doesn't update the models statistics.
    		 */
    		update: function (rec) {
    			if (rec == null) return;

    			var obsFtrVec = obsFtrSpace.extractVector(rec);
    			var contFtrVec = controlFtrSpace.extractVector(rec);
    			var timestamp = rec.time.getTime();

    			mc.update(obsFtrVec, contFtrVec, timestamp);
    		},

    		/**
    		 * Saves the feature space and model into the specified files.
    		 */
    		save: function (mcFName) {
    			try {
    				console.log('Saving Markov chain ...');

    				var fout = new fs.FOut(mcFName);

	    			mc.save(fout);
	    			obsFtrSpace.save(fout);
	    			controlFtrSpace.save(fout);

	    			fout.flush();
	    			fout.close();

	    			console.log('Done!');
    			} catch (e) {
    				console.log('Failed to save the model!!' + e.message);
    			}
    		},

    		/**
    		 * Returns the state used in the visualization.
    		 */
    		getVizState: function () {
    			log.debug('Fetching visualization ...');
    			return mc.toJSON();
    		},

    		/**
    		 * Returns the hierarchical Markov chain model.
    		 */
    		getModel: function () {
    			return mc;
    		},

    		/**
    		 * Returns the feature space.
    		 */
    		getFtrSpace: function () {
    			return { observations: obsFtrSpace, controls: controlFtrSpace };
    		},

    		/**
    		 * Returns the current state at the specified height. If the height is not specified it
    		 * returns the current states through the hierarchy.
    		 */
    		currState: function (height) {
    			return mc.currState(height);
    		},

    		/**
    		 * Returns the most likely future states.
    		 */
    		futureStates: function (level, state, time) {
    			return mc.futureStates(level, state, time);
    		},

    		/**
    		 * Returns the most likely future states.
    		 */
    		pastStates: function (level, state, time) {
    			return mc.pastStates(level, state, time);
    		},

    		getFtrNames: function () {
    			return {
    				observation: getObsFtrNames(),
    				control: getControlFtrNames()
    			}
    		},

    		/**
    		 * Returns state details as a Javascript object.
    		 */
    		stateDetails: function (stateId, height) {
    			var futureStates = mc.futureStates(height, stateId);
    			var pastStates = mc.pastStates(height, stateId);
    			var isTarget = mc.isTarget(stateId, height);
    			var stateNm = mc.getStateName(stateId);
    			var wgts = mc.getStateWgtV(stateId);

    			var features = getFtrDescriptions(stateId);

    			return {
    				id: stateId,
    				name: stateNm.length > 0 ? stateNm : null,
    				isTarget: isTarget,
    				features: features,
    				futureStates: futureStates,
    				pastStates: pastStates,
    				featureWeights: wgts
    			};
    		},

    		/**
    		 * Returns a histogram for the desired feature in the desired state.
    		 */
    		histogram: function (stateId, ftrIdx) {
    			var hist = mc.histogram(stateId, ftrIdx);

    			var nObsFtrs = getObsFtrCount();

    			if (ftrIdx < nObsFtrs) {
	    			for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = obsFtrSpace.invertFeature(ftrIdx, hist.binStartV[i]);
	    			}
    			} else {
    				for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = controlFtrSpace.invertFeature(ftrIdx - nObsFtrs, hist.binStartV[i]);
	    			}
    			}

    			return hist;
    		},

    		/**
    		 * Callback when the current state changes.
    		 */
    		onStateChanged: function (callback) {
    			mc.onStateChanged(callback);
    		},

    		/**
    		 * Callback when an anomaly is detected.
    		 */
    		onAnomaly: function (callback) {
    			mc.onAnomaly(callback);
    		},

    		onOutlier: function (callback) {
    			mc.onOutlier(function (ftrV) {
    				var invFtrV = obsFtrSpace.invertFeatureVector(ftrV);

    				var features = [];
    				for (var i = 0; i < invFtrV.length; i++) {
    					features.push({name: obsFtrSpace.getFeature(i), value: invFtrV.at(i)});
    				}

    				callback(features);
    			});
    		},

    		onPrediction: function (callback) {
    			mc.onPrediction(callback);
    		},

    		/**
    		 * Returns the distribution of features accross the states on the
    		 * specified height.
    		 */
    		getFtrDist: function (height, ftrIdx) {
    			var stateIds = mc.stateIds(height);

    			var result = [];
    			for (var i = 0; i < stateIds.length; i++) {
    				var stateId = stateIds[i];
    				var coord = getFtrCoord(stateId, ftrIdx);
    				result.push({ state: stateId, value: coord });
    			}

    			return result;
    		},

    		setControl: function (ftrIdx, factor) {
    			var controlFtrIdx = ftrIdx - obsFtrSpace.dims.length;
    			mc.setControlFactor(controlFtrIdx, factor);
    		}
    	};

    	return that;
    };

    /**
    * @classdesc Anomaly detector that checks if the test point is too far from
    * the nearest known point.
    * @class
    * @param {Object} [detectorParam={rate:0.05}] - Constructor parameters
    * @param {number} [detectorParam.rate=0.05] - The rate is the expected fraction of emmited anomalies (0.05 -> 5% of cases will be classified as anomalies)
    */
    exports.NearestNeighborAD = function(detectorParam) {
        // Parameters
        detectorParam = detectorParam == undefined ? {} : detectorParam;
        detectorParam.rate = detectorParam.rate == undefined ? 0.05 : detectorParam.rate;
        assert(detectorParam.rate > 0 && detectorParam.rate <= 1.0, 'rate parameter not in range (0,1]');
        // model param
        this.rate = detectorParam.rate;
        // default model
        this.thresh = 0;

        /**
        * Gets the 100*(1-rate) percentile
        * @param {module:la.Vector} vector - Vector of values
        * @returns {number} Percentile
        */
        function getThreshold(vector, rate) {
            var sorted = vector.sortPerm().vec;
            var idx = Math.floor((1 - rate) * sorted.length);
            return sorted[idx];
        }
        var neighborDistances = undefined;

        /**
        * Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of
        * the model.
        */
        this.fit = function (A) {
            this.X = A;
            // distances
            var D = la.pdist2(A, A);
            // add big numbers on the diagonal (exclude the point itself from the nearest point calcualtion)
            var E = D.plus(D.multiply(la.ones(D.rows)).diag()).multiply(-1);
            var neighborDistances = new la.Vector({ vals: D.rows });
            for (var i = 0; i < neighborDistances.length; i++) {
                // nearest neighbour squared distance
                neighborDistances[i] = D.at(i, E.rowMaxIdx(i));
            }
            this.thresh = getThreshold(neighborDistances, this.rate);
        }

        /**
        * Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold)
        * @param {module:la.Vector} x - Test vector
        * @returns {number} Returns 1.0 if x is an anomaly and 0.0 otherwise
        */
        this.predict = function (x) {
            // compute squared dist and compare to thresh
            var d = la.pdist2(this.X, x.toMat()).getCol(0);
            var idx = d.multiply(-1).getMaxIdx();
            var p = d[idx];
            //console.log(p)
            return p > this.thresh ? 1 : 0;
        }

        /**
        * Adds a new point (or points) to the known points and recomputes the threhshold
        * @param {(module:la.Vector | module:la.Matrix)} x - Test example (vector input) or column examples (matrix input)
        */
        this.update = function (x) {
            // append x to known examples and retrain (slow version)
            // speedup 1: don't reallocate X every time (fixed window, circular buffer)
            // speedup 2: don't recompute distances d(X,X), just d(X, y), get the minimum
            // and append to neighborDistances
            this.fit(la.cat([[this.X, x.toMat()]]));
            //console.log('new threshold ' + this.thresh);
        }
    }


    //!ENDJSDOC

    return exports;
}
