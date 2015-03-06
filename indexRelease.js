process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
module.exports = exports = require('./src/nodejs/scripts/qm.js')('Release'); // core functionality
exports.analytics = require('./src/nodejs/scripts/analytics.js')('Release'); // includes additional JS implementations
exports.fs = require('bindings')('Release/qm.node').fs; // pure native addon
exports.ht = require('bindings')('Release/qm.node').ht; // pure native addon
exports.la = require('./src/nodejs/scripts/la.js')('Release'); // includes additional JS implementations
exports.snap = require('./src/nodejs/scripts/snap.js')('Release'); // includes additional JS implementations
exports.stat = require('bindings')('Release/qm.node').statistics; // pure native addon