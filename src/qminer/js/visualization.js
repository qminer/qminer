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

//#
//# ### visualization.js (use require)
//#
//# Functions for visualizing data
//# The library must be loaded using `var vis = require('visualization.js');`.
//# 
//# **Functions and properties:**
//#

// array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
// Input: [{ema : {Val : v1, Time : t1}, tick : {Val : v2, Time : t2}}, {ema : {Val : v3, Time : t3}, tick : {Val : v4, Time : t4}}]
// Output: [{name : "ema", data : [[t1, v1], [t3, v3]]} , {name : "tick", data : [[t2, v2], [t4, v4]] }]
//#- `objJson = vis.highchartsConverter(objJson)` -- array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
exports.highchartsConverter = function (dataJson) {

    var result = [];
    var temp = {};
    for (key in dataJson[0]) {
        temp[key] = [];
    }
    for (objN = 0; objN < dataJson.length; objN++) {
        var obj = dataJson[objN];
        for (key in obj) {
            var tm = time.parse(obj[key].Time);
            var longtime = 1000 * tm.timestamp + tm.milisecond;
            temp[key].push([longtime, obj[key].Val]);
        }
    }
    for (key in temp) {
        result.push({ name: key, data: temp[key] });
    }
    return result;
}

//#- `vis.drawHighChartsTimeSeries(data)` -- copies the highCharts_ts.html template, injects JSON data and injects libraries
exports.drawHighChartsTimeSeries = function(data, fnm, params) {

    var lib1 = fs.openRead(process.qminer_home + "gui/js/Highcharts/js/highcharts.js").readAll();
    var libs = lib1 + '\n' +  fs.openRead(process.qminer_home + "gui/js/Highcharts/js/modules/exporting.js").readAll();

    // read the correct template file
    var reader = fs.openRead(process.qminer_home + "gui/visualization_templates/highCharts_ts.html");
    var writer = fs.openWrite(fnm);
    // replace data
    while (!reader.eof) {
        var line = reader.readLine();
        line = line.replace("$DATA$", JSON.stringify(data));
        line = line.replace("$LIBS$", libs);
        writer.writeLine(line);
    }
    writer.close();
}