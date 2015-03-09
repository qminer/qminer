hfile = process.argv[2] || 'ht_nodejs.h';
jsfile = process.argv[3] || 'ht.js';
outfile = process.argv[4] || 'outi.js';

console.log('header: ' + hfile + ', javascript: ' + jsfile + ', output: ' + outfile);
fs = require('fs');

// hfile: keep looking for /** and //#
hstr = fs.readFileSync(hfile, 'ascii');

//  start with /** (\/\*\*) end with */ (\*\/) (multiple lines: [\s\S]*?) OR starts with //# (\/\/#)(in a single line)
var regex = /(\/\*\*([\s\S]*?)\*\/|\/\/#.*)/g;

fout = fs.createWriteStream(outfile)
while ((match = regex.exec(hstr))!= null) {
	//console.log(match[0]);
	str = match[0];
	if (str.indexOf('//#') != -1) {
	    str = str.slice(3);	
	}
	fout.write(str + '\r\n');	
}

// hfile: keep looking for /** and //#
jsstr = fs.readFileSync(jsfile, 'ascii');
//  start with /** (\/\*\*) end with */ (\*\/) (multiple lines: [\s\S]*?) OR starts with //# (\/\/#)(in a single line)
var regex = /(\/\/!STARTJSDOC)([\s\S]*?)(\/\/!ENDJSDOC)/;
match = regex.exec(jsstr)
fout.write(match[2] + '\r\n');	

fout.end();
