process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')(); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')(); // pure native addon
exports.fs = require('bindings')('fs.node'); // pure native addon
exports.ht = require('bindings')('ht.node'); // pure native addon
exports.la = require('./src/nodejs/scripts/la.js')(); // includes additional JS implementations
exports.snap = require('bindings')('snap.node'); // pure native addon
exports.stat = require('bindings')('statistics.node'); // pure native addon

