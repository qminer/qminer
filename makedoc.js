hfile = process.argv[2];// || './src/nodejs/ht/ht_nodejs.h';
jsfile = process.argv[3];// || './src/nodejs/scripts/ht.js';
outfile = process.argv[4];// || './nodedoc/htdoc.js';

docHead = process.argv[5];
viewArray = process.argv.slice(6);

console.log('header: ' + hfile + ', javascript: ' + jsfile + ', output: ' + outfile);
fs = require('fs');

// hfile: keep looking for /** and //#
hstr = fs.readFileSync(hfile, 'ascii');

//  start with /** (\/\*\*) end with */ (\*\/) (multiple lines: [\s\S]*?) OR starts with //# (\/\/#)(in a single line)
var regex = /(\/\*\*([\s\S]*?)\*\/|\/\/#.*)/g;

fout = fs.createWriteStream(outfile)
var headerStr = "";	
while ((match = regex.exec(hstr))!= null) {	
    str = match[0];
    // remove //# if found
	if (str.indexOf('//#') != -1) {
	    str = str.slice(3);	
	}
	headerStr += str + '\r\n';	
}

if (docHead != undefined && docHead != '') {
	var docHeadStr = fs.readFileSync(docHead, 'ascii');	
	fout.write(docHeadStr + '\r\n');
}

if (viewArray != undefined && viewArray.length > 0) {
    // use template 
    for (var i = 0; i < viewArray.length; i++) {
        var rendered = headerStr;

        var mustache = require('mustache')
        mustache.escape = function (value) { return value; }
        var view = require(viewArray[i]).view;
        rendered = '{{=<% %>=}}' + rendered + '<%={{ }}=%>';
        rendered = mustache.render(rendered, view);

        fout.write(rendered + '\r\n');
    }
} else {
    fout.write(headerStr);
}



if (jsfile != '') {
    jsstr = fs.readFileSync(jsfile, 'ascii');
    //  append code between //!STARTJSDOC and //!ENDJSDOC
    var regex = /(\/\/!STARTJSDOC)([\s\S]*?)(\/\/!ENDJSDOC)/;
    match = regex.exec(jsstr)
    fout.write(match[2] + '\r\n');

}
fout.end();
