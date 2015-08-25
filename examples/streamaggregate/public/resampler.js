///////////////////////////////////////////////////////////
// used for the resampler chart

var socket = io();

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
                        maxPointsResampler = 14,
                        counterOrigin = 0,
                        counterResampler = 0,
                        date;
                    // use sockets
                    socket.on('getResampler', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesResampler.addPoint([date, data.value], false, (counterResampler >= maxPointsResampler));
                        counterResampler++;
                    });
                    socket.on('getRandom', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesRandom.addPoint([date, data.value], true, (counterOrigin >= maxPointsOrigin));
                        counterOrigin++;
                    });

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