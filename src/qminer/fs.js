// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

//////////////////////////////////////////
// FileSystem

//#- `data = fs.readFile(fileName)` - read file and return it as string
fs.readFile = function (fileName) {
    return fs.openRead(fileName).readAll();
};

//#- `fs.readLines(fileName, callback)` - raed file line by line as string and 
//#      calls the given callback with line as the only parameter
fs.readLines = function(fileName, callback) {
    var fin = fs.openRead(fileName);
    while (!fin.eof) {
        try {
            var line = fin.readLine();
            callback(line);
        } catch (err) {
            console.log(err);
        }
    }
}

//#- `fs.readLines(fileName, callback)` - raed file line by line as string, parses
//#     it into array and calls the given callback with array as the only parameter.
//#     Code is taken from http://stackoverflow.com/a/1293163
fs.readCsvLines = function(fileName, callback, strDelimiter) {
    // Check to see if the delimiter is defined. If not, then default to comma.
    strDelimiter = (strDelimiter || ",");
    // Create a regular expression to parse the CSV values.
    var objPattern = new RegExp((
            // Delimiters.
            "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +
            // Quoted fields.
            "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +
            // Standard fields.
            "([^\"\\" + strDelimiter + "\\r\\n]*))"
        ), "gi");
    
    var fin = fs.openRead(fileName);
    while (!fin.eof) {
        try {
            var line = fin.readLine();
            var arrData = [ ];
            // Create an array to hold our individual pattern matching groups.
            var arrMatches = null;
            // Keep looping over the regular expression matches until we can no longer find a match.
            while (arrMatches = objPattern.exec(line)){
                // Let's check to see which kind of value we captured (quoted or unquoted).
                var strMatchedValue = arrMatches[2] ?
                    // We found a quoted value. When we capture this value, unescape any double quotes.
                    arrMatches[2].replace(new RegExp( "\"\"", "g" ), "\"") :
                    // We found a non-quoted value.
                    arrMatches[3];
                // Now that we have our value string, let's add it to the data array.
                arrData.push(strMatchedValue);
            }
            // Return the parsed data.
            callback(arrData);
        } catch (err) {
            console.log(err);
        }
    }
}

//#- `fout = fs.writeCsvLine(fout, arr)` -- escape fields from array `arr` into CSV line and 
//#     write is to `fout` as string; returns fout. Not the most efficient thing in the world.
//#     NOT YET TESTED!
fs.writeCsvLine = function (fout, arr) {
    var line = '';
    for (var i = 0; i < arr.length; i++) {
        if (i > 0) { line += ','; }
        // secape quotes
        var val = (arr[i] + '').replace(/"/g, '""');
        // if we have comma inside, put whole field in quotes
        if (val.search(/("|,|\n)/g) >= 0) {
            line += '"' + val + '"';
        } else {
            line += val;
        }
    }
    fout.writeLine(line);
    return fout;
}

//#- `fout = fs.writeCsv(fout, arr)` -- calls `fs.writeCsvLine` for each element of `arr`
fs.writeCsv = function (fout, arr) {
    for (var i = 0; i < arr.length; i++) {
        fs.writeCsvLine(fout, arr[i]);
    }
    return fout;
}

//#- `json = fs.readJson(fileName)` - read file as a string and parse it top JSON
fs.readJson = function (fileName) {
    return JSON.parse(fs.readFile(fileName));
}

//#- `fs.writeJson(fileName, json)` - stringify `json` object and save it to file
fs.writeJson = function(fileName, json) {
    fs.openWrite(fileName).writeLine(JSON.stringify(json)).close();
}

//#- `fs.readJsonLines(fileName, callback)` - raed file line by line as string,
//#     parse each line to JSON and callback with JSON as the only parameter
fs.readJsonLines = function(fileName, callback) {
    var fin = fs.openRead(fileName);
    while (!fin.eof) {
        try {
            var line = fin.readLine();
            var json = JSON.parse(line);
            callback(json);
        } catch (err) {
            console.log(err);
        }
    }
}