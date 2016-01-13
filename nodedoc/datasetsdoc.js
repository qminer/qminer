
    /**
    * Datasets module includes some standard toy datasets. In addition, this module also includes various
    * random sample generators that can be used to build artificial datasets of controlled size and complexity.
    * @module datasets
    * @example <caption>Asynchronous loading</caption>
    * var qm = require('qminer');
    * var datasets = qm.datasets;
    *
    * // Create clean base
    * var base = new qm.Base({ mode: 'createClean' });
    *
    * // Load Iris dataset in async way
    * datasets.loadIris(base, function (err, store) {
    *     if (err) throw err;
    *     // Sucessfully loaded database
    *     base.close();
    * });
    *
    * @example <caption>Synchronous loading</caption>
    * var qm = require('qminer');
    * var datasets = qm.datasets;
    *
    * // Create clean base
    * var base = new qm.Base({ mode: 'createClean' });
    *
    * // Load Iris in sync way.
    * var Iris = datasets.loadIrisSync(base);
    * // Sucessfully loaded database
    * base.close();
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

        _base.loadCSV(options, function (err) {
            if (err) return callback(err);

            var Iris = _base.store(options.store);

            return callback(null, Iris)
        })
    }

    /**
    * Loads Iris dataset in synchronous way. Returns link to the created Iris store.
    * @param {module:qm.Base} base
    * @returns {module:qm.Store} Store with Iris dataset.
    */
    exports.loadIrisSync = function (_base) {

        var options = {
            file: __dirname + '/data/iris.csv',
            store: 'Iris',
            base: _base
        }

        _base.loadCSV(options);

        return _base.store(options.store);
    }

    // Module description
    exports.description = function () {
        return ("Module includes functions to load and make datasets.");
    }

    
