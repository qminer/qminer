// creates the tests out of the examples in the documentation

hfile = process.argv[2]; // the path to the files (e.g. ./qminer/nodedoc/)
outfile = process.argv[3]; // the the folder where the tests will be saved (e.g. ./qminer/test/nodejs/)

fs = require('fs');

// get the .js files from the file array
if (hfile != 0) {
    var fileNames = fs.readdirSync(hfile);
}
var JSFiles = [];
for (var i = 0; i < fileNames.length; i++) {
    if (fileNames[i].indexOf(".js") != -1 && fileNames[i].indexOf(".html") == -1) {
        JSFiles.push(fileNames[i]);
    }
}

// constructs the it('should something', function () { ... }) test
var constructIt = function (str, i) {
    var test = "it('should make test number " + i + "', function () {\n this.timeout(10000); \n";
    test += str + "\n});\n";
    return test;
}

// for each .js file extract the examples
for (var i = 0; i < JSFiles.length; i++) {
    // prepared outfile for writing
    if (outfile != '') {
        fout = fs.createWriteStream(outfile + 'example' + JSFiles[i]);
    }

    // write the describe of the file (to know in which file the test throws the error)
    fout.write("describe('example tests for the " + JSFiles[i] + " file', function () {\n");

    var hstr = fs.readFileSync(hfile + JSFiles[i], 'ascii');
    // get the rows between * @example and @example or */ - getting the example code
    var regex = /(\/\*\*([\s\S]*?)\*\/)/g;
    var count = 0;
    while ((match = regex.exec(hstr)) != null) {
        str = match[0];

        if ((str.indexOf("IntVector") != -1 || str.indexOf("StrVector") != -1 || str.indexOf("BoolVector") != -1) && 
            (str.indexOf("cosine") != -1 || str.indexOf("sortPerm") != -1 || str.indexOf("outer") != -1)) {
            continue;
        }

        // for quicker search of the test
        title = str.slice(3);
        title = title.slice(title.indexOf('*') + 1);
        // get the description of the function
        describe = title.slice(1, title.indexOf('*') - 1);
        lastTab = describe.lastIndexOf("\t");
        if (lastTab != -1) {
            describe = describe.substring(0, lastTab);
        }
        lastNewline = describe.lastIndexOf("\n");
        if (lastNewline != -1) {
            describe = describe.substring(0, lastNewline - 1);
        }
        if (lastNewline == -1 && lastTab == -1) {
            describe = describe.substring(0, describe.length - 1);
        }
        // if it has the example
        var exampleIdx = str.indexOf('@example');
        if (exampleIdx != -1) {
            // write the description
            fout.write('describe("' + describe + '", function () {\n');
            // create the it block/test
			str = str.slice(exampleIdx);
			examples = str.split('@example');
			for (var ExpN = 0; ExpN < examples.length; ExpN++) {
				var example = examples[ExpN];
				//console.log(example);
				if (example == '') { continue; }
				if (example.indexOf('</caption>') != -1) { 
					example = example.slice(example.indexOf('</caption>') + 10);
				}
				count += 1;
				example = example.replace(/(\*)/g, '');
				if (example.indexOf('*/') != -1) { example = example.slice(0, example.length - 2);}
				else { example = example.slice(0, example.length - 1); }
				fout.write(constructIt(example, count));
			}
            // str = str.slice(exampleIdx);
            // str = str.replace(/(\*)/g, '');
            // str = str.slice(8, str.length - 2);
            // fout.write(constructIt(str, count));
            
			// end the description block
            fout.write("});\n");
        }
    }
    fout.write("\n});\n");
}