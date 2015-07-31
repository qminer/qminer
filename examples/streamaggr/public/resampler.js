///////////////////////////////////////////////////////////
// used for the resampler chart

var socket = io();
var random, timeRes, resValue, resTime, number;
socket.on('getResampler', function (data) {
    random = data.random;
    timeRes = data.timeRes;
    resValue = data.resValue;
    resTime = data.resTime;
    number = data.number;
});

$(function () {
    $(document).ready(function () {
        Highcharts.setOptions({
            global: {
                useUTC: false
            }
        });
    });

    // the resampler graphs
    $('#resampler').highcharts({
        chart: {
            animation: Highcharts.svg,
            marginRight: 10,
            events: {
                load: function () {
                    // set up the updating of the chart each second
                    var seriesRandom = this.series[0],
                        seriesResampler = this.series[1],

                        maxPointsOrigin = 10,
                        counterOrigin = 0,
                        counterResampler = 0,
                        previous = 0,
                        date, value, ifFull;
                    setInterval(function () {
                        if (previous != timeRes) {
                            if (previous != 0 && resValue.length > 0) {
                                for (var i = 0; i < resValue.length; i++) {
                                    date = (new Date(resTime[i])).getTime();
                                    value = resValue[i];
                                    seriesResampler.addPoint([date, value], false, (counterResampler >= number));
                                    counterResampler++;
                                }
                            }
                            date = (new Date(timeRes)).getTime();
                            ifFull = (counterOrigin >= maxPointsOrigin);
                            seriesRandom.addPoint([date, random], true, ifFull);
                            previous = timeRes;
                            counterOrigin++;
                        }
                    }, 1000);

                }
            }
        },
        title: {
            text: 'Resampler chart'
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
            name: 'Brownian Motion',
            type: 'line',
            data: []
        },
        {
            name: 'Resampler Values',
            type: 'line',
            lineWidth: 0,
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 5,
            },
            data: []
        }
        ]
    });
});