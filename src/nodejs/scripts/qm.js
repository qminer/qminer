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
    
    exports = qm;

    //==================================================================
    // BASE
    //==================================================================
    
    /**
     * Loads the store from a CSV file. The opts parameter must have the following format:
     * 
     * {
     * 		file: 'nameOfFile',		// the name of the input file.
     * 		store: 'nameOfStore',	// name of the store which will be created
     * 		base: base,				// QMiner base object that creates the store
     * 		delimiter: ',',			// optional delimiter
     * 		quote: '"'				// optional character to escape values that contain a delimiter
     * }
     * 
     * @param {object} opts - options object, explained in the description
     * @param {function} [callback] - callback function, called on errors and when the procedure finishes
     */
    exports.Base.prototype.loadCSV = function (opts, callback) {
    	console.log('Loading CSV file ...');
    	
    	if (opts.delimiter == null) opts.delimiter = ',';
    	if (opts.quote == null) opts.quote = '"';
    	if (opts.ignoreFields == null) opts.ignoreFields = [];
    	
    	try {
    		var fname = opts.file;
    		var storeName = opts.store;
    		var base = opts.base;
    		
    		var fieldTypes = null;
    		var store = null;
    		var buff = [];
    		
    		var ignoreFields = {};
    		for (var i = 0; i < opts.ignoreFields.length; i++)
    			ignoreFields[opts.ignoreFields] = null;
    		
    		var csvOpts = {
    			headers: true,
    			ignoreEmpty: true,
    			delimiter: opts.delimiter,
    			quote: opts.quote
    		};
    		
    		// need to get the headers and columns types to actually create a store
    		function initFieldTypes(data) {
    			if (fieldTypes == null) fieldTypes = {};
    			
    			for (var key in data) {
//    				if (key in ignoreFields)
//    					continue;
    				
    				var val = data[key];
    				if (fieldTypes[key] == null) {
    					if (val.length == 0)
    						fieldTypes[key] = null;
    					else if (isNaN(val))
    						fieldTypes[key] = 'string';
    					else
    						fieldTypes[key] = 'float';
    						
    				}
    			}
    		}
    		
    		function fieldTypesInitialized() {
    			if (fieldTypes == null) return false;
    			
    			for (var key in fieldTypes) {
//    				if (key in ignoreFields)
//    					continue;
    				
    				if (fieldTypes[key] == null)
    					return false;
    			}
    			
    			return true;
    		}
    		
    		function getUninitializedFlds() {
    			var result = [];
    			
    			for (var key in fieldTypes) {
//    				if (key in ignoreFields)
//    					continue;
    				
    				if (fieldTypes[key] == null)
    					result.push(key);
    			}
    			
    			return result;
    		}
    		
    		function createStore(rec) {
    			try {
	    			var storeDef = {
	    				name: storeName,
	    				fields: []
	    			};
	    			
	    			for (var fieldName in rec) {
	    				storeDef.fields.push({
							name: fieldName,
							type: fieldTypes[fieldName],
							"null": true,
	    				});
	    			}
	    			
	    			base.createStore(storeDef);
	    			store = base.store(storeName);
	    			
	    			// insert all the record in the buffer into the store
	    			buff.forEach(function (data) {
	    				store.push(data);
	    			})
    			} catch (e) {
    				if (callback != null)
    					callback(e);
    			}
    		}
    		
    		var storeCreated = false;
    		var lines = 0;
    		
    		csv.fromPath(fname, csvOpts)
    			.transform(function (data) {
    				var transformed = {};
    				
    				for (var key in data) {
    					if (key in ignoreFields)
    						continue;
    					
    					var val = data[key];
    					var transKey = key.replace(/\s+/g, '_')	// remove invalid characters
    									  .replace(/\.|%|\(|\)|\/|-|\+/g, '');
    					
    					if (fieldTypes != null && fieldTypes[transKey] != null)
    						transformed[transKey] = fieldTypes[transKey] == 'float' ? parseFloat(val) : val;
    					else
    						transformed[transKey] = (isNaN(val) || val.length == 0) ? val : parseFloat(val);
    				}
    				
    				return transformed;
    			})
    		   	.on('data', function (data) {    		   		
    		   		if (++lines % 10000 == 0)
    		   			console.log(lines + '');
    			   
    		   		if (fieldTypes == null)
    		   			initFieldTypes(data);
    		   		
    		   		if (store == null && fieldTypesInitialized())
    		   			createStore(data);
    		   		else if (!fieldTypesInitialized())
    		   			initFieldTypes(data);
    		   		
    		   		if (store != null)
    		   			store.push(data);
    		   		else
    		   			buff.push(data);
    		   	})
    		   	.on('end', function () {
    		   		if (callback != null) {
    		   			if (!fieldTypesInitialized()) {
        		   			var fieldNames = getUninitializedFlds();
        		   			callback(new Error('Finished with uninitialized fields: ' + 
								JSON.stringify(fieldNames)) + ', add them to ignore list!');
        		   			return;
        		   		} else {
        		   			callback();
        		   		}
    		   		}
    		   	});   		
    	} catch (e) {
    		if (callback != null)
    			callback(e);
    	}
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
                        store.base.gc();
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