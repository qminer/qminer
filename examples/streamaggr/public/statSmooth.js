///////////////////////////////////////////////////////////
// Used for the stat/smooth chart

var socket = io();
var stat, smooth, timeStat;
socket.on('getStats', function (data) {
    stat = data.stat;
    smooth = data.smooth;
    timeStat = data.timeStat;
});

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
                    setInterval(function () {
                        date = (new Date(timeStat)).getTime();
                        seriesOrigin.addPoint([date, stat], false, (count >= maxSamples));
                        seriesSmooth.addPoint([date, smooth], true, (count >= maxSamples));
                        count++;
                    }, 1000);
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