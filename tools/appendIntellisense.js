/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
var fnm = process.argv[2];
var foutnm = process.argv[3];
var str = process.argv[4];

fs = require('fs');
fileStr = fs.readFileSync(fnm, 'ascii');

fout = fs.createWriteStream(foutnm);
fout.write('//intellisense start\n');
str = str.replace(/\\n/g,'\n');
fout.write(str + '\n');
fout.write('//intellisense end\n');
fout.write(fileStr);

fout.end();