module.exports = require('assert');
exports = module.exports; // re-establish link

exports.run = function (test, msg) {
    return;
}

exports.exists = function (obj, msg) {
    exports.ok(obj != null, msg);
}