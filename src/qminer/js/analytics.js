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

// wrapper around batch classification models
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

exports.loadBatchModel = function (sin) {
    var models = JSON.parse(sin.readLine());
    var featureSpace = exports.loadFeatureSpace(sin);
    for (var cat in models) {        
        models[cat].model = exports.loadSvmModel(sin);
    }
    // we finished the constructor
    return new createBatchModel(featureSpace, models);    
}
