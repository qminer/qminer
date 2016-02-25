
// renames the "Namespace" in the html file to "Factory class"

var htmlfile = process.argv[2]; // the html to rename 

//console.log('html: ' + htmlfile);
fs = require('fs');

var htmlstr = '';
if (htmlfile != '') {
    htmlstr = fs.readFileSync(htmlfile, 'ascii');
}

// replace the namespace into factory class
htmlstr = htmlstr.replace('new&nbsp;', '');

// write the new html
var fout = fs.createWriteStream(htmlfile);
fout.write(htmlstr);

fout.end();