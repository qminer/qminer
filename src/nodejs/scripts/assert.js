module.exports = exports = require('assert');

exports.run = function (test, msg) {
    return;
}

exports.exists = function (obj, msg) {
    exports.ok(obj != null, msg);
}