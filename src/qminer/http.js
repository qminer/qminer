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

// shortcuts
http.onGet = function (path, callback) {
    http.onRequest(path, "GET", callback);
}

http.onPost = function (path, callback) {
    http.onRequest(path, "POST", callback);
}

http.onPut = function (path, callback) {
    http.onRequest(path, "PUT", callback);
}

http.onDelete = function (path, callback) {
    http.onRequest(path, "DELETE", callback);
}

http.onPatch = function (path, callback) {
    http.onRequest(path, "PATCH", callback);
}

// packaging reply as jsonp when callback parameter is provided in URL
http.jsonp = function (req, res, data) {
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