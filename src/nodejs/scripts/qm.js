var nodefs = require('fs');
var csv = require('fast-csv');

// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    exports = require('bindings')(pathPrefix + '/qm.node');

    var fs = require('bindings')(pathPrefix + '/fs.node');

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
        if (trigger.onUpdate != undefined) Callbacks["onUpdate"] = trigger.onUpdate;
        if (trigger.onDelete != undefined) Callbacks["onDelete"] = trigger.onDelete;
        var streamAggr = new exports.StreamAggr(this.base, Callbacks, this.name);
    }

    exports.Store.prototype.addStreamAggr = function (params) {
        // this == store instance: print //console.log(util.inspect(this, { colors: true })); 
        var streamAggr = new exports.StreamAggr(this.base, params, this.name);
    }
    
    //==================================================================
    // RECORD SET
    //==================================================================
    
    /**
     * Saves the record set into a CSV file specified in the opts parameter.
     * 
     * @param {object} opts - The options parameter contains 2 fields. The first field 'opts.fname' specifies the output file. The second field 'opts.headers' specifies if headers should be included in the output file.
     * @param {function} callback - The callback fired when the operation finishes.
     */
    exports.RecSet.prototype.saveCSV = function (opts, callback) {
    	// defaults
    	if (opts.headers == null) opts.headers = true;
    	
    	// write to file
    	try {
	    	console.log('Writing ' + this.length + ' lines to CSV file: ' + opts.fname + ' ...');
	
	    	var out = nodefs.createWriteStream(opts.fname);
	    	var csvOut = csv.createWriteStream({headers: opts.headers});
	    	
	    	out.on('error', function (e) {
	    		callback(e);
	    	});
	    	
	    	out.on('finish', function () {
	    		callback();
	    	});
	    	
	    	csvOut.pipe(out);
	    	
	    	this.each(function (rec, idx) {
	    		try {
		    		if (idx % 10000 == 0)
		    			console.log(idx);
		    		csvOut.write(rec.toJSON());
	    		} catch (e) {
	    			callback(e);
	    		}
	    	});
	    	
	    	csvOut.end();
    	} catch (e) {
    		callback(e);
    	}
    }
    
    //==================================================================
    // FEATURE SPACE
    //==================================================================
    
    //#- `qm.FeatureSpace.getSpFeatVecCols(spVec)` -- Return array of feature names based on feature space `fsp` where the elements of a sparse feature vector `spVec` are non-zero.
    exports.FeatureSpace.prototype.getSpFeatVecCols = function (spVec) {
        // get index and value vectors
        var valVec = spVec.valVec();
        var idxVec = spVec.idxVec();
        var cols = [];
        for (var elN = 0; elN < idxVec.length; elN++) {
            cols.push(this.getFtr(idxVec[elN]));
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
                    store.add(rec);
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

    return exports;
}