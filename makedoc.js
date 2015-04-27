/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

hfile = process.argv[2];// || './src/nodejs/ht/ht_nodejs.h';
jsfile = process.argv[3];// || './src/nodejs/scripts/ht.js';
outfile = process.argv[4];// || './nodedoc/htdoc.js';

docHead = process.argv[5];
viewArray = process.argv.slice(6);

console.log('header: ' + hfile + ', javascript: ' + jsfile + ', output: ' + outfile);
fs = require('fs');

// hfile: keep looking for /** and //#
hstr = '';
if (hfile != '') {
	hstr = fs.readFileSync(hfile, 'ascii');
}

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
