///////////////////////////////////////////////////////////
// used for the merger chart

var socket = io();
var gauss = { val: 0, time: 0 };
socket.on('getGauss', function (data) {
    gauss.val = data.value;
    gauss.time = data.time;
});
var otherGauss = { val: 0, time: 0 };
socket.on('getOtherGauss', function (data) {
    otherGauss.val = data.value;
    otherGauss.time = data.time;
});

var merger = { first: 0, second: 0, time: 0 };
socket.on('getMerger', function (data) {
    merger.first = data.firstValue;
    merger.second = data.secondValue;
    merger.time = data.time;
});

$(function () {
    $(document).ready(function () {
        Highcharts.setOptions({
            global: {
                useUTC: false
            }
        });
    });

    // the gauss graphs
    $('#gauss').highcharts({
        chart: {
            animation: Highcharts.svg,
            marginRight: 10,
            events: {
                load: function () {
                    // set up the updating of the chart each second
                    var seriesGauss = this.series[0],
                        seriesOtherGauss = this.series[1],
                        maxPointsGauss = 10,
                        counterGauss = 0,
                        counterOtherGauss = 0,
                        date;
                    // use sockets
                    socket.on('getGauss', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesGauss.addPoint([date, data.value], true, (counterGauss >= maxPointsGauss));
                        counterGauss++;
                    });
                    socket.on('getOtherGauss', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesOtherGauss.addPoint([date, data.value], true, (counterOtherGauss >= maxPointsGauss));
                        counterOtherGauss++;
                    });
                }
            }
        },
        title: {
            text: 'The Gaussian chart'
        },
        xAxis: {
            type: 'datetime',
            tickPixelInterval: 150
        },
        yAxis: {
            title: {
                text: 'values'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            formatter: function () {
                return '<b>' + this.series.name + '<b><br/>' +
                    Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) + '<br/>' +
                    Highcharts.numberFormat(this.y, 4);
            }
        },
        legend: {
            enable: false
        },
        exporting: {
            enable: false
        },
        colors: ['#0000FF', '#FF0000'],
        series: [{
            name: 'First Gauss\' Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        },
        {
            name: 'Second Gauss\' Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        }
        ]
    });

    // the gauss graphs
    $('#merger').highcharts({
        chart: {
            animation: Highcharts.svg,
            marginRight: 10,
            events: {
                load: function () {
                    // set up the updating of the chart each second
                    var seriesFirstValues = this.series[0],
                        seriesSecondValues = this.series[1],
                        maxPointsMerger = 10,
                        counterMerger = 0,
                        date;
                    // use sockets
                    socket.on('getMerger', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesFirstValues.addPoint([date, data.firstValue], false, (counterMerger >= maxPointsMerger));
                        seriesSecondValues.addPoint([date, data.secondValue], true, (counterMerger >= maxPointsMerger));
                        counterMerger++;
                    });
                }
            }
        },
        title: {
            text: 'Merger Values Chart'
        },
        xAxis: {
            type: 'datetime',
            tickPixelInterval: 150
        },
        yAxis: {
            title: {
                text: 'values'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            formatter: function () {
                return '<b>' + this.series.name + '<b><br/>' +
                    Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) + '<br/>' +
                    Highcharts.numberFormat(this.y, 4);
            }
        },
        legend: {
            enable: false
        },
        exporting: {
            enable: false
        },
        colors: ['#0000FF', '#FF0000'],
        series: [{
            name: 'First Merger\'s Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        },
        {
            name: 'Second Merger\'s Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        }
        ]
    });


    // the gauss graphs
    $('#combinedMerger').highcharts({
        chart: {
            animation: Highcharts.svg,
            marginRight: 10,
            events: {
                load: function () {
                    // set up the updating of the chart each second
                    var seriesGaussFirst = this.series[0],
                        seriesGaussSecond = this.series[1],
                        firstMerger = this.series[2],
                        secondMerger = this.series[3],
                        maxPointsGauss = 10,
                        maxPointsMerger = 10,
                        counterMerger = 0,
                        counterGauss = 0,
                        counterOtherGauss = 0,
                        date;
                    // use sockets
                    socket.on('getGauss', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesGaussFirst.addPoint([date, data.value], true, (counterGauss >= maxPointsGauss));
                        counterGauss++;
                    });
                    socket.on('getOtherGauss', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesGaussSecond.addPoint([date, data.value], true, (counterOtherGauss >= maxPointsGauss));
                        counterOtherGauss++;
                    });
                    socket.on('getMerger', function (data) {
                        date = (new Date(data.time)).getTime();
                        firstMerger.addPoint([date, data.firstValue], false, (counterMerger >= maxPointsMerger));
                        secondMerger.addPoint([date, data.secondValue], true, (counterMerger >= maxPointsMerger));
                        counterMerger++;
                    });
                }
            }
        },
        title: {
            text: 'Combined Merger Chart'
        },
        xAxis: {
            type: 'datetime',
            tickPixelInterval: 150
        },
        yAxis: {
            title: {
                text: 'values'
            },
            plotLines: [{
                value: 0,
                width: 1,
                color: '#808080'
            }]
        },
        tooltip: {
            formatter: function () {
                return '<b>' + this.series.name + '<b><br/>' +
                    Highcharts.dateFormat('%Y-%m-%d %H:%M:%S', this.x) + '<br/>' +
                    Highcharts.numberFormat(this.y, 4);
            }
        },
        legend: {
            enable: false
        },
        exporting: {
            enable: false
        },
        colors: ['#4D4DFF', '#FF4D4D', '#0000FF', '#FF0000'],
        series: [{
            name: 'First Gauss\' Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        },
        {
            name: 'Second Gauss\' Values',
            type: 'line',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        },
        {
            name: 'First Merger\'s Values',
            type: 'line',
            lineWidth: 5,
            marker: {
                enabled: true,
                symbol: 'circle',
                radius: 5
            },
            data: []
        },
        {
            name: 'Second Merger\'s Values',
            type: 'line',
            lineWidth: 5,
            marker: {
                enabled: true,
                symbol: 'circle',
                radius: 5
            },
            data: []
        }
        ]
    });

});