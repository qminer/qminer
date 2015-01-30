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
// HTTP 

//#- `http.onGet(path, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onGet("test", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onGet = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "GET", callback);
}

//#- `http.onPost(path, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onPost("test", function (req, resp) { })` executed from `script.js` on localhost will execute a post request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPost = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "POST", callback);
}

//#- `http.onPut(path, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onPut("test", function (req, resp) { })` executed from `script.js` on localhost will execute a put request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPut = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "PUT", callback);
}

//#- `http.onDelete(path, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onDelete("test", function (req, resp) { })` executed from `script.js` on localhost will execute a delete request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onDelete = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "DELETE", callback);
}

//#- `http.onPatch(path, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onPatch("test", function (req, resp) { })` executed from `script.js` on localhost will execute a patch request from `http://localhost/script/test`. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
http.onPatch = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "PATCH", callback);
}

//#- `http.jsonp(httpRequest, httpResponse, dataJSON)` -- packaging reply as jsonp when callback parameter is provided in URL
http.jsonp = function (req, res, data) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    // convert to string
    var dataStr;
    try {
        dataStr = JSON.stringify(data)
    } catch (err) {
        res.setStatusCode(500);
        res.setContentType("text/plain");
        res.send("error stringifying");
    }
    // return
    res.setStatusCode(200);
    res.setContentType("application/json");
    if (req.args.callback) {           
        res.send(req.args.callback + "(" + dataStr + ");");
    } else {
        res.send(dataStr);
    }
}