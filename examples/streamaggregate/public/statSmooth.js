///////////////////////////////////////////////////////////
// Used for the stat/smooth chart

var socket = io();

$(function () {
    $(document).ready(function () {
        Highcharts.setOptions({
            global: {
                useUTC: false
            }
        });
    });

    // the stat/smooth graphs
    $('#statSmooth').highcharts({
        chart: {
            type: 'line',
            animation: Highcharts.svg,
            marginRight: 10,
            events: {
                load: function () {
                    // set up the updating of the chart each second
                    var seriesOrigin = this.series[0],
                        seriesSmooth = this.series[1],
                        maxSamples = 10,
                        count = 0,
                        date;
                    // used sockets
                    socket.on('getStats', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesOrigin.addPoint([date, data.value], false, (count >= maxSamples));
                    });
                    socket.on('getAverage', function (data) {
                        date = (new Date(data.time)).getTime();
                        seriesSmooth.addPoint([date, data.value], true, (count >= maxSamples));
                        count++;
                    });
                }
            }
        },
        title: {
            text: 'Moving Average Example'
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
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        },
        {
            name: 'Smoothen Values',
            marker: {
                enable: true,
                symbol: 'circle',
                radius: 3,
            },
            data: []
        }]
    });
});