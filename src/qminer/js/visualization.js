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

override = require('json-override.js')
time = require("time");

// array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
// Input: [{ema : {Val : v1, Time : t1}, tick : {Val : v2, Time : t2}}, {ema : {Val : v3, Time : t3}, tick : {Val : v4, Time : t4}}]
// Output: [{name : "ema", data : [[t1, v1], [t3, v3]]} , {name : "tick", data : [[t2, v2], [t4, v4]] }]
//#- `objJson = vis.highchartsTSConverter(objJson)` -- array of multimeasurements to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
exports.highchartsTSConverter = function (dataJson) {
    var result = [];
    var temp = {};
    for (key in dataJson[0]) {
        temp[key] = [];
    }
    for (objN = 0; objN < dataJson.length; objN++) {
        var obj = dataJson[objN];
        for (key in obj) {
            var tm = time.parse(obj[key].Time);
            var longtime = 1000 * tm.timestamp + tm.millisecond;
            temp[key].push([longtime, obj[key].Val]);
        }
    }
    for (key in temp) {
        result.push({ name: key, data: temp[key] });
    }
    return result;
}

// record set JSON to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
// Input: {"$hits":432,"records":[{"$id":0,"datetime":"t1","mcutmp_avg":v11,"mcutmp_min":v12,"mcutmp_max":v13},{"$id":1,"datetime":"t2","mcutmp_avg":v21,"mcutmp_min":v22,"mcutmp_max":v23}]}
// Output: [{name : "mcutmp_avg", data : [[t1, v11], [t2, v21]]} , {name : "mcutmp_min", data : [[t1, v12], [t2, v22]] }, {name : "mcutmp_max", data : [[t1, v13], [t2, v23]] }]
//#- `objJson = vis.highchartsConverter(fieldsJson, objJson)` -- arecord set JSON to array of univariate time series. Input time stamps are strings. Output time stamps are milliseconds from 1970.
exports.highchartsConverter = function (fieldsJson, dataJson) {

    var keys = {};
    var datetime;
    for (key in fieldsJson) {        
        if (fieldsJson[key].type != "datetime") {
            keys[fieldsJson[key].name] = [];
            //console.log("" + fieldsJson[key].name);  
        } else datetime = fieldsJson[key].name;
    }
    //printj(keys)
    
    var result = [];    
    for (objN = 0; objN < dataJson.records.length; objN++) {
        var obj = dataJson.records[objN];
        for (key in obj) {
            var longtime;
            if (key == datetime){
                var tm = time.parse(obj[key]);            
                longtime = 1000 * tm.timestamp + tm.millisecond;  
            } else {                
                if (keys[key]) {                    
                    keys[key].push([longtime, obj[key]]);
                    //console.log(longtime + " " + JSON.stringify(obj[key]));
                }
            }                       
        }
    }
    for (key in keys) {
        //console.log(JSON.stringify(keys[key]));
        result.push({ name: key, data: keys[key] });
    }
    return result;
}

exports.highchartsParams = function () { 
    return {
        chart: {
                type: 'spline',
        },
        title: {
                text: 'spline chart'
        },
        subtitle: {
            text: 'multiple timeseries, unequally sampled'
        },
        xAxis: {
                type: 'datetime',
                dateTimeLabelFormats: { // don't display the dummy year
                month: '%e. %b',
                year: '%b'
                },
            title: {
                    text: 'Time'
            }
        },
        yAxis: {
                title: {
                    text: 'Val'
                },
            //min: 0
        },
        tooltip: {
                headerFormat: '<b>{series.name}</b><br>',
                pointFormat: '{point.x:%e. %b}: {point.y:.2f} '
        },
        //[{ "name": "tick", "data": [[1407427442309, 0.207912], [1407427443309, 0.309017], [1407427444309, 0.406737], [1407427445309, 0.5], [1407427446309, 0.587785], [1407427447309, 0.669131], [1407427448309, 0.743145], [1407427449309, 0.809017], [1407427450309, 0.866025], [1407427451309, 0.913545]] }, { "name": "js", "data": [[1407427442309, 5.207912], [1407427443309, 5.309017], [1407427444309, 5.406737], [1407427445309, 5.5], [1407427446309, 5.587785], [1407427447309, 5.669131], [1407427448309, 5.743145], [1407427449309, 5.809017], [1407427450309, 5.866025], [1407427451309, 5.913545]] }, { "name": "ema", "data": [[1407427442309, 0], [1407427443309, 0], [1407427444309, 0.005917], [1407427445309, 0.013315], [1407427446309, 0.022087], [1407427447309, 0.032111], [1407427448309, 0.043254], [1407427449309, 0.055368], [1407427450309, 0.068297], [1407427451309, 0.081876]] }]
        series: []
    }
}


//#- `vis.drawHighChartsTimeSeries(data, fileName, paramsJson)` -- copies the highCharts_ts.html template, injects JSON data, injects libraries, overrides the chart parameters if provided
exports.drawHighChartsTimeSeries = function(data, fnm, overrideParams) {
    var params = exports.highchartsParams();    
    if (typeof overrideParams != 'undefined') {
        params = override(params, overrideParams, false);
    }
    printj(params)
    params.series = data;
    
    var lib1 = fs.openRead(process.qminer_home + "gui/js/Highcharts/js/highcharts.js").readAll();
    var libs = lib1 + '\n' +  fs.openRead(process.qminer_home + "gui/js/Highcharts/js/modules/exporting.js").readAll();

    // read the correct template file
    var reader = fs.openRead(process.qminer_home + "gui/visualization_templates/highCharts_ts.html");
    var writer = fs.openWrite(fnm);
    // replace data
    while (!reader.eof) {
        var line = reader.readLine();
        line = line.replace("$PARAMS$", JSON.stringify(params));
        line = line.replace("$LIBS$", libs);
        writer.writeLine(line);
    }
    writer.close();
}
