///////////////////////////////////////////////////////////
// used for the resampler chart

var socket = io();
var gaussVal, gaussTime;
socket.on('getMergerGauss', function (data) {
    gaussVal = data.gaussVal;
    gaussTime = data.gaussTime;
});
var otherGaussVal, otherGaussTime;
socket.on('getMergerOtherGauss', function (data) {
    otherGaussVal = data.otherGaussVal;
    otherGaussTime = data.otherGaussTime;
});

var firstGauss, secondGauss, mergerTime;
socket.on('getMerger', function (data) {
    firstGauss = data.firstGauss;
    secondGauss = data.secondGauss;
    mergerTime = data.mergerTime;
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
                        previousGauss = 0,
                        previousOtherGauss = 0,
                        date;
                    setInterval(function () {
                        if (previousGauss != gaussTime) {
                 
                            date = (new Date(gaussTime)).getTime();
                            seriesGauss.addPoint([date, gaussVal], false, (counterGauss >= maxPointsGauss));
                            previousGauss = gaussTime;
                            counterGauss++;
                        }
                        if (previousOtherGauss != otherGaussTime) {
                            date = (new Date(otherGaussTime)).getTime();
                            seriesOtherGauss.addPoint([date, otherGaussVal], true, (counterOtherGauss >= maxPointsGauss));
                            previousOtherGauss = otherGaussTime;
                            counterOtherGauss++;
                        }
                    }, 1000);

                }
            }
        },
        title: {
            text: 'The Guassian chart'
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
                    var seriesGaussFirst = this.series[0],
                        seriesGaussSecond = this.series[1],
                        maxPointsMerger = 10,
                        counterMerger = 0,
                        date;
                    setInterval(function () {
                        if (firstGauss.length > 0) {
                            for (var i = 0; i < firstGauss.length; i++) {
                                date = (new Date(mergerTime[i])).getTime();
                                seriesGaussFirst.addPoint([date, firstGauss[i]], false, (counterMerger >= maxPointsMerger));
                                seriesGaussSecond.addPoint([date, secondGauss[i]], true, (counterMerger >= maxPointsMerger));
                                counterMerger++;
                            }
                        }
                    }, 1000);

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
                        maxPointsMerger = 14,
                        counterMerger = 0,
                        counterGauss = 0,
                        counterOtherGauss = 0,
                        previousGauss = 0,
                        previousOtherGauss = 0,
                        date;
                    setInterval(function () {

                        if (previousGauss != gaussTime) {
                            date = (new Date(gaussTime)).getTime();
                            seriesGaussFirst.addPoint([date, gaussVal], false, (counterGauss >= maxPointsGauss));
                            previousGauss = gaussTime;
                            counterGauss++;
                        }
                        if (previousOtherGauss != otherGaussTime) {
                            date = (new Date(otherGaussTime)).getTime();
                            seriesGaussSecond.addPoint([date, otherGaussVal], true, (counterOtherGauss >= maxPointsGauss));
                            previousOtherGauss = otherGaussTime;
                            counterOtherGauss++;
                        }

                        if (firstGauss.length > 0) {
                            for (var i = 0; i < firstGauss.length; i++) {
                                date = (new Date(mergerTime[i])).getTime();
                                firstMerger.addPoint([date, firstGauss[i]], false, (counterMerger >= maxPointsMerger));
                                secondMerger.addPoint([date, secondGauss[i]], true, (counterMerger >= maxPointsMerger));
                                counterMerger++;
                            }
                        }
                    }, 1000);

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