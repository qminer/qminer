// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    exports = require('bindings')(pathPrefix + '/la.node');
        
    exports.SparseMatrix.prototype.frob2 = function () {
        return Math.pow(this.frob(), 2);
    }
    
    //#- `arr = la.copyVecToArray(vec)` -- copies vector `vec` into a JS array of numbers `arr`
    exports.copyVecToArray = function (vec) {
        var len = vec.length;
        var arr = [];
        for (var elN = 0; elN < len; elN++) {
            arr[elN] = vec[elN];
        }
        return arr;
    };

    return exports;
}

