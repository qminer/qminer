/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var q = require('qminer');

var dirName = process.argv[2];
var licName = process.argv[3];
var fExt = process.argv[4];

var licStr = q.fs.openRead(licName).readAll();

var files = q.fs.listFile(dirName, fExt, true);
for (var i = 0; i < files.length; i++) {
    var temp = licStr + q.fs.openRead(files[i]).readAll();
    q.fs.openWrite(files[i]).write(temp).close();
}
