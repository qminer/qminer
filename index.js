process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')(); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')(); // includes additional JS implementations
exports.fs = require('bindings')('qm.node').fs; // pure native addon
exports.ht = require('bindings')('qm.node').ht; // pure native addon
exports.la = require('./src/nodejs/scripts/la.js')(); // includes additional JS implementations
exports.snap = require('./src/nodejs/scripts/snap.js')(); // includes additional JS implementations
exports.stat = require('bindings')('qm.node').statistics; // pure native addon