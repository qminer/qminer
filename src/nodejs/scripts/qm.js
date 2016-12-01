/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
var nodefs = require('fs');
var util = require('util');

module.exports = exports = function (pathQmBinary) {
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    var fs = qm.fs;
    exports = qm;

    //!STARTJSDOC

    //==================================================================
    // BASE
    //==================================================================

    /**
    * @typedef {object} BaseLoadCSVParam
    * The parameter given to {@link module:qm.Base#loadCSV}.
    * @property {string} file - The name of the input file.
    * @property {string} store - Name of the store which will be created.
    * @property {module:qm.Base} base - QMiner base object that creates the store.
    * @property {string} [delimiter = ','] - Optional delimiter.
    * @property {string} [quote = '"'] - Optional character to escape values that contain a delimiter.
    */

    /**
     * Loads the store from a CSV file.
     * @param {module:qm~BaseLoadCSVParam} opts - Options object.
     * @param {function} [callback] - Callback function, called on errors and when the procedure finishes.
     */
    exports.Base.prototype.loadCSV = function (opts, callback) {
    	// console.log('Loading CSV file ...');

    	if (opts.delimiter == null) opts.delimiter = ',';
    	if (opts.quote == null) opts.quote = '"';
    	if (opts.ignoreFields == null) opts.ignoreFields = [];
    	if (opts.file == null) throw new Error('Missing parameter file!');

    	if (callback == null) {
            callback = function (e) {
                if (e != null) {
                    // console.log(e.stack);
                }
            }
        }

    	try {
    		var base = this;

	    	var fname = opts.file;
			var storeName = opts.store;

			var fieldTypes = null;
			var store = null;
			var buff = [];

			var ignoreFields = {};
			for (var i = 0; i < opts.ignoreFields.length; i++)
				ignoreFields[opts.ignoreFields] = null;

			// read the CSV file and fill the store
			var headers = null;

			function transformLine(line) {
				var transformed = {};

				for (var i = 0; i < line.length; i++) {
					var header = headers[i];
					var value = line[i];

					if (fieldTypes != null && fieldTypes[header] != null) {
						transformed[header] = fieldTypes[header] == 'float' ? parseFloat(value) : value;
					} else {
						transformed[header] = (isNaN(value) || value.length == 0) ? value : parseFloat(value);
					}
				}

				return transformed;
			}

    		function initFieldTypes(data) {
    			if (fieldTypes == null) fieldTypes = {};

    			for (var key in data) {
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

    			if (fieldTypesInitialized()) {
    				// console.log('Fields initialized: ' + JSON.stringify(fieldTypes));
                }
    		}

    		function fieldTypesInitialized() {
    			if (fieldTypes == null) return false;

    			for (var key in fieldTypes) {
    				if (fieldTypes[key] == null)
    					return false;
    			}

    			return true;
    		}

			function getUninitializedFlds() {
    			var result = [];

    			for (var key in fieldTypes) {
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

	    			// console.log('Creating store with definition ' + JSON.stringify(storeDef) + ' ...');

	    			base.createStore(storeDef);
	    			store = base.store(storeName);

	    			// insert all the record in the buffer into the store
	    			buff.forEach(function (data) {
	    				store.push(data);
	    			});
    			} catch (e) {
					callback(e);
    			}
    		}

			var storeCreated = false;
			var line = 0;
			// console.log('Saving CSV to store ' + storeName + ' ' + fname + ' ...');

			var fin = new fs.FIn(fname);
			fs.readCsvLines(fin, {
				onLine: function (lineArr) {
					try {
						if (line++ == 0) {	// the first line are the headers
							headers = [];
							for (var i = 0; i < lineArr.length; i++) {
								headers.push(lineArr[i].replace(/\s+/g, '_').replace(/\.|%|\(|\)|\/|-|\+/g, '')) 	// remove invalid characters
							}
							// console.log('Headers initialized: ' + JSON.stringify(headers));
						}
						else {
							if (line % 1000 == 0) {
								// console.log(line + '');
                            }
                            
							var data = transformLine(lineArr);

							if (fieldTypes == null)
								initFieldTypes(data);

							if (store == null && fieldTypesInitialized())
								createStore(data);
							else if (!fieldTypesInitialized())
								initFieldTypes(data);

							if (store != null) {
								store.push(data);
							} else
								buff.push(data);
						}
					} catch (e) {
						// console.log('Exception while reading CSV lines: ' + e.stack);
						callback(e);
					}
				},
				onEnd: function () {
					// finished
					// console.log('Finished!');

					if (callback != null) {
			   			if (!fieldTypesInitialized()) {
				   			var fieldNames = getUninitializedFlds();
				   			callback(new Error('Finished with uninitialized fields: ' +
								JSON.stringify(fieldNames)) + ', add them to ignore list!');
				   			return;
				   		} else {
				   			callback(undefined, store);
				   		}
			   		}
				}
			});

			fin.close();
    	} catch (e) {
			callback(e);
    	}
    };

    //==================================================================
    // STORE
    //==================================================================

    /**
     * Adds a stream aggregate to a store.
     * @param {function} trigger - The trigger containing the method {@link module:qm.StreamAggr#onAdd} and optional
     * {@link module:qm.StreamAggr#onUpdate} and {@link module:qm.StreamAggr#onDelete}.
     */
    exports.Store.prototype.addTrigger = function (trigger) {
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

    /**
     * Adds a stream aggregate to the store. For use example see {@link module:qm.StreamAggr} constructor example.
     * @param {(module:qm~StreamAggregator | function)} arg - Constructor arguments. There are two argument types:
     * <br>1. Using the {@link module:qm~StreamAggregator} object,
     * <br>2. using a function/JavaScript class. The function has defined The object containing the schema of the stream aggregate or the function object defining the operations of the stream aggregate.
     */ 
    exports.Store.prototype.addStreamAggr = function (params) {
        return new exports.StreamAggr(this.base, params, this.name);
    }

    /**
     * Inspects the stores.
     * @param {number} depth - The depth of inspection. How many times to recurse while formatting the store object.
     * @returns {string} String representation of the store.
     */
    exports.Store.prototype.inspect = function (depth) {
        var d = (depth == null) ? 0 : depth;
        return util.inspect(this, { depth: d, 'customInspect': false });
    }

    /**
     * Load given file line by line, parse each line to JSON and push it to the store.
     * @param {String} file - Name of the JSON line file.
     * @param {Number} [limit] - Maximal number of records to load from file.
     * @returns {number} Number of records loaded from file.
     */
    exports.Store.prototype.loadJson = function (file, limit) {
        var fin = fs.openRead(file);
        var count = 0;
        while (!fin.eof) {
            var line = fin.readLine();
            if (line == "") { continue; }
            try {
                var rec = JSON.parse(line);
                this.push(rec);
                // count, GC and report
                count++;
                if (limit != undefined && count == limit) { break; }
            } catch (err) {
                throw new Error("Error parsing line number: " + count + ", line content:[" + line + "]: " + err);                
            }
        }
        return count;
    }

    //==================================================================
    // RECORD SET
    //==================================================================

    /**
     * Stores the record set as a CSV file.
     *
     * @param {Object} opts - Arguments.
     * @property {String} opts.fname - Name of the output file.
     * @property {Boolean} [opts.includeHeaders = true] - Indicates wether to include the header in the first line.
     * @property {String} [opts.timestampType = 'timestamp'] - Date format. Possible options: 
     * <br>1. `'ISO'` - Datetime fields will be printed as ISO dates,
     * <br>2. `'timestamp'` - Datetime fields will be printed as timestamps.
     * @property {String} [opts.escapeChar = "] - Character which escapes quotes.
     */
    exports.RecSet.prototype.saveCsv = function (opts) {
    	if (opts == null || opts.fname == null) throw new Error('Missing parameter fname!');
    	if (opts.includeHeaders == null) opts.includeHeaders = true;
    	if (opts.timestampType == null) opts.timestampType = 'timestamp';
    	if (opts.escapeChar == null) opts.escapeChar = '"';

    	var escapeStr = opts.escapeChar + '"';
    	
    	// read field descriptions
    	var fields = this.store.fields;
    	var fieldDesc = [];
    	for (var i = 0; i < fields.length; i++) {
    		var desc = fields[i];
    		var type = desc.type;

    		if (type != 'float' && type != 'int' && type != 'bool' && type != 'datetime' &&
    				type != 'string')
    			throw new Error('Invalid field type: ' + type);
    		if (desc.internal) continue;

    		fieldDesc.push({name: desc.name, type: desc.type});
    	}

    	var nFields = fieldDesc.length;
    	var useTimestamp = opts.timestampType != 'ISO';

    	var fout = new fs.FOut(opts.fname);

    	// write the headers
    	if (opts.includeHeaders) {
    		var headerLine = '';
    		for (var i = 0; i < nFields; i++) {
    			headerLine += '"' + fieldDesc[i].name.replace(/"/g, escapeStr) + '"';
    			if (i < nFields - 1)
    				headerLine += ',';
    		}
    		fout.writeLine(headerLine);
    	}

    	// write the lines
    	var len = this.length;
    	var recN = 0;
    	this.each(function (rec) {
    		var line = '';
    		for (var i = 0; i < nFields; i++) {
    			var fldVal = rec[fieldDesc[i].name];
    			var type = fieldDesc[i].type;

    			if (fldVal != null) {
	    			if (type == 'float' || type == 'int' || type == 'bool') {
	    				line += fldVal;
	    			} else if (type == 'datetime') {
	    				line += useTimestamp ? fldVal.getTime() : fldVal.toISOString();
	    			} else if (type == 'string') {
	    				line += '"' + fldVal.replace(/"/g, escapeStr) + '"';
	    			} else {
	    				throw new Error('Invalid type of field: ' + type);
	    			}
    			}

    			if (i < nFields - 1)
    				line += ',';
    		}

    		if (recN++ < len - 1)
    			fout.writeLine(line);
    		else
    			fout.write(line);
    	});

    	fout.flush();
    	fout.close();
    }

    //==================================================================
    // CIRCULAR RECORD BUFFER
    //==================================================================

    /**
    * @classdesc Circular buffer for storing records. Size of buffer is defined at
    * start and is denoted in number of records. When buffer is full, old records
    * are removed from the buffer and new records are stored in their place. For
    * adding and deleting a callback is called. Records are stored by their IDs.
    * @class
    * @param {Object} [params] - Constructor parameters.
    * @param {module:qm.Store} param.store - Store for the records in the buffer.
    * @param {number} param.size - Size of the buffer (number of records).
    * @param {function} [param.onAdd] - Callback executed when new record is
    * added to the buffer. Callback is give two parameters: 
    * <br>`rec` - The record. Type {@link module:qm.Record}.
    * <br>`circRecBuff` - The circular record buffer instance. Type {@link module:qm.CircularRecordBuffer}.
    * @param {function} [param.onDelete] - Callback executed when record is removed
    * from the buffer. Callback is give two parameters:
    * <br>`rec` - The record. Type {@link module:qm.Record}.
    * <br>`circRecBuff` - The circular record buffer instance. Type {@link module:qm.CircularRecordBuffer}.
    * @example
	* // TODO
    */
    exports.CircularRecordBuffer = function (params) {
        // check we have all encessary parameters
        if (params.store == undefined) { throw new Error("CircularRecordBuffer requires store in constructor"); }
        if (!(params.store instanceof qm.Store)) { throw new Error("CircularRecordBuffer requires store in constructor" + params.store); }
        if (params.size == undefined) { throw new Error("CircularRecordBuffer requires size in constructor"); }
        if (!(params.size >= 1)) { throw new Error("CircularRecordBuffer positive size in constructor"); }
        // parameters
        this.store = params.store;
        this.size = params.size;
        this.buffer = new qm.la.IntVector();
        this.next = 0;
        // Callbacks
        this.onAdd = (params.onAdd == undefined) ? function () {} : params.onAdd;
        this.onDelete = (params.onDelete == undefined) ? function () {} : params.onDelete;

        /**
        * Load circular buffer from input stream. Assumes store, onAdd and onDelete
        * were already initialized in constructor.
        * @param {module:fs.FIn} fin - input stream.
        * @example 
        * // TODO
        */
        this.load = function (fin) {
            var finParam = fin.readJson();
            this.size = finParam.size;
            this.next = finParam.next;
            this.buffer.load(fin);
        }

        /**
        * Saves circular buffer to the output stream. Does not save store, onAdd
        * and onDelete callbacks.
        * @param {module:fs.FOut} fout - Output stream.
        * @returns {module:fs.FOut} The output stream `fout`.
        * @example
        * // TODO
        */
        this.save = function (fout) {
            fout.writeJson({
                size: this.size,
                next: this.next
            });
            this.buffer.save(fout);
            return fout;
        }

        /**
    	* Add new record to the buffer.
        * @param {module:qm.Record} rec - New record.
        * @example
        * // TODO
    	*/
        this.push = function (rec) {
            if (this.buffer.length < this.size) {
                // we did not fill buffer yet, just add new element
                this.buffer.push(rec.$id);
                this.onAdd(rec, this);
            } else {
                // we are full, first delete the oldest record.
                var oldRec = this.store[this.buffer[this.next]];
                this.onDelete(oldRec, this);
                // remember new record
                this.buffer[this.next] = rec.$id;
                this.onAdd(rec, this);
                // move pointer to the oldest record forwards
                this.next++;
                if (this.next == this.size) { this.next = 0; }
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

    // deprecated, here for backwards compatibility
    exports.load = function () {
        var _obj = {};
        _obj.jsonFileLimit = function (store, file, limit) {
            return store.loadJson(file, limit);
        }
        _obj.jsonFile = function (store, file) {
            return store.loadJson(file);
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

    //!ENDJSDOC

    return exports;
}
