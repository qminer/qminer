// replaces the doc header with QMiner JavaScript API v*

var hfile = process.argv[2]; // the path to the files (e.g. ./qminer/nodedoc/)

var fs = require('fs');
var qm = require('qminer'); // needed only for version

// get the .html files from the file array
var fileNames;
if (hfile !== 0) {
    fileNames = fs.readdirSync(hfile);
}
var htmlFiles = [];
for (var i = 0; i < fileNames.length; i++) {
    if (fileNames[i].indexOf(".js") === -1 && fileNames[i].indexOf(".html") !== -1) {
        htmlFiles.push(fileNames[i]);
    }
}

var fout = null;
for (var i = 0; i < htmlFiles.length; i++) {
    // read the file
    var fin = fs.readFileSync(hfile + htmlFiles[i], 'ascii');
    var pattern = /<a href=\"index.html\" class=\"jsdoc-navbar-package-name\">Home<\/a>/g;
    var changed = "<a href=\"index.html\" class=\"jsdoc-navbar-package-name\">QMiner JavaScript API v" + qm.version + "<\/a>";
    fin = fin.replace(pattern, changed);

    // create a write stream
    fout = fs.createWriteStream(hfile + htmlFiles[i]);
    fout.write(fin);
}
