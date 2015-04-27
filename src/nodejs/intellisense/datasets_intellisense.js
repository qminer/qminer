/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
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

    
