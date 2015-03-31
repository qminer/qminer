var fnm = process.argv[2];
var foutnm = process.argv[3];
var str = process.argv[4];

fs = require('fs');
fileStr = fs.readFileSync(fnm, 'ascii');

fout = fs.createWriteStream(foutnm);
fout.write('//intellisense start\n');
fout.write(str + '\n');
fout.write('//intellisense end\n');
fout.write(fileStr);

fout.end();