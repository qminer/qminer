/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
var nodefs = require('fs');
var util = require('util');

// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'.
// Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';

    var qm = require('bindings')(pathPrefix + '/qm.node');
    var fs = qm.fs;

    exports = qm;
    
    //==================================================================
    // STORE
    //==================================================================

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

    exports.Store.prototype.addStreamAggr = function (params) {
        var newParams = params; newParams.store = this.name;
        return new exports.StreamAggr(this.base, newParams, this.name);
    }

    exports.Store.prototype.inspect = function (depth) {
        var d = (depth == null) ? 0 : depth;
        return util.inspect(this, { depth: d, 'customInspect': false });
    }

    //==================================================================
    // RECORD SET
    //==================================================================

    /**
     * Stores the record set as a CSV file.
     * 
     * @param {Object} opts - arguments
     * @property {String} opts.fname - name of the output file
     * @property {Boolean} [opts.includeHeaders] - indicates wether to include the header in the first line
     * @property {String} [opts.timestampType] - If set to 'ISO', datetime fields will be printed as ISO dates, otherwise as timestamps. Defaults to 'timestamp'.
     */
    exports.RecSet.prototype.saveCsv = function (opts) {
    	if (opts == null || opts.fname == null) throw new Error('Missing parameter fname!');
    	if (opts.includeHeaders == null) opts.includeHeaders = true;
    	if (opts.timestampType == null) opts.timestampType = 'timestamp';
    	
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
    			headerLine += fieldDesc[i].name;
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
	    				line += '"' + fldVal + '"';
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
    * @param {Object} [param] - Constructor parameters
    * @param {module:qm.Store} param.store - Store for the records in the buffer.
    * @param {number} param.size - Size of the buffer (number of records).
    * @param {function} [param.onAdd] - Callback executed when new record is
    * added to the buffer. Callback is give two parameters: record and instance of CircularRecordBuffer.
    * @param {function} [param.onDelete] - Callback executed when record is removed
    * from the buffer. Callback is give two parameters: record and instance of CircularRecordBuffer.
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
        * @param {module:fs.FIn} fin - input stream
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
        * @param {module:fs.FOut} fout - output stream
        * @returns {module:fs.FOut} output stream
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

    //!ENDJSDOC

    return exports;
}
