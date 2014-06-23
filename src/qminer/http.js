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

//#- `onGet(path, function (request, response) { ... })`
http.onGet = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "GET", callback);
}

//#- `onPost(path, function (request, response) { ... })`
http.onPost = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "POST", callback);
}

//#- `onPut(path, function (request, response) { ... })`
http.onPut = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "PUT", callback);
}

//#- `onDelete(path, function (request, response) { ... })`
http.onDelete = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "DELETE", callback);
}

//#- `onPatch(path, function (request, response) { ... })`
http.onPatch = function (path, callback) {
    if (process.isArg("noserver")) { console.log("Warning: QMiner running with -noserver parameter, http callbacks will not be executed."); }
    http.onRequest(path, "PATCH", callback);
}

//#- `jsonp(request, response, data)` -- packaging reply as jsonp when callback parameter is provided in URL
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