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