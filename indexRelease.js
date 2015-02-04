process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')('Release'); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')('Release'); // pure native addon
exports.fs = require('bindings')('Release/fs.node'); // pure native addon
exports.ht = require('bindings')('Release/ht.node'); // pure native addon
exports.la = require('./src/nodejs/scripts/la.js')('Release'); // includes additional JS implementations
exports.snap = require('bindings')('Release/snap.node'); // pure native addon
exports.stat = require('bindings')('Release/statistics.node'); // pure native addon

