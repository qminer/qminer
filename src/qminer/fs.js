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

//#- `json = fs.readJson(fileName)` - read file as a string and parse it top JSON
fs.readJson = function (fileName) {
    return JSON.parse(fs.readFile(fileName));
}

//#- `fs.writeJson(fileName, json)` - stringify `json` object and save it to file
fs.writeJson = function(fileName, json) {
    fs.openWrite(fileName).writeLine(JSON.stringify(json)).close();
}

//#- `fs.readJsonLines(fileName, callback)` - raed file line by line as string,
//#     parse each line to JSON and callback with JSON as only parameter
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