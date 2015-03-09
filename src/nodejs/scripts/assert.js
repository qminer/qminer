module.exports = exports = require('assert');

exports.run = function (test, msg) {
    return;
}

exports.exists = function (obj, msg) {
    exports.ok(obj != null, msg);
}

exports.eqtol = function (flt1, flt2, tol, msg) {
    tol = typeof tol !== 'undefined' ? tol : Math.pow(10, -10);
    msg = typeof msg !== 'undefined' ? msg : "";

    var err = 'relative difference between ' + flt1 + ' and '
        + flt2 + ' is not within tol= ' + tol + '. ' + msg;

    var diff = Math.abs(flt1 - flt2);

    exports.ok(diff < tol,  err);
}