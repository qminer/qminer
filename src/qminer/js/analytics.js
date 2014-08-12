// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

var util = require("utilities.js");

module.exports = require("__analytics__");
exports = module.exports; // re-establish link

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
    }

    this.predict = function (record) {
        var vec = this.featureSpace.ftrSpVec(record);
        var result = { };
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
                top = { category : cat, weight: result[cat] }
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
    var featureSpace = exports.newFeatureSpace(features);
    // initialize features
    featureSpace.updateRecords(records);
    console.log("newBatchModel", "  number of dimensions = " + featureSpace.dim);    
    // prepare spare vectors
    console.log("newBatchModel", "  preparing feature vectors");   
    var sparseVecs = featureSpace.ftrSpColMat(records);
    // prepare target vectors
    var targets = { };
    // figure out if new category name, or update count
    function initCats(categories, catName) {
        if (categories[catName]) {
            categories[catName].count++; 
        } else {
            // check if we should ignore this category
            if (limitCategories && !util.isInArray(limitCategories, catName)) { return; }
            // check if we should ignore this category
            categories[catName] = { 
                name: catName, 
                type: "classification",
                count: 1, 
                target: linalg.newVec({mxVals : records.length})
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
            var cats = records[i][target.name];
            for (var cat in targets) {
                targets[cat].target.push(util.isInArray(cats, cat) ? 1.0 : -1.0);
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
            target: linalg.newVec({mxVals : records.length})                    
    
        };
        for (var i = 0; i < records.length; i++) {
            targets[target.name].target.push(records[i][target.name]);
        }        
    }
    // training model for each category
    console.log("newBatchModel", "  training SVM");
    var models = { };
    for (var cat in targets) {
        if (targets[cat].count >= 50) {
            models[cat] = {
                name : targets[cat].name,
                type : targets[cat].type,
            };
            if (targets[cat].type === "classification") {
                console.log("newBatchModel", "    ... " + cat + " (classification)");
                models[cat].model = exports.trainSvmClassify(sparseVecs, targets[cat].target, 
                    { c: 1, j: 1, batchSize: 10000, maxIterations: 100000, maxTime: 600, minDiff: 0.001 });
            } else if (targets[cat].type === "regression") {
                console.log("newBatchModel", "    ... " + cat + " (regression)");
                models[cat].model = exports.trainSvmRegression(sparseVecs, targets[cat].target, 
                    { c: 1, eps: 1e-2, batchSize: 10000, maxIterations: 100000, maxTime: 600, minDiff: 0.001 });
            }
        }
    }
    // done
    console.log("newBatchModel", "Done");  
    // we finished the constructor
    return new createBatchModel(featureSpace, models);
}

//#- `batchModel = analytics.loadBatchModel(fin)` -- loads batch model frm input stream `fin`
exports.loadBatchModel = function (sin) {
    var models = JSON.parse(sin.readLine());
    var featureSpace = exports.loadFeatureSpace(sin);
    for (var cat in models) {        
        models[cat].model = exports.loadSvmModel(sin);
    }
    // we finished the constructor
    return new createBatchModel(featureSpace, models);    
}

function round100(num) { return Math.round(num * 100) / 100; }

function classifcationScore(cats) {
	this.target = { };
    
	this.targetList = [ ];
	for (var i = 0; i < cats.length; i++) {
		this.target[cats[i]] = { 
			id: i, count: 0, predictionCount: 0,
			TP: 0, TN: 0, FP: 0, FN: 0,
			all : function () { return this.TP + this.FP + this.TN + this.FN; },
			precision : function () { return this.TP / (this.TP + this.FP); },
			recall : function () { return this.TP / (this.TP + this.FN); },				
			accuracy : function () { return (this.TP + this.TN) / this.all(); }
		};
		this.targetList.push(cats[i]);		
	}
	
	this.confusion = la.newMat({ rows: this.targetList.length, cols: this.targetList.length }); 
	   
	this.count = function (correct, predicted) {
        // wrapt classes in arrays if not already
        if (util.isString(correct)) { this.count([correct], predicted); return; }
        if (util.isString(predicted)) { this.count(correct, [predicted]); return; }
        // go over all possible categories and counts
        for (cat in this.target) {
            var catCorrect = util.isInArray(correct, cat);
            var catPredicted = util.isInArray(predicted, cat);            
            // update counts for correct categories
            if (catCorrect) { this.target[cat].count++; }
            // update counts for how many times category was predicted
            if (catPredicted) { this.target[cat].predictionCount; }
            // update true/false positive/negative count
            if (catCorrect && catPredicted) {
                // both predicted and correct say true
                this.target[cat].TP++;
            } else if (catCorrect) {
                // this was only correct but not predicted
                this.target[cat].FN++;
            } else if (catPredicted) {
                // this was only predicted but not correct
                this.target[cat].FP++;
            } else {
                // both predicted and correct say false
                this.target[cat].TN++;
            }
            // update confusion matrix
        }
        // update confusion matrix for case when are predicting only one label
//        if (correct.length == 1 && predicted.length == 1) {
//            var correct_i = this.target[correct[0]].id;
//            var predicted_i = this.target[predicted[0]].id;	
//            var old_count = this.confusion.at(correct_i, predicted_i);
//            this.confusion.put(correct_i, predicted_i, old_count + 1);
//        }
	}

	this.report = function () { 
		for (cat in this.target) {
			console.log(cat + 
				": Count " + this.target[cat].count + 
				", All " + this.target[cat].all() + 
				", Precission " + round100(this.target[cat].precision()) + 
				", Recall " +   round100(this.target[cat].recall()) +
				", Accuracy " +   round100(this.target[cat].accuracy()));
		}
	}

	this.reportCSV = function (fout) { 
		// precison recall
		fout.writeLine("category,count,precision,recall,accuracy");
		for (cat in this.target) {
			fout.writeLine(cat + 
				"," + this.target[cat].count + 
				"," + round100(this.target[cat].precision()) + 
				"," + round100(this.target[cat].recall()) +
				"," + round100(this.target[cat].accuracy()));
		}
		// confusion header
//		fout.writeLine();		
//		for (var i = 0; i < this.targetList.length; i++) {
//			fout.write(",");
//			fout.write(this.targetList[i]);
//		}
//		fout.writeLine();
//		for (var i = 0; i < this.targetList.length; i++) {
//			fout.write(this.targetList[i]);
//			for (var j = 0; j < this.targetList.length; j++) {
//				fout.write(",");
//				fout.write(this.confusion.at(i,j));
//			}
//			fout.writeLine();
//		}
	}
	
	this.results = function () {
		var res = { };
		for (cat in this.target) {
			res[cat] = {
				precision : this.target[cat].precision(),
				recall    : this.target[cat].recall(),
				accuracy  : this.target[cat].accuracy(),
			}
		}
	}
}

//#- `result = analytics.crossValidation(rs, features, target)` -- creates a batch
//#     model for records from record set `rs` using `features; `target` is the
//#     target field and is assumed discrete; the result is a results object
//#     with the following API:
//#     - `result.target` -- an object with categories as keys and the following
//#       counts as members of these keys: `count`, `TP`, `TN`, `FP`, `FN`,
//#       `all()`, `precision()`, `recall()`, `accuracy()`.
//#     - `result.confusion` -- confusion matrix between categories
//#     - `result.report()` -- prints basic report on to the console
//#     - `result.reportCSV(fout)` -- prints CSV output to the `fout` output stream
exports.crossValidation = function (records, features, target, folds, limitCategories) {
	// create empty folds
	var fold = [];
	for (var i = 0; i < folds; i++) {
		fold.push(la.newIntVec());
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
		var train = la.newIntVec();
		var test = la.newIntVec();
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
		if (!cfyRes) { cfyRes = new classifcationScore(model.target); }
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
}

// active learning (analytics:function activeLearner, parameters
//#- `alModel = analytics.newActiveLearner(fsp, textField, rs, nPos, nNeg, query, c, j)` -- initializes the
//#    active learning. The algorihm is run by calling `model.startLoop()`. The algorithm has two stages: query mode, where the algorithm suggests potential
//#    positive and negative examples based on the query text, and SVM mode, where the algorithm keeps
//#   selecting examples that are closest to the SVM margin (every time an example is labeled, the SVM
//#   is retrained.
//#   The inputs are the feature space `ftrSpace`, `textField` (string) which is the name
//#    of the field in records that is used to create feature vectors, `recSet` (record set) a set of records from a store
//#    that is used as unlabeled data, `nPos` (integer) and `nNeg` (integer) set the number of positive and negative
//#    examples that have to be identified in the query mode before the program enters SVM mode.
//#   The next parameter is the `query` (string) which should be related to positive examples. 
//#   Final Parameters `c` and `j` are SVM parameters.
exports.newActiveLearner = function (ftrSpace, textField, recSet, nPos, nNeg, query, c, j) {
    return new exports.activeLearner(ftrSpace, textField, recSet, nPos, nNeg, query, c, j);
}
exports.activeLearner = function (ftrSpace, textField, recSet, nPos, nNeg, query, c, j) {
    // svm parameter defaults
    c = c || 1.0; j = j || 1.0;
    var store = recSet.store;
    var X = la.newSpMat();
    var y = la.newVec();
    // QUERY MODE
    var queryMode = true;
    // bow similarity between query and training set 
    var queryObj ={}; queryObj[textField] = query;
    var queryRec = store.newRec(queryObj); // record
    var querySpVec = ftrSpace.ftrSpVec(queryRec); // query sparse vector
    querySpVec.normalize();
    var recsMat = ftrSpace.ftrSpColMat(recSet); //recSet feature matrix
    recsMat.normalizeCols();
    var simV = recsMat.multiplyT(querySpVec); //similarities (q, recSet)
    var sortedSimV = simV.sortPerm(); //ascending sort
    var simVs = sortedSimV.vec; //sorted similarities (q, recSet)
    var simVp = sortedSimV.perm; //permutation of sorted similarities (q, recSet)
    // counters for questions in query mode
    var nPosQ = 0; //for traversing simVp from the end
    var nNegQ = 0; //for traversing simVp from the start

    // SVM MODE
    var svm;
    var posIdxV = la.newIntVec(); //indices in recordSet
    var negIdxV = la.newIntVec(); //indices in recordSet
    var posIdV = la.newIntVec(); //record IDs
    var negIdV = la.newIntVec(); //record IDs
    var classVec = la.newVec({ "vals": recSet.length }); //svm scores for record set
    var resultVec = la.newVec({ "vals": recSet.length }); // non-absolute svm scores for record set
    // returns record set index of the unlabeled record that is closest to the margin
    //#   - `recSetIdx = alModel.selectQuestion()` -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
    this.selectQuestion = function () {
        if (posIdV.length >= nPos && negIdV.length >= nNeg) { queryMode = false; }
        if (queryMode) {
            if (posIdV.length < nPos) {
                nPosQ = nPosQ + 1;
                console.say("query mode, try to get pos");
                return simVp[simVp.length - 1 - (nPosQ - 1)];
            }
            if (negIdV.length < nNeg) {
                nNegQ = nNegQ + 1;
                console.say("query mode, try to get neg");
                return simVp[nNegQ - 1];
            }
        }
        else {
            ////call svm, get record closest to the margin            
            //console.startx(function (x) { return eval(x); });
            svm = analytics.trainSvmClassify(X, y, {c: c, j: j}); //column examples, y float vector of +1/-1, default svm paramvals
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
            for (var recN = 0; recN < recSet.length; recN++) {
                if (classVec[recN] !== Number.POSITIVE_INFINITY) {

                    var svmMargin = svm.predict(recsMat[recN]);
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
            console.say("svm mode, margin: " + sorted.vec[0] + ", npos: " + posCount + ", nneg: " + negCount);
            return sorted.perm[0];            
        }
    };
    // asks the user for class label given a record set index
    //#   - `alModel.getAnswer(alAnswer, recSetIdx)` -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
    //#      The user input should be either "y" (indicating that recSet[recSetIdx] is a positive example), "n" (negative example).
    this.getAnswer = function (ALanswer, recSetIdx) {
        //todo options: ?newQuery
        if (ALanswer === "y") {
            posIdxV.push(recSetIdx);
            posIdV.push(recSet[recSetIdx].$id);
            X.push(recsMat[recSetIdx]);
            y.push(1.0);
        } else {
            negIdxV.push(recSetIdx);
            negIdV.push(recSet[recSetIdx].$id);
            X.push(recsMat[recSetIdx]);
            y.push(-1.0);
        }
        // +k query // rank unlabeled according to query, ask for k most similar
        // -k query // rank unlabeled according to query, ask for k least similar
    };
    //#   - `alModel.startLoop()` -- starts the active learning loop in console
    this.startLoop = function() {
        while (true) {
            var recSetIdx = this.selectQuestion();
            console.say(recSet[recSetIdx].Text + ": y/(n)/stop?");
            var ALanswer = console.getln();
            if (ALanswer == "stop") { break; }
            if (posIdxV.length + negIdxV.length == recSet.length) { break;}
            this.getAnswer(ALanswer, recSetIdx);            
        }
    };
    //#   - `alModel.saveSvmModel(fout)` -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
    this.saveSvmModel = function (outputStream) {
        // must be in SVM mode
        if (queryMode) {
            console.say("AL.save: Must be in svm mode");
            return;
        }
        svm.save(outputStream);
    };
    //#   - `numArr = alModel.getPos(thresh)` -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
    this.getPos = function(threshold) {
      if(this.queryMode) { return null; } // must be in SVM mode to return results
      if(!threshold) { threshold = 0; }
      var posIdxArray = [];
      for (var recN = 0; recN < recSet.length; recN++) {
        if (resultVec[recN] >= threshold) {
          posIdxArray.push(recN);
        }
      }
      return posIdxArray;
    };
    //#   - `bool = alModel.getQueryMode()` -- returns true if in query mode, false otherwise (SVM mode)
    this.getQueryMode = function() {
      return queryMode;
    };
    //#   - `num = alModel.getnpos()` -- return the  number of examples marked as positive. 
    this.getnpos = function() { return npos; };
    //#   - `num = alModel.getnneg()` -- return the  number of examples marked as negative.
    this.getnneg = function() { return nneg; };
    //#   - `alModel.setj(num)` - sets the SVM j parameter to the provided value.
    this.setj = function(nj) { j = nj; };
    //#   - `alModel.setc(num)` - sets the SVM c parameter to the provided value.
    this.setc = function(nc) { c = nc; };
    //this.saveLabeled
    //this.loadLabeled
};


//////////// RIDGE REGRESSION 
// solve a regularized least squares problem
//#- `ridgeRegressionModel = analytics.newRidgeRegression(kappa, dim, buffer)` -- solves a regularized ridge
//#  regression problem: min|X w - y|^2 + kappa |w|^2. The inputs to the algorithm are: `kappa`, the regularization parameter,
//#  `dim` the dimension of the model and (optional) parameter `buffer` (integer) which specifies
//#  the length of the window of tracked examples (useful in online mode). The model exposes the following functions:
exports.newRidgeRegression = function (kappa, dim, buffer) {
    return new analytics.ridgeRegression(kappa, dim, buffer);
}
exports.ridgeRegression = function (kappa, dim, buffer) {
    var X = [];
    var y = [];
    buffer = typeof buffer !== 'undefined' ? buffer : -1;
    var w = la.newVec({ "vals": dim });
    //#   - `ridgeRegressionModel.add(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set
    this.add = function (x, target) {
        X.push(x);
        y.push(target);
        if (buffer > 0) {
            if (X.length > buffer) {
                this.forget(X.length - buffer);
            }
        }
    };
    //#   - `ridgeRegressionModel.addupdate(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set and retrains the model
    this.addupdate = function (x, target) {
        this.add(x, target);
        this.update();
    }
    //#   - `ridgeRegressionModel.forget(n)` -- deletes first `n` (integer) examples from the training set
    this.forget = function (ndeleted) {
        ndeleted = typeof ndeleted !== 'undefined' ? ndeleted : 1;
        ndeleted = Math.min(X.length, ndeleted);
        X.splice(0, ndeleted);
        y.splice(0, ndeleted);
    };
    //#   - `ridgeRegressionModel.update()` -- recomputes the model
    this.update = function () {
        var A = this.getMatrix();
        var b = la.copyFltArrayToVec(y);
        w = this.compute(A, b);
    };
    //#   - `vec = ridgeRegressionModel.getModel()` -- returns the parameter vector `vec` (dense vector)
    this.getModel = function () {
        return w;
    };
    this.getMatrix = function () {
        if (X.length > 0) {
            var A = la.newMat({ "cols": X[0].length, "rows": X.length });
            for (var i = 0; i < X.length; i++) {
                A.setRow(i, X[i]);
            }
            return A;
        }
    };
    //#   - `vec2 = ridgeRegressionModel.compute(mat, vec)` -- computes the model parameters `vec2`, given 
    //#    a row training example matrix `mat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |mat' vec2 - vec|^2 + kappa |vec2|^2.
    //#   - `vec2 = ridgeRegressionModel.compute(spMat, vec)` -- computes the model parameters `vec2`, given 
    //#    a row training example sparse matrix `spMat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |spMat' vec2 - vec|^2 + kappa |vec2|^2.
    this.compute = function (A, b) {
        var I = la.eye(A.cols);
        var coefs = (A.transpose().multiply(A).plus(I.multiply(kappa))).solve(A.transpose().multiply(b));
        return coefs;
    };
    //#   - `num = model.predict(vec)` -- predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.
    this.predict = function (x) {
        return w.inner(x);
    };
};

///////// CLUSTERING BATCH K-MEANS
//#- `mat2 = analytics.computeKmeans(mat, k, iter)`-- solves the k-means algorithm based on a training
//#   matrix `mat`  where colums represent examples, `k` (integer) the number of centroids and
//#   `iter` (integer), the number of iterations. The solution `mat2` is a dense matrix, where each column
//#    is a cluster centroid.
//#- `mat2 = analytics.computeKmeans(spMat, k, iter)`-- solves the k-means algorithm based on a training
//#   matrix `spMat`  where colums represent examples, `k` (integer) the number of centroids and
//#   `iter` (integer), the number of iterations. The solution `mat2` is a dense matrix, where each column
//#    is a cluster centroid.
exports.kmeans = function(X, k, iter) {
    // select random k columns of X, returns a dense C++ matrix
    var selectCols = function (X, k) {
        var idx = la.randIntVec(X.cols, k);
        var idxMat = la.newSpMat({ "rows": X.cols });
        for (var i = 0; i < idx.length; i++) {
            var spVec = la.newSpVec([[idx[i], 1.0]], { "dim": X.cols });
            idxMat.push(spVec);
        }
        var C = X.multiply(idxMat);
        var result = new Object();
        result.C = C;
        result.idx = idx;
        return result;
    };

    // modified k-means algorithm that avoids empty centroids
    // A Modified k-means Algorithm to Avoid Empty Clusters, Malay K. Pakhira
    // http://www.academypublisher.com/ijrte/vol01/no01/ijrte0101220226.pdf
    var getCentroids = function (X, idx, oldC) {
        // select random k columns of X, returns a dense matrix
        // 1. construct a sparse matrix (coordinate representation) that encodes the closest centroids
        var idxvec = la.copyIntArrayToVec(idx);
        var rangeV = la.rangeVec(0, X.cols - 1);
        var ones_cols = la.ones(X.cols);
        var idxMat = la.newSpMat(idxvec, rangeV, ones_cols, X.cols);
        idxMat = idxMat.transpose();
        var ones_n = la.ones(X.cols);
        // 2. compute the number of points that belong to each centroid, invert
        var colSum = idxMat.multiplyT(ones_n);
        for (var i = 0; i < colSum.length; i++) {
            var val = 1.0 / (1.0 + colSum.at(i)); // modification
            colSum.put(i, val);
        }
        // 3. compute the centroids
        var w = new util.clsStopwatch();
        w.tic();
        var sD = colSum.spDiag();
        var C = ((X.multiply(idxMat)).plus(oldC)).multiply(sD); // modification
        return C;
    };


    // X: column examples
    // k: number of centroids
    // iter: number of iterations
    assert.ok(k <= X.cols, "k <= X.cols");
    var w = new util.clsStopwatch();
    var norX2 = la.square(X.colNorms());
    var initialCentroids = selectCols(X, k);
    var C = initialCentroids.C;
    var idxvOld = initialCentroids.idx;
    //printArray(idxvOld); // DEBUG
    var ones_n = la.ones(X.cols).multiply(0.5);
    var ones_k = la.ones(k).multiply(0.5);
    w.tic();
    for (var i = 0; i < iter; i++) {
        console.say("iter: " + i);
        var norC2 = la.square(C.colNorms());
        //D =  full(C'* X) - norC2' * (0.5* ones(1, n)) - (0.5 * ones(k,1) )* norX2';
        var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
        var idxv = la.findMaxIdx(D);
        //var energy = 0.0;
        //for (var j = 0; j < X.cols; j++) {            
        //    if (D.at(idxv[j],j) < 0) {
        //        energy += Math.sqrt(-2 * D.at(idxv[j], j));
        //    }
        //}
        //console.say("energy: " + 1.0/ X.cols * energy);
        if (util.arraysIdentical(idxv, idxvOld)) {
            console.say("converged at iter: " + i); //DEBUG
            break;
        }
        idxvOld = idxv.slice();
        C = getCentroids(X, idxv, C); //drag
    }
    w.toc("end");
    return C;
};


////////////// ONLINE CLUSTERING (LLOYD ALGORITHM)
//#- `lloydModel = analytics.newLloyd(dim, k)` -- online clustering based on the Lloyd alogrithm. The model intialization
//#  requires `dim` (integer) the dimensionality of the inputs and `k` (integer), number of centroids. The model exposes the following functions:
exports.newLloyd = function (dim, k) {
    return new analytics.lloyd(dim, k);
}
exports.lloyd = function (dim, k) {
    // Private vars
    var C = la.genRandomMatrix(dim, k);//linalg.newMat({ "rows": dim, "cols": k, "random": true });;
    var counts = la.ones(k);
    var norC2 = la.square(C.colNorms());
    //#   - `lloydModel.init()` -- initializes the model with random centroids
    this.init = function () {
        C = la.genRandomMatrix(dim, k); //linalg.newMat({ "rows": dim, "cols": k, "random": true });
        counts = la.ones(k);
        norC2 = la.square(C.colNorms());
    };
    //#   - `mat = lloydModel.getC()` -- returns the centroid matrix `mat`
    this.getC = function () {
        return C;
    };

    this.giveAll = function () {
        var result = new Object();
        result.C = C;
        result.counts = counts;
        result.norC2 = norC2;
        return result;
    };
    //#   - `lloydModel.setC(mat)` -- sets the centroid matrix to matrix `mat`
    this.setC = function (C_) {
        C = la.newMat(C_);
        norC2 = la.square(C.colNorms());
    };
    //#   - `lloydModel.update(vec)` -- updates the model with a vector `vec`
    //#   - `lloydModel.update(spVec)` -- updates the model with a sparse vector `spVec`
    this.update = function (x) {
        var idx = this.getCentroidIdx(x);
        //C(:, idx) = 1/(counts[idx] + 1)* (counts[idx] * C(:, idx)  + x);
        var vec = ((C.getCol(idx).multiply(counts[idx])).plus(x)).multiply(1.0 / (counts[idx] + 1.0));
        C.setCol(idx, vec);
        counts[idx] = counts[idx] + 1;
        norC2[idx] = la.square(vec.norm());
    };
    //#   - `vec2 = lloydModel.getCentroid(vec)` -- returns the centroid `vec2` (dense vector) that is the closest to vector `vec`
    //#   - `vec2 = lloydModel.getCentroid(spVec)` -- returns the centroid `vec2` (dense vector) that is the closest to sparse vector `spVec`
    this.getCentroid = function (x) {
        var idx = this.getCentroidIdx(x);
        var vec = C.getCol(idx);
        return vec;
    };
    //#   - `idx = lloydModel.getCentroidIdx(vec)` -- returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to vector `vec`
    //#   - `idx = lloydModel.getCentroidIdx(spVec)` -- returns the centroid index `idx` (integer) that corresponds to the centroid that is the closest to sparse vector `spVec`
    this.getCentroidIdx = function (x) {
        var D = C.multiplyT(x);
        D = D.minus(norC2.multiply(0.5));
        var idxv = la.findMaxIdx(D);
        return idxv[0];
    };
};

/////////// perceptron : 0/1 classification
//#- `perceptronModel = analytics.newPerceptron(dim, use_bias)` -- the perceptron learning algorithm initialization requires
//#   specifying the problem dimensions `dim` (integer) and optionally `use_bias` (boolean, default=false). The
//#   model is used to solve classification tasks, where classifications are made by a function class(x) = sign(w'x + b). The following functions are exposed:
exports.newPerceptron = function (dim, use_bias) {
    return new analytics.perceptron(dim, use_bias);
}
exports.perceptron = function (dim, use_bias) {
    use_bias = typeof use_bias !== 'undefined' ? use_bias : false;
    var w = la.newVec({ "vals": dim });
    var b = 0;
    //#   - `perceptronModel.update(vec,num)` -- updates the internal parameters `w` and `b` based on the training feature vector `vec` and target class `num` (0 or 1)! 
    //#   - `perceptronModel.update(spVec,num)` -- updates the internal parameters `w` and `b` based on the training sparse feature vector `spVec` and target class `num` (0 or 1)! 
    this.update = function (x, y) {
        var yp = (w.inner(x) + b) > 0;
        if (y != yp) {
            var e = y - yp;
            w = w.plus(x.multiply(e));
            if (use_bias) {
                b = b + e;
            }
        }
    };
    //#   - `num = perceptronModel.predict(vec)` -- returns the prediction (0 or 1) for a vector `vec`
    //#   - `num = perceptronModel.predict(spVec)` -- returns the prediction (0 or 1) for a sparse vector `spVec`
    this.predict = function (x) {
        return (w.inner(x) + b) > 0 ? 1 : 0;
    };
    //#   - `perceptronParam = perceptronModel.getModel()` -- returns an object `perceptronParam` where `perceptronParam.w` (vector) and `perceptronParam.b` (bias) are the separating hyperplane normal and bias. 
    this.getModel = function () {
        var model;
        model.w = w;
        model.b = b;
        return model;
    };

};


///////// ONLINE KNN REGRESSION 
//#- `kNearestNeighbors = analytics.newKNearestNeighbors(k, buffer, power)`-- online regression based on knn alogrithm. The model intialization
//#  requires `k` (integer), number of nearest neighbors, optional parameter `buffer` (default is -1) and optional parameter `power` (default is 1), 
//#  when using inverse distance weighting average to compute prediction. The model exposes the following functions:
exports.newKNearestNeighbors = function (k, buffer, power) {
    return new analytics.kNearestNeighbors(k, buffer, power);
}
exports.kNearestNeighbors = function (k, buffer, power) {
    this.X = la.newMat();
    this.y = la.newVec();
    this.k = k;

    // Optional parameters
    var power = typeof power !== 'undefined' ? power : 1;
    var buffer = typeof buffer !== 'undefined' ? buffer : -1;

    // Internal vector logs to create X and y
    var matrixVec = [];
    var targetVec = [];

    //#   - `kNearestNeighbors.update(vec, num)` -- adds a vector `vec` and target `num` (number) to the "training" set
    this.update = function (vec, num) {
        //console.log("Updated..."); //DEBUG
        add(vec, num);
        this.X = getColMatrix();
        this.y = getTargetVec();
    }

    //#   - `num = kNearestNeighbors.predict(vec)` -- predicts the target `num` (number), given feature vector `vec` based on k nearest neighburs,
    //#   using simple average, or inverse distance weighting average, where `power` (intiger) is optional parameter.
    this.predict = function (vec) {
        if (this.X.cols < this.k) { return -1 }
        var neighbors = this.getNearestNeighbors(vec); //vector of indexes
        var targetVals = this.y.subVec(neighbors.perm);
        var prediction = getAverage(targetVals); // using simple average
        //var prediction = getIDWAverage(targetVals, neighbors.vec, power); // using inverse distance weighting average
        return prediction;
    }

    //#   - `object = kNearestNeighbors.getNearestNeighbors(vec)` -- findes k nearest neighbors. Returns object with two vectors: indexes `perm` (intVec) and values `vec` (vector)
    this.getNearestNeighbors = function (vec) {
        var distVec = la.pdist2(this.X, la.repvec(vec, 1, this.X.cols)).getCol(0);
        var sortRes = distVec.sortPerm(); // object with two vectors: values and indexes

        var result = new Object();
        var newPerm = la.newIntVec({ "vals": this.k, "mxvals": this.k });
        var newVec = la.newVec({ "vals": this.k, "mxvals": this.k });
        for (var ii = 0; ii < this.k; ii++) {
            newPerm[ii] = sortRes.perm[ii];
            newVec[ii] = sortRes.vec[ii];
        }
        result.perm = newPerm;
        result.vec = newVec;
        return result; // object with two vectors: values and indexes
    }

    // Calculate simple average
    var getAverage = function (vec) {
        var sum = vec.sum();
        var avr = sum / vec.length;
        return avr;
    }

    // Inverse distance weighting average
    var getIDWAverage = function (vec, dist, power) {
        var numerator = la.elementByElement(vec, dist, function (a, b) { return result = b == 0 ? a : a / Math.pow(b, power) }).sum();
        var denumerator = la.elementByElement(la.ones(dist.length), dist, function (a, b) { return result = b == 0 ? a : a / Math.pow(b, power) }).sum();
        return numerator / denumerator;
    }

    // Used for updatig
    var add = function (x, target) {
        matrixVec.push(x);
        targetVec.push(target);
        if (buffer > 0) {
            if (matrixVec.length > buffer) {
                forget(matrixVec.length - buffer);
            }
        }
    }

    // Create row matrix from matrixVec array log
    var getMatrix = function () {
        if (matrixVec.length > 0) {
            var A = la.newMat({ "cols": matrixVec[0].length, "rows": matrixVec.length });
            for (var i = 0; i < matrixVec.length; i++) {
                A.setRow(i, matrixVec[i]);
            }
            return A;
        }
    };

    // Create column matrix from matrixVec array log
    var getColMatrix = function () {
        if (matrixVec.length > 0) {
            var A = la.newMat({ "cols": matrixVec.length, "rows": matrixVec[0].length });
            for (var i = 0; i < matrixVec.length; i++) {
                A.setCol(i, matrixVec[i]);
            }
            return A;
        }
    };

    // Forget function used in buffer. Deletes first `n` (integer) examples from the training set
    var forget = function (ndeleted) {
        ndeleted = typeof ndeleted !== 'undefined' ? ndeleted : 1;
        ndeleted = Math.min(matrixVec.length, ndeleted);
        matrixVec.splice(0, ndeleted);
        targetVec.splice(0, ndeleted);
    };

    // Create vector from targetVec array
    var getTargetVec = function () {
        return la.copyFltArrayToVec(targetVec);
    }
}


/////////// Kalman Filter
//#- `kf = analytics.newKalmanFilter(dynamParams, measureParams, controlParams)` -- the Kalman filter initialization procedure
//#   requires specifying the model dimensions `dynamParams` (integer), measurement dimension `measureParams` (integer) and
//#   the `controlParams` control dimension. Algorithm works in two steps - prediction (short time prediction according to the
//#   specified model) and correction. The following functions are exposed:
exports.newKalmanFilter = function (dynamParams, measureParams, controlParams) {
    return new analytics.kalmanFilter(dynamParams, measureParams, controlParams);
}
exports.kalmanFilter = function (dynamParams, measureParams, controlParams) {
    var CP = controlParams;
    var MP = measureParams;
    var DP = dynamParams;

    // CP should be >= 0
    CP = Math.max(CP, 0);

    var statePre = la.newVec({ "vals": DP, "mxvals": DP }); // prior state vector (after prediction and before measurement update)
    var statePost = la.newVec({ "vals": DP, "mxvals": DP }); // post state vector (after measurement update)
    var transitionMatrix = la.newMat({ "cols": DP, "rows": DP }); // transition matrix (model)

    var processNoiseCov = la.newMat({ "cols": DP, "rows": DP }); // process noise covariance
    var measurementMatrix = la.newMat({ "cols": DP, "rows": MP }); // measurement matrix
    var measurementNoiseCov = la.newMat({ "cols": MP, "rows": MP }); // measurement noise covariance
     
    var errorCovPre = la.newMat({ "cols": DP, "rows": DP }); // error covariance after prediction
    var errorCovPost = la.newMat({ "cols": DP, "rows": DP }); // error covariance after update
    var gain = la.newMat({ "cols": MP, "rows": DP }); 

    var controlMatrix;

    if (CP > 0)
        controlMatrix = la.newMat({ "cols": CP, "rows": DP }); // control matrix

    // temporary matrices used for calculation
    var temp1VV = la.newMat({ "cols": DP, "rows": DP });
    var temp2VV = la.newMat({ "cols": DP, "rows": MP });
    var temp3VV = la.newMat({ "cols": MP, "rows": MP });
    var itemp3VV = la.newMat({ "cols": MP, "rows": MP });
    var temp4VV = la.newMat({ "cols": DP, "rows": MP });

    var temp1V = la.newVec();
    var temp2V = la.newVec();

    //#   - `kf.setStatePost(_val)` -- sets the post state (DP) vector.
    this.setStatePost = function (_statePost) {
        statePost = _statePost;
    };

    //#   - `kf.setTransitionMatrix(_val)` -- sets the transition (DP x DP) matrix.
    this.setTransitionMatrix = function (_transitionMatrix) {
        transitionMatrix = _transitionMatrix;
    };

    //#   - `kf.setMeasurementMatrix(_val)` -- sets the measurement (MP x DP) matrix.
    this.setMeasurementMatrix = function (_measurementMatrix) {
        measurementMatrix = _measurementMatrix;
    };

    //#   - `kf.setProcessNoiseCovPost(_val)` -- sets the process noise covariance (DP x DP) matrix.
    this.setProcessNoiseCov = function (_processNoiseCov) {
        processNoiseCov = _processNoiseCov;
    }

    //#   - `kf.setMeasurementNoiseCov(_val)` -- sets the measurement noise covariance (MP x MP) matrix.
    this.setMeasurementNoiseCov = function (_measurementNoiseCov) {
        measurementNoiseCov = _measurementNoiseCov;
    }

    //#   - `kf.setErrorCovPre(_val)` -- sets the pre error covariance (DP x DP) matrix.
    this.setErrorCovPre = function (_errorCovPre) {
        errorCovPre = _errorCovPre;
    }

    //#   - `kf.setErrorCovPost(_val)` -- sets the post error covariance (DP x DP) matrix.
    this.setErrorCovPost = function (_errorCovPost) {
        errorCovPost = _errorCovPost;
    }

    //#   - `statePost = kf.correct(measurement)` -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
    this.setControlMatrix = function (_controlMatrix) {
        controlMatrix = _controlMatrix;
    }

    //#   - `statePre = kf.predict(control)` -- returns a predicted state vector `statePre` where `control` is the control vector (normally not set).
    this.predict = function (control) {
        // update the state: x'(k) = A * x(k)
        statePre = transitionMatrix.multiply(statePost);

        // x'(k) = x'(k) + B * u(k)
        if (control.length) {
            temp1V = controlMatrix.multiply(control);
            temp2V = statePre.plus(temp1V);
        }

        // update error covariance matrices: temp1 = A * P(k)
        temp1VV = transitionMatrix.multiply(errorCovPost);

        // P'(k) = temp1 * At + Q
        errorCovPre = temp1VV.multiply(transitionMatrix.transpose()).plus(processNoiseCov);

        // return statePre
        return statePre;
    };

    //#   - `statePost = kf.correct(measurement)` -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
    this.correct = function (measurement) {
        // temp2 = H * P'(k)
        temp2VV = measurementMatrix.multiply(errorCovPre);
        
        // temp3 = temp2 * Ht + R
        temp3VV = temp2VV.multiply(measurementMatrix.transpose()).plus(measurementNoiseCov);

        // temp4 = inv(temp3) * temp2 = Kt(k)
        itemp3VV = la.inverseSVD(temp3VV);
        temp4VV = itemp3VV.multiply(temp2VV);

        // K(k)
        gain = temp4VV.transpose();

        // temp2V = z(k) - H*x'(k)
        temp1V = measurementMatrix.multiply(statePre);
        temp2V = measurement.minus(temp1V);

        // x(k) = x'(k) + K(k) * temp2V
        temp1V = gain.multiply(temp2V);     
        statePost = statePre.plus(temp1V);

        // P(k) = P'(k) - K(k) * temp2
        errorCovPost = errorCovPre.minus(gain.multiply(temp2VV));

        // return statePost
        return statePost;
    };

};

/////////// Extended Kalman Filter
//#- `ekf = analytics.newExtendedKalmanFilter(dynamParams, measureParams, controlParams)` -- the Extended Kalman filter 
//#   is used with non-linear models, which are specified through transition and measurement equation. The initialization procedure
//#   requires specifying the model dimensions `dynamParams` (integer), measurement dimension `measureParams` (integer) and
//#   the `controlParams` control dimension. Algorithm works in two steps - prediction (short time prediction according to the
//#   specified model) and correction. The following functions are exposed:
exports.newExtendedKalmanFilter = function (dynamParams, measureParams, controlParams, parameterN) {
    return new analytics.extendedKalmanFilter(dynamParams, measureParams, controlParams, parameterN);
}
exports.extendedKalmanFilter = function (dynamParams, measureParams, controlParams, parameterN) {
    var CP = controlParams;
    var MP = measureParams;
    var DP = dynamParams;
    var P = parameterN;

    // CP should be >= 0
    CP = Math.max(CP, 0);

    var statePre = la.newVec({ "vals": DP, "mxvals": DP }); // prior state vector (after prediction and before measurement update)
    var statePost = la.newVec({ "vals": DP, "mxvals": DP }); // post state vector (after measurement update)
    var transitionMatrix = la.newMat({ "cols": DP, "rows": DP }); // transition matrix (model)

    var processNoiseCov = la.newMat({ "cols": DP, "rows": DP }); // process noise covariance
    var measurementMatrix = la.newMat({ "cols": DP, "rows": MP }); // measurement matrix
    var measurementNoiseCov = la.newMat({ "cols": MP, "rows": MP }); // measurement noise covariance

    var errorCovPre = la.newMat({ "cols": DP, "rows": DP }); // error covariance after prediction
    var errorCovPost = la.newMat({ "cols": DP, "rows": DP }); // error covariance after update
    var gain = la.newMat({ "cols": MP, "rows": DP });
    var parameterV = la.newVec({ "vals": P, "mxvals": P }); // parameters vector

    var controlMatrix;

    if (CP > 0)
        controlMatrix = la.newMat({ "cols": CP, "rows": DP }); // control matrix

    // temporary matrices used for calculation
    var temp1VV = la.newMat({ "cols": DP, "rows": DP });
    var temp2VV = la.newMat({ "cols": DP, "rows": MP });
    var temp3VV = la.newMat({ "cols": MP, "rows": MP });
    var itemp3VV = la.newMat({ "cols": MP, "rows": MP });
    var temp4VV = la.newMat({ "cols": DP, "rows": MP });

    var temp1V = la.newVec();
    var temp2V = la.newVec();

    // virtual functions
    // this.observationEq = function () { };
    // this.transitionEq = function () { };
    var observationEq;
    var transitionEq;

    //#   - `ekf.setTransitionEq(_val)` -- sets transition equation for EKF (`_val` is a function).
    this.setTransitionEq = function (_transitionEq) {
        this.transitionEq = _transitionEq;
    };

    //#   - `ekf.setObservationEq(_val)` -- sets observation equation for EKF (`_val` is a function).
    this.setObservationEq = function (_observationEq) {
        this.observationEq = _observationEq;
    };

    //#   - `ekf.setParameterV(_val)` -- sets parameter vector of size `parameterN`.
    this.setParameterV = function (_parameterV) {
        parameterV = _parameterV;
    };

    //#   - `ekf.getParameterV()` -- gets parameter vector.
    this.getParameterV = function () {
        return parameterV;
    };


    //#   - `ekf.setStatePost(_val)` -- sets the post state (DP) vector.
    this.setStatePost = function (_statePost) {
        statePost = _statePost;
    };

    //#   - `ekf.getStatePost()` -- returns the statePost vector.
    this.getStatePost = function () {
        return statePost;
    };

    //#   - `ekf.setTransitionMatrix(_val)` -- sets the transition (DP x DP) matrix.
    this.setTransitionMatrix = function (_transitionMatrix) {
        transitionMatrix = _transitionMatrix;
    };

    //#   - `ekf.setMeasurementMatrix(_val)` -- sets the measurement (MP x DP) matrix.
    this.setMeasurementMatrix = function (_measurementMatrix) {
        measurementMatrix = _measurementMatrix;
    };

    //#   - `ekf.setProcessNoiseCovPost(_val)` -- sets the process noise covariance (DP x DP) matrix.
    this.setProcessNoiseCov = function (_processNoiseCov) {
        processNoiseCov = _processNoiseCov;
    }

    //#   - `ekf.setMeasurementNoiseCov(_val)` -- sets the measurement noise covariance (MP x MP) matrix.
    this.setMeasurementNoiseCov = function (_measurementNoiseCov) {
        measurementNoiseCov = _measurementNoiseCov;
    }

    //#   - `ekf.setErrorCovPre(_val)` -- sets the pre error covariance (DP x DP) matrix.
    this.setErrorCovPre = function (_errorCovPre) {
        errorCovPre = _errorCovPre;
    }
    
    //#   - `ekf.setErrorCovPost(_val)` -- sets the post error covariance (DP x DP) matrix.
    this.setErrorCovPost = function (_errorCovPost) {
        errorCovPost = _errorCovPost;
    }

    //#   - `statePost = ekf.correct(measurement)` -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
    this.setControlMatrix = function (_controlMatrix) {
        controlMatrix = _controlMatrix;
    }

    //#   - `statePre = ekf.predict(control)` -- returns a predicted state vector `statePre` where `control` is the control vector (normally not set).
    this.predict = function (control) {
        // update the state: x'(k) = A * x(k)
        // Standard KF: statePre = transitionMatrix.multiply(statePost);
        statePre = this.transitionEq();

        // x'(k) = x'(k) + B * u(k)
        if (control.length) {
            temp1V = controlMatrix.multiply(control);
            temp2V = statePre.plus(temp1V);
        }

        // update error covariance matrices: temp1 = A * P(k)
        temp1VV = transitionMatrix.multiply(errorCovPost);

        // P'(k) = temp1 * At + Q
        errorCovPre = temp1VV.multiply(transitionMatrix.transpose()).plus(processNoiseCov);

        // return statePre
        return statePre;
    };

    //#   - `statePost = ekf.correct(measurement)` -- returns a corrected state vector `statePost` where `measurement` is the measurement vector.
    this.correct = function (measurement) {
        // temp2 = H * P'(k)
        temp2VV = measurementMatrix.multiply(errorCovPre);

        // temp3 = temp2 * Ht + R
        temp3VV = temp2VV.multiply(measurementMatrix.transpose()).plus(measurementNoiseCov);

        // temp4 = inv(temp3) * temp2 = Kt(k)
        itemp3VV = la.inverseSVD(temp3VV);
        temp4VV = itemp3VV.multiply(temp2VV);

        // K(k)
        gain = temp4VV.transpose();

        // temp2V = z(k) - H*x'(k)
        // standard KF: temp1V = measurementMatrix.multiply(statePre);
        temp1V = this.observationEq();
        temp2V = measurement.minus(temp1V);

        // x(k) = x'(k) + K(k) * temp2V
        temp1V = gain.multiply(temp2V);
        statePost = statePre.plus(temp1V);

        // P(k) = P'(k) - K(k) * temp2
        errorCovPost = errorCovPre.minus(gain.multiply(temp2VV));

        // return statePost
        return statePost;
    };

};
