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
