/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */


var pathQmBinary = __dirname + '/out/qm.node';
var foundBinary = require('fs').existsSync(pathQmBinary);

if (foundBinary) {
    // we're not in tonic environment
    process.env['QMINER_HOME'] = __dirname + "/src/glib/bin/";
    var pathQmBinary = __dirname + '/out/qm.node';

    module.exports = exports = require('./src/nodejs/scripts/qm.js')(pathQmBinary);
    exports.path = require('path').normalize(pathQmBinary);
    exports.version = require(__dirname + '/package.json').version;
    exports.analytics = require('./src/nodejs/scripts/analytics.js')(pathQmBinary);
    exports.fs = require('./src/nodejs/scripts/fs.js')(pathQmBinary);
    exports.ht = require('./src/nodejs/scripts/ht.js')(pathQmBinary);
    exports.la = require('./src/nodejs/scripts/la.js')(pathQmBinary);
    exports.snap = require('./src/nodejs/scripts/snap.js')(pathQmBinary);
    exports.statistics = require('./src/nodejs/scripts/statistics.js')(pathQmBinary);
    exports.datasets = require('./src/nodejs/datasets/datasets.js')(pathQmBinary);
    exports.deprecated = require('./src/nodejs/scripts/deprecated.js')(pathQmBinary);
    exports.qm_util = require('./src/nodejs/scripts/qm_util.js');

} else {
    throw new Error('qminer binary not found!');
}
