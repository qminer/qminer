var fs = require('../../build/Debug/fs.node');

exports.addToQm = function (qm_mod) {
    qm_mod.store.prototype.addTrigger = function (trigger) {
        // this == store instance: print //console.log(util.inspect(this, { colors: true })); 
        // name is automatically generated
        // saveJson isn't needed    
        var Callbacks = {
            onAdd: trigger.onAdd,
            saveJson: function (limit) { return {}; }
        };
        if (trigger.onUpdate != undefined) Callbacks["onUpdate"] = trigger.onUpdate;
        if (trigger.onDelete != undefined) Callbacks["onDelete"] = trigger.onDelete;
        var streamAggr = new qm_mod.sa(this.base, Callbacks, this.name);        
    }

    // loading data into stores
    qm_mod.load = function () {
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
};

exports.addToAssert = function (assert) {
    // currently ignored
    //`assert.run(test, msg)` -- prints success if this code is reached
    assert.run = function (test, msg) {
        return;
    }
};

exports.addToProcess = function (proc) {
    proc.isArg = function (arg) {
        return process.argv.indexOf(arg) >= 0;
    }
};