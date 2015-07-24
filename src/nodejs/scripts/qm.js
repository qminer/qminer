/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
var nodefs = require('fs');
var csv = require('fast-csv');
var util = require('util');


// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'.
// Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    
    var qm = require('bindings')(pathPrefix + '/qm.node');
    var fs = qm.fs;
    var qmutil = require(__dirname + '/qm_util.js');
    
    exports = qm;

    //==================================================================
    // BASE
    //==================================================================
    
    /**
     * Loads a store from a CSV file. The method expects the CSV file to have a header. 
     * 
     * The opts parameter must have the following format:
     * 
     * {
     * 		file: 'nameOfFile',		// the name of the input file.
     * 		store: 'nameOfStore',	// name of the store which will be created
     * 		delimiter: ',',			// optional delimiter
     * }
     * 
     * @param {object} opts - options object, explained in the description
     * @param {function} [callback] - callback function, called on errors and when the procedure finishes
     */
    exports.Base.prototype.loadCSV = function (opts, callback) {
    	console.log('Loading CSV file ...');
    	
    	if (opts.delimiter == null) opts.delimiter = ',';
    	if (opts.ignoreFields == null) opts.ignoreFields = [];
    	
		var storeName = opts.store;
		var base = this;
		
		var store = null;
		var fieldNames = null;
		var fieldTypes = null;
		var initialized = false;
		
		var buff = [];    		
		
		var ignoreFields = {};
		for (var i = 0; i < opts.ignoreFields.length; i++)
			ignoreFields[opts.ignoreFields] = null;
		
		function initStore() {
			console.log('Initializing store ...');
			
			// create the store
			var storeDef = {
				name: storeName,
				fields: []
			};
			
			for (var i = 0; i < fieldNames.length; i++) {
				var key = fieldNames[i];
				storeDef.fields.push({
					name: key,
					type: fieldTypes[key],
					'null': true
				});
			}
										
			base.createStore(storeDef);
			store = base.store(storeName);
		}
		
		function initFieldNames(header) {
			fieldNames = [];
			for (var i = 0; i < header.length; i++) {
				var key = header[i];
				
				if (key in ignoreFields) continue;
				
				var transKey = key.replace(/\s+/g, '_')	// remove invalid characters
				  				  .replace(/\.|%|\(|\)|\/|-|\+/g, '');
				fieldNames.push(transKey)
			}				
		}
		
		function typesInitialized() {
			if (fieldTypes == null) return false;
			if (initialized) return true;
			
			for (var i = 0; i < fieldNames.length; i++) {
				var key = fieldNames[i];
				if (fieldTypes[key] == null)
					return false;
			}
			
			initialized = true;
			return initialized;
		}
		
		function initTypes(data) {
			if (fieldTypes == null) fieldTypes = {};
							
			for (var key in data) {
				var val = data[key];
				
				if (fieldTypes[key] == null) {
					if (isNaN(val))
						fieldTypes[key] = 'string';
					else
						fieldTypes[key] = 'float';
				}
			}
		}
		
		var lines = 0;
		
		console.log('Reading CSV file ...');
		qmutil.readCsvLines({
			file: opts.file,
			delimiter: opts.delimiter,
			onLine: function (err, lineStrArr) {
    			// check for errors
    			if (err != null) {
    				callback(err);
    				return;
    			}
    			
    			if (++lines % 10000 == 0)
		   			console.log(lines + '');
    			
    			// header line
    			if (lines == 1)  {
    				initFieldNames(lineStrArr);
    			} else {
    				// put the data into a map
    				var data = {};
    				for (var i = 0; i < lineStrArr.length; i++) {
    					var key = fieldNames[i];
    					var val = lineStrArr[i];
    					
    					if (key in ignoreFields) continue;
    					
    					// if the value is empty ignore it
    					// and a null value will be inserted automatically
    					if (val.length > 0)
    						data[key] = val;
    				}
    				
    				// initialize types if you can
    				if (!typesInitialized())
    					initTypes(data);
    				
    				// cast the fields
    				for (var key in data) {
    					if (fieldTypes[key] == 'float')
    						data[key] = parseFloat(data[key]);
    				}

    				// add to buffer
    				buff.push(data);
    				
    				// if the store is not initialized, check if you can initialize it
    				if (store == null && typesInitialized())
    					initStore();
    				
    				// if the store is initialized => flush the buffer
    				if (store != null) {
    					while (buff.length > 0) {
    						store.push(buff.shift());
    					}
    				}
    			}
    		},
    		onEnd: function () {
        		if (callback != null)
        			callback(null, store);
    		}
		});
    }
    
    //==================================================================
    // STORE
    //==================================================================
    
    exports.Store.prototype.addTrigger = function (trigger) {
        // this == store instance: print //console.log(util.inspect(this, { colors: true })); 
        // name is automatically generated
        // saveJson isn't needed    
        var Callbacks = {
            onAdd: trigger.onAdd,
            saveJson: function (limit) { return {}; }
        };
        if (trigger.onUpdate != undefined) { Callbacks["onUpdate"] = trigger.onUpdate; }
        if (trigger.onDelete != undefined) { Callbacks["onDelete"] = trigger.onDelete; }
        var streamAggr = new exports.StreamAggr(this.base, Callbacks, this.name);
    }

    exports.Store.prototype.addStreamAggr = function (params) {
        // this == store instance: print //console.log(util.inspect(this, { colors: true })); 
        return new exports.StreamAggr(this.base, params, this.name);
    }
    
    exports.Store.prototype.inspect = function (depth) {        
        var d = (depth == null) ? 0 : depth;
        return util.inspect(this, { depth: d, 'customInspect': false });
    }
    
    //==================================================================
    // RECORD SET
    //==================================================================
    
    /**
     * Saves the record set into a CSV file specified in the opts parameter.
     * 
     * @param {object} opts - The options parameter contains 2 fields. 
	 *      The first field 'opts.fname' specifies the output file. 
	 *      The second field 'opts.headers' specifies if headers should be included in the output file.
     * @param {function} [callback] - The callback fired when the operation finishes.
     */
    exports.RecSet.prototype.saveCSV = function (opts, callback) {
    	// defaults
    	if (opts.headers == null) { opts.headers = true; }
    	
    	try {
    		console.log('Writing ' + this.length + ' lines to CSV file: ' + opts.fname + ' ...');
    		
    		// find out which columns to quote
    		var store = this.store;
    		var fields = store.fields;
    		
    		var quoteColumns = {};
    		for (var i = 0; i < fields.length; i++) {
    			var fldName = fields[i].name;
    			quoteColumns[fldName] = store.isString(fldName) || store.isDate(fldName);
    		}
	
	    	// write to file
	    	var out = nodefs.createWriteStream(opts.fname);
	    	var csvOut = csv.createWriteStream({
	    		headers: opts.headers,
	    		quoteHeaders: true,
	    		quoteColumns: quoteColumns
	    	});
	    	
	    	out.on('error', function (e) {
	    		if (callback != null) {
	    			callback(e);
				}
	    	});
	    	
	    	out.on('finish', function () {
	    		if (callback != null) {
	    			callback();
				}
	    	});
	    	
	    	csvOut.pipe(out);
	    	
	    	this.each(function (rec, idx) {
	    		try {
		    		if (idx % 10000 == 0) {
		    			console.log(idx);
					}
		    		csvOut.write(rec.toJSON());
	    		} catch (e) {
	    			if (callback != null) {
	    				callback(e);
					}
	    		}
	    	});
	    	
	    	csvOut.end();
    	} catch (e) {
    		if (callback != null) {
    			callback(e);
			}
    	}
    }
    
    //==================================================================
    // FEATURE SPACE
    //==================================================================
    
    //#- `qm.FeatureSpace.getSparseVectorFeatures(spVec)` -- Return array of feature 
	//#  names based on feature space `fsp` where the elements of a sparse feature
	//#  vector `spVec` are non-zero.
    exports.FeatureSpace.prototype.getSparseVectorFeatures = function (spVec) {
        // get index vector
        var idxVec = spVec.idxVec();
        var cols = [];
        for (var elN = 0; elN < idxVec.length; elN++) {
            cols.push(this.getFeature(idxVec[elN]));
        }
        return cols;
    }
    
    //==================================================================
    // EXPORTS
    //==================================================================

    // loading data into stores
    exports.load = function () {
        var _obj = {};

        //#- `num = qm.load.jsonFileLimit(store, fileName, limit)` -- load file `fileName` 
        //#   line by line, parsing each line as JSON and adding it as record to `store`.
        //#   When `limit != -1` only first first `limit` lines are loaded. Returns `num`:
        //#   the number of lines loaded.
        _obj.jsonFileLimit = function (store, file, limit) {
            var fin = fs.openRead(file);
            var count = 0;
            while (!fin.eof) {
                var line = fin.readLine();
                if (line == "") { continue; }
                try {
                    var rec = JSON.parse(line);
                    store.push(rec);
                    // count, GC and report
                    count++;
                    if (count % 1000 == 0) {
                        store.base.garbageCollect();
                    }
                    if (count % 10000 == 0) {
                        console.log("  " + count + " records");
                    }
                    if (count == limit) {
                        break;
                    }
                } catch (err) {
                    console.log("Error parsing [" + line + "]: " + err)
                }
            }
            console.log("Loaded " + count + " records to " + store.name);
            return count;
        }

        //#- `num = qm.load.jsonFile(store, fileName)` -- load file `fileName` line by line, 
        //#   parsing each line as JSON and adding it as record to `store`. Returns `num`:
        //#   the number of lines loaded.
        _obj.jsonFile = function (store, file) {
            return _obj.jsonFileLimit(store, file, -1);
        }

        return _obj;
    }();

    exports.delLock = function () {
        if (nodefs.existsSync('lock')) {
            try {
                nodefs.unlinkSync('lock');
            } catch (e) {
                console.log(e);
            }
        }
    }

    exports.rmDir = function (dirPath) {
        try { var files = nodefs.readdirSync(dirPath); }
        catch (e) { return; }
        if (files.length > 0)
            for (var i = 0; i < files.length; i++) {
                var filePath = dirPath + '/' + files[i];
                if (nodefs.statSync(filePath).isFile())
                    nodefs.unlinkSync(filePath);
                else
                    rmDir(filePath);
            }
        nodefs.rmdirSync(dirPath);
    };

	function forbidConstructor(obj) {
		proto = obj.prototype;
		obj = function () {throw  new Error('constructor is private, ' + obj.prototype.constructor.name +  ' is factory based.');}
		obj.prototype = proto;
		return obj;
	}

	// Forbids constructors that would crash node - these objects are factory constructed
	exports.Store = forbidConstructor(exports.Store);
	exports.RecSet = forbidConstructor(exports.RecSet);

    return exports;
}