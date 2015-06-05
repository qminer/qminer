//intellisense start
exports = {}; require.modules.qminer_datasets = exports;
//intellisense end

    /**
    * Datasets module includes some standard toy datasets. In addition, this module also includes various
    * random sample generators that can be used to build artificial datasets of controlled size and complexity.
    * @module datasets 
    * @example <caption>Asynchronous loading</caption>
    * var qm = require('qminer');
    * var datasets = qm.datasets;
    * 
    * qm.delLock();
    * qm.config('qm.conf', true, 8080, 1024);
    *
    * var base = qm.create('qm.conf', '', true);
    *
    * // Load Iris dataset in async way
    * datasets.loadIris(base, function (err, store) {
    *     if (err) throw err;
    *     console.log("\nSucessfully loaded database: " + store.name)
    *     console.log(JSON.stringify(store.first, null, 2));
    * })
    * @example <caption>Synchronous loading</caption>
    * var qm = require('qminer');
    * var datasets = qm.datasets;
    * 
    * qm.delLock();
    * qm.config('qm.conf', true, 8080, 1024);
    *
    * var base = qm.create('qm.conf', '', true);
    *
    * // Load Iris in sync way.
    * var Iris = datasets.loadIrisSync(base);
    * console.log("\nSucessfully loaded database: " + Iris.name)
    * console.log(JSON.stringify(Iris.first, null, 2));
    */

    /**
    * Loads Iris dataset in asynchronous way. Returns link to the created Irsi store.
    * @param {module:qm.Base} base
    * @returns {module:qm.Store} Store with Iris dataset.
    */
    exports.loadIris = function (_base, callback) {

        var options = {
            file: __dirname + '/data/iris.csv',
            store: 'Iris',
            base: _base
        }

        console.log('Loading dataset ' + options.store + ' ...');
        _base.loadCSV(options, function (err) {
            if (err) return callback(err);

            var Iris = _base.store(options.store);
            console.log(options.store + ' dataset loaded')

            return callback(null, Iris)
        })
    }

    /**
    * Loads Iris dataset in synchronous way. Returns link to the created Iris store.
    * @param {module:qm.Base} base
    * @returns {module:qm.Store} Store with Iris dataset.
    */
    exports.loadIrisSync = function (_base) {

        var finished = false;
        var options = {
            file: __dirname + '/data/iris.csv',
            store: 'Iris',
            base: _base
        }

        console.log('Loading dataset ' + options.store + ' ...');
        _base.loadCSV(options, function (err) {
            if (err) return callback(err);
            
            var Iris = _base.store(options.store);
            console.log(options.store + ' dataset loaded')

            finished = true;
        })

        // Used deasync module to turn this function into sync. 
        while (finished == false) {
            require('deasync').runLoopOnce();
        }
        return _base.store(options.store);
    }
    
    // Module description
    exports.description = function () {
        return ("Module includes functions to load and make datasets.");
    }

    
