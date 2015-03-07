// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
//    exports = require('bindings')(pathPrefix + '/analytics.node');
    var sget = require('sget');
    var qm = require('bindings')(pathPrefix + '/qm.node');
    var fs = qm.fs;
    
    exports = qm.analytics;

    var la = require(__dirname + '/la.js')(pathPrefix);
    var qm_util = require(__dirname + '/qm_util.js');

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




    //#- `alModel = analytics.newActiveLearner(query, qRecSet, fRecSet, ftrSpace, settings)` -- initializes the
    //#    active learning. The algorihm is run by calling `model.startLoop()`. The algorithm has two stages: query mode, where the algorithm suggests potential
    //#    positive and negative examples based on the query text, and SVM mode, where the algorithm keeps
    //#   selecting examples that are closest to the SVM margin (every time an example is labeled, the SVM
    //#   is retrained.
    //#   The inputs are: query (text), record set `qRecSet`, record set `fRecSet`,  the feature space `ftrSpace` and a 
    //#   `settings`JSON object. The settings object specifies:`textField` (string) which is the name
    //#    of the field in records that is used to create feature vectors, `nPos` (integer) and `nNeg` (integer) set the number of positive and negative
    //#    examples that have to be identified in the query mode before the program enters SVM mode.
    //#   We can set two additional parameters `querySampleSize` and `randomSampleSize` which specify the sizes of subsamples of qRecSet and fRecSet, where the rest of the data is ignored in the active learning.
    //#   Final parameters are all SVM parameters (c, j, batchSize, maxIterations, maxTime, minDiff, verbose).
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
            var queryRec = qRecSet.store.newRec(temp); // record
            querySpVec = ftrSpace.ftrSpVec(queryRec);
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
            uMat = ftrSpace.ftrSpColMat(uRecSet);

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


        //#   - `rs = alModel.getRecSet()` -- returns the record set that is being used (result of sampling)
        this.getRecSet = function () { return uRecSet };

        //#   - `idx = alModel.selectedQuestionIdx()` -- returns the index of the last selected question in alModel.getRecSet()
        this.selectedQuestionIdx = -1;

        //#   - `bool = alModel.getQueryMode()` -- returns true if in query mode, false otherwise (SVM mode)
        this.getQueryMode = function () { return queryMode; };

        //#   - `numArr = alModel.getPos(thresh)` -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
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

        //#   - `objJSON = alModel.getSettings()` -- returns the settings object
        this.getSettings = function () { return settings; }

        // returns record set index of the unlabeled record that is closest to the margin
        //#   - `recSetIdx = alModel.selectQuestion()` -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
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
        //#   - `alModel.getAnswer(ALAnswer, recSetIdx)` -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
        //#      The user input should be either "y" (indicating that recSet[recSetIdx] is a positive example), "n" (negative example).
        this.getAnswer = function (ALanswer, recSetIdx) {
            //todo options: ?newQuery
            if (ALanswer === "y") {
                posIdxV.push(recSetIdx);
                posRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.ftrSpVec(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(1.0);
            } else {
                negIdxV.push(recSetIdx);
                negRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.ftrSpVec(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(-1.0);
            }
            // +k query // rank unlabeled according to query, ask for k most similar
            // -k query // rank unlabeled according to query, ask for k least similar
        };
        //#   - `alModel.startLoop()` -- starts the active learning loop in console
        this.startLoop = function () {
            while (true) {
                var recSetIdx = this.selectQuestion();               
                var ALanswer = sget(uRecSet[recSetIdx].Text + ": y/(n)/s? Command s stops the process").trim();                
                if (ALanswer == "s") { break; }
                if (posIdxV.length + negIdxV.length == uRecSet.length) { break; }
                this.getAnswer(ALanswer, recSetIdx);
            }
        };
        //#   - `alModel.saveSvmModel(fout)` -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
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
	//#- `ridgeRegressionModel = new analytics.RidgeRegression(kappa, dim, buffer)` -- solves a regularized ridge
	//#  regression problem: min|X w - y|^2 + kappa |w|^2. The inputs to the algorithm are: `kappa`, the regularization parameter,
	//#  `dim` the dimension of the model and (optional) parameter `buffer` (integer) which specifies
	//#  the length of the window of tracked examples (useful in online mode). The model exposes the following functions:
	exports.RidgeRegression = function (kappa, dim, buffer) {
	    var X = [];
	    var y = [];
	    buffer = typeof buffer !== 'undefined' ? buffer : -1;
	    var w = new la.Vector({ "vals": dim });
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
	        var b = new la.Vector(y);
	        w = this.compute(A, b);
	    };
	    //#   - `vec = ridgeRegressionModel.getModel()` -- returns the parameter vector `vec` (dense vector)
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
	    
    
    /**
     * Hierarchical Markov model.
     */
    exports.HierarchMarkov = function (opts) {
    	// constructor
    	if (opts == null) throw 'Missing parameters!';
    	if (opts.base == null) throw 'Missing parameter base!';
    	
    	// create model and feature space
    	var mc;
    	var ftrSpace;
    	
    	if (opts.hmcConfig != null && opts.ftrSpaceConfig != null && opts.base != null) {
    		mc = opts.sequenceEndV != null ? new exports.HMC(opts.hmcConfig, opts.sequenceEndV) : new exports.HMC(opts.hmcConfig);
    		ftrSpace = new qm.FeatureSpace(opts.base, opts.ftrSpaceConfig);
    	} 
    	else if (opts.hmcFile != null && opts.ftrSpaceFile != null) {
    		mc = new exports.HMC(opts.hmcFile);
    		ftrSpace = new qm.FeatureSpace(opts.base, opts.ftrSpaceFile);
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
    			ftrSpace.updateRecords(recSet);
    			
    			var colMat = ftrSpace.ftrColMat(recSet);
    			var timeV = recSet.getVec(timeField);
    			
    			log.info('Creating model ...');
    			mc.fit(colMat, timeV, batchEndV);
    			log.info('Done!');
    			
    			return that;
    		},
    		
    		/**
    		 * Adds a new record. Doesn't update the models statistics.
    		 */
    		update: function (rec) {
    			var ftrVec = ftrSpace.ftrVec(rec);
    			var recTm = rec.time;
    			var timestamp = recTm.getTime();
    			
    			mc.update(ftrVec, timestamp);
    		},
    		
    		/**
    		 * Saves the feature space and model into the specified files.
    		 */
    		save: function (mcFName, ftrFname) {
    			log.info('Saving Markov chain ...');
    			mc.save(mcFName);
    			log.info('Saving feature space ...');
    			ftrSpace.save(ftrFname);
    			log.info('Done!');
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
    			return ftrSpace;
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
    			var names = [];
    			
    			var dims = ftrSpace.dims;
    			for (var i = 0; i < dims.length; i++) {
    				names.push(ftrSpace.getFtr(i));
    			}
    			
    			return names;
    		},
    		
    		/**
    		 * Returns state details as a Javascript object.
    		 */
    		stateDetails: function (stateId, level) {
    			var coords = mc.fullCoords(stateId);
    			var invCoords = ftrSpace.invFtrVec(coords);
    			var futureStates = mc.futureStates(level, stateId);
    			var pastStates = mc.pastStates(level, stateId);
    			var stateNm = mc.getStateName(stateId);
    			var wgts = mc.getStateWgtV(stateId);
    			
    			var ftrNames = that.getFtrNames();
    			var features = [];
    			for (var i = 0; i < invCoords.length; i++) {
    				features.push({name: ftrNames[i], value: invCoords.at(i)});
    			}
    			
    			return {
    				id: stateId,
    				name: stateNm.length > 0 ? stateNm : null,
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
    			
    			for (var i = 0; i < hist.binStartV.length; i++) {
    				hist.binStartV[i] = ftrSpace.invFtr(ftrIdx, hist.binStartV[i]);
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
    				var invFtrV = ftrSpace.invFtrVec(ftrV);
    				
    				var features = [];
    				for (var i = 0; i < invFtrV.length; i++) {
    					features.push({name: ftrSpace.getFtr(i), value: invFtrV.at(i)});
    				}
    				
    				callback(features);
    			});
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
    				var coords = ftrSpace.invFtrVec(mc.fullCoords(stateId));
    				
    				result.push({ state: stateId, value: coords[ftrIdx] });
    			}
    			
    			return result;
    		}
    	};
    	
    	return that;
    };

    return exports;
}