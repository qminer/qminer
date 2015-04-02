// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    exports = require('bindings')(pathPrefix + '/qm.node').snap;


    var fs = require('bindings')(pathPrefix + '/qm.node').fs;

    exports.UndirectedGraph.prototype.nodes3 = function () {
        return (this.nodes * 3);
    }

    exports.UndirectedGraph.prototype.draw = function (fnm) {
        drawGraph(fnm, this);
    }

    exports.DirectedGraph.prototype.draw = function (fnm) {
        drawGraph(fnm, this);
    }

    exports.DirectedMultigraph.prototype.draw = function (fnm) {
        drawGraph(fnm, this);
    }

    function drawGraph(fnm, graph) {
        var br = 0;
        var json_out = {};
        var json_out_edges = new Array();

        graph.eachEdge(function (E) {
            var id1 = E.srcId;
            var id2 = E.dstId;
            json_out_edges.push({ source: id1, target: id2 });
            br++;
        });
        br = 0;
        var json_out_data = {};

        graph.eachNode(function (N) {
            var id = N.id;
            var size_var = N.deg;
            json_out_data[id] = { size: size_var };
            br++;
        });

        json_out["edges"] = json_out_edges;
        json_out["data"] = json_out_data;

        var template = fs.openRead("templateGraphDraw.html").readAll();
        var output = template.replace("{{{data}}}", JSON.stringify(json_out));
        fs.openWrite(fnm).write(output).close();

        return json_out;
    }

    return exports;

}