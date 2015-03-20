// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';

    // Import additional modules
    var qm = require('bindings')(pathPrefix + '/qm.node'); // A ne bo to nalozil samo funkcije iz c++?

    // Export function for loading famous Iris dataset
    exports.loadIris = function (_base, callback) {

        var options = {
            file: 'data/iris.csv',
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


    // Export function for loading famous Iris dataset
    exports.loadIrisSync = function (_base) {

        var finished = false;
        var options = {
            file: 'data/iris.csv',
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
    
    return exports;
}


// TODOL: Later do this with Iris.def 
//var storeDef = [{
//    "name": "Iris",
//    "fields": [
//         { "name": "SepalLength", "type": "float" },
//         { "name": "SepalWidth", "type": "float" },
//         { "name": "PetalLength", "type": "float" },
//         { "name": "PetalWidth", "type": "float" },
//         { "name": "Class", "type": "string" },
//    ]
//}];