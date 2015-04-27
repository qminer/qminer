/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
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