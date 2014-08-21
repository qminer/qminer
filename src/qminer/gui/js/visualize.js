function isObjectAndNotArray(object) {
    return (typeof object === 'object' && !Array.isArray(object));
}

// 'createNew' defaults to false
function overwriteKeys(baseObject, overrideObject, createNew) {
    if (createNew) {
        baseObject = JSON.parse(JSON.stringify(baseObject));
    }
    if (typeof overrideObject != 'undefined') {
        Object.keys(overrideObject).forEach(function (key) {
            if (isObjectAndNotArray(baseObject[key]) && isObjectAndNotArray(overrideObject[key])) {
                overwriteKeys(baseObject[key], overrideObject[key]);
            }
            else {
                baseObject[key] = overrideObject[key];
            }
        });
    }
    return baseObject;
}

visualize = {};
visualize.highchartsParams = function () {
    return {
        chart: {
            type: 'spline'
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

visualize.highCharts = function (data, containerName, overrideParams) {
    var defaultParams = visualize.highchartsParams();
    var params = overwriteKeys(defaultParams, overrideParams, true);
    params.series = data;
    $(function () {
        $('#' + containerName).highcharts(params);
    });
}
