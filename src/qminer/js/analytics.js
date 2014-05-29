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

exports = require("__analytics__");

function createBatchModel(featureSpace, models) {
    this.featureSpace = featureSpace;
    this.models = models;
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
    // prediction
    this.predict = function (record) {
        var vec = this.featureSpace.ftrSpVec(record);
        var result = { };
        for (var cat in this.models) {
            result[cat] = this.models[cat].model.predict(vec);
        }
        return result;
    }
    return this;
}

//#- `model = analytics.newBatchModel(records, features, target)` -- learns a new batch model
//#     using `records` as training data and `features` as feature space; `target` is
//#     a field for the records which we are trying to predict (e.g. store.field("Rating");
//#     if target field string or string vector, the result is a SVM classification model,
//#     and if target field is a float, the result is a SVM regression model; resulting 
//#     model has the following functions:
//#   - `result = model.predict(record)` -- creates feature vector from `record`, sends it
//#     through the model and returns the result as an array of scores.
//#   - `model.save(fout)` -- saves the model to `fout` output stream
exports.newBatchModel = function (records, features, target) {
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
        if (targets[cat].count >= 20) {
            models[cat] = {
                name : targets[cat].name,
                type : targets[cat].type,
            };
            if (targets[cat].type === "classification") {
                console.log("newBatchModel", "    ... " + cat + " (classification)");
                models[cat].model = exports.trainSvmClassify(sparseVecs, targets[cat].target);
            } else if (targets[cat].type === "regression") {
                console.log("newBatchModel", "    ... " + cat + " (regression)");
                models[cat].model = exports.trainSvmRegression(sparseVecs, targets[cat].target);
            }
        }
    }
    // done
    console.log("newBatchModel", "Done");  
    // we finished the constructor
    return new createBatchModel(featureSpace, models);
}

//#- `analytics.loadBatchModel(fin)` -- loads batch model frm input stream `fin`
exports.loadBatchModel = function (sin) {
    var models = JSON.parse(sin.readLine());
    var featureSpace = exports.loadFeatureSpace(sin);
    for (var cat in models) {        
        models[cat].model = exports.loadSvmModel(sin);
    }
    // we finished the constructor
    return new createBatchModel(featureSpace, models);    
}

// active learning (analytics:function activeLearner, parameters
//# 
exports.activeLearner = function (ftrSpace, textField, recSet, nPos, nNeg, query) {
    var store = recSet.store;
    var X = linalg.newSpMat();
    var y = linalg.newVec();
    // QUERY MODE
    var queryMode = true;
    // bow similarity between query and training set 
    var queryRec = store.newRec({ textField: query }); // record
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
    var posIdxV = linalg.newIntVec(); //indices in recordSet
    var negIdxV = linalg.newIntVec(); //indices in recordSet
    var posIdV = linalg.newIntVec(); //record IDs
    var negIdV = linalg.newIntVec(); //record IDs
    var classVec = linalg.newVec({ "vals": recSet.length }); //svm scores for record set

    // returns record set index of the unlabeled record that is closest to the margin
    this.selectQuestion = function () {
        if (posIdV.length >= nPos && negIdV.length >= nNeg) { queryMode = false; }
        if (queryMode) {
            if (posIdV.length < nPos) {
                nPosQ = nPosQ + 1;
                console.say("query mode, try to get pos");
                this.getAnswer(simVp[simVp.length - 1 - (nPosQ - 1)]);
            }
            if (negIdV.length < nNeg) {
                nNegQ = nNegQ + 1;
                console.say("query mode, try to get neg");
                this.getAnswer(simVp[nNegQ - 1]);
            }
        }
        else {
            ////call svm, get record closest to the margin            
            //console.startx(function (x) { return eval(x); });
            svm = analytics.trainSvmClassify(X, y); //column examples, y float vector of +1/-1, default svm paramvals
            // mark positives
            for (var i = 0; i < posIdxV.length; i++) { classVec[posIdxV[i]] = Number.POSITIVE_INFINITY; }
            // mark negatives
            for (var i = 0; i < negIdxV.length; i++) { classVec[negIdxV[i]] = Number.POSITIVE_INFINITY; }
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
                }
            }
            var sorted = classVec.sortPerm();
            console.say("svm mode, margin: " + sorted.vec[0] + ", npos: " + posCount + ", nneg: " + negCount);
            this.getAnswer(sorted.perm[0]);
        }
    };
    // asks the user for class label given a record set index
    this.getAnswer = function (recSetIdx) {
        //todo options: ?newQuery        
        console.say(recSet[recSetIdx].Text + ": y/(n)/stop?");
        var ALanswer = console.getln();
        if (ALanswer !== "stop") {
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
            this.selectQuestion();
        }
        // +k query // rank unlabeled according to query, ask for k most similar
        // -k query // rank unlabeled according to query, ask for k least similar
    };
    this.saveSvmModel = function (outputStream) {
        // must be in SVM mode
        if (queryMode) {
            console.say("AL.save: Must be in svm mode");
            return;
        }
        svm.save(outputStream);
    };
    //this.saveLabeled
    //this.loadLabeled
};
