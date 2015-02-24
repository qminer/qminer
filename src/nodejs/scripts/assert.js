module.exports = exports = require('assert');

exports.run = function (test, msg) {
    return;
}

exports.exists = function (obj, msg) {
    exports.ok(obj != null, msg);
}

exports.eqtol = function (flt1, flt2, tol, msg) {
    tol = typeof tol !== 'undefined' ? tol : Math.pow(10, -10);
    var err = 'AssertionError: relative difference between ' + flt1 + ' and '
        + flt2 + ' is not within tol= ' + tol + '. ' + msg;

    var abs1 = Math.abs(flt1);
    var abs2 = Math.abs(flt2);
    var diff = Math.abs(flt1 - flt2);

    if (flt1 == flt2) {
        return;
    } else {
        exports.ok(diff / (abs1 + abs2) < tol,  err);
    }
}