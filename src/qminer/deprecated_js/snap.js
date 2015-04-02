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

var util = require("utilities.js");

module.exports = require("__snap__");
exports = module.exports; // re-establish link

// class for storing community sizes
function CmtySizes(CommunityId, Size) {
    this.CommunityId = CommunityId;
    this.Size = Size;
}
// class for storing community assignements
function CmtyC(NodeId, CommunityId) {
    this.NodeId = NodeId;
    this.CommunityId = CommunityId;
}
// class for community distribution statistics
function TransformationStat(IdPrev, Id, PrevP, CurrP) {
    this.IdPrev = IdPrev;
    this.Id = Id;
    this.PrevP = PrevP;
    this.CurrP = CurrP;
}

function getLabel(c, dist, prev_sizes, prev, alpha, beta, first_new_id) {
    // return value - initialized to first free id
    var returnName = first_new_id;
    // determine the size the current community
    var sizeT=0;
    for (var i = 0; i < dist.length; i++)
        sizeT += dist[i].Size;
    // TransformationStat - transformation statistics
    var stat = new Array();
    for (var i = 0; i < dist.length; i++) {
        if (dist[i].Size > 0) {
            var s = dist[i].Size;
            var c_1 = dist[i].CommunityId;
            for (var j = 0; j < prev_sizes.length; j++) {
                if (c_1 == prev_sizes[j].CommunityId) {
                    var sizeT_1 = prev_sizes[j].Size;
                    stat.push(new TransformationStat(c_1, c, s / sizeT_1, s / sizeT));
                }
            }
        }
    }

    // analyse the statistics - logic for determining label
    var count_curr_g_beta = new Array();
    var prev_g_alpha = new Array();
    for (var i = 0; i < stat.length; i++) {
        if (stat[i].PrevP > alpha && stat[i].CurrP > beta)
            prev_g_alpha.push(i);
    }
    if (prev_g_alpha.length == 1) {
        returnName = stat[prev_g_alpha[0]].IdPrev;
    }
    return returnName;
}

exports.directedJson = function (graph, t) {

    var edges = new Array();
    var nodes = new Array();

    graph.eachEdge(function (E) { 
        edges.push({n1:E.srcId, n2:E.dstId, w:2, t0:t.get(E.srcId), t1:t.get(E.dstId)});
    });

    graph.eachNode(function (N) { 
        nodes.push({id:N.id, size:2, t:t.get(N.id)});
    });

    var out = {};

    var out = { edges: edges, communities: nodes };

    return JSON.stringify(out);
};

//#- `JSON = snap.toJson(graph)` -- returns JSON object of graph with `source` and `target` attributes
exports.toJsonGraph = function (graph, opts) {
    var br = 0;
    var json_out = {};
    var json_out_edges = new Array();
    for (var i = graph.firstEdge; br < graph.edges ; i.next()) {
        var id1 = i.srcId;
        var id2 = i.dstId;
        json_out_edges.push({ source: id1, target: id2 });
        br++;
    }
    br = 0;
    // object
    var json_out_data = {};
    if (opts.color) {
   
        for (var k = 0; k < opts.color.cols; k++) {
            var column = opts.color[k];
            var idxvec = column.idxVec();
            for (var i = 0; i < idxvec.length; i++) {
                if (idxvec[i] != 0)
                    json_out_data[idxvec[i]] = { size: graph.node(idxvec[i]).deg, color: k };
            }
        }

    }
    else {
        for (var i = graph.firstNode ; br < graph.nodes ; i.next()) {
            var id = i.id;
            var size_var = i.deg;
            json_out_data[id] = { size: size_var };
            br++;
        }
    }
    json_out["edges"] = json_out_edges;
    json_out["data"] = json_out_data;
    return json_out;
};

//#- `JSON = snap.toJson(graph)` -- returns JSON object of array of graphs with `source` and `target` attributes
exports.toJsonGraphArray = function (data) {
    var json = "";
    var json_out = "";

    for (var j=0; j<data.length; j++) {
        json_out = "";
        var br = 0;
        for (var i = data[j].firstEdge() ; br < data[j].edges ; i.next()) {
            var n1 = i.srcId;
            var n2 = i.dstId;
            json_out += ",{\"source\":" + n1 + ",\"target\":" + n2 + "}\n";
            br++;
        }
        json_out = "\n[" + json_out.substr(1, json_out.length - 1) + "]";

        if (j < data.length - 1)
            json_out += ",\n";

        json += json_out;
    }
    json = "\n[" + json + "]";
    var obj_out = eval("(" + json + ')');
    return obj_out;
};


//#- `graph = snap.removeNodes(graph, n)` -- removes nodes with degree up to n
exports.removeNodes = function (graph, n) {
    var br = 0;
    var c = graph.nodes;
    var toDelete = new Array();
    for (var i = graph.firstNode ; br < c ; i.next()) {
        if (i.deg <= n)
            toDelete.push(i.id);
        br++;
    }

    for (var i = 0; i < toDelete.length; i++)
        graph.delNode(toDelete[i]);

    return graph;
};

//#- `graph = snap.groupNodes(graph, n)` -- groups nodes by SpVec
exports.groupNodes = function (graph, data) {
    //  communities number
    columns = data.cols;
    // communities
    cKey = data.idxVec();

    var g = snap.newUGraph();
    for (var i = 0; i < columns; i++) {
        if (!g.isNode(i))
            g.addNode(i);
    }

    var br = 0;
    for (var i = graph.firstEdge() ; br < graph.edges ; i.next()) {
        var n1 = i.srcId;
        var n2 = i.dstId;
        var c1=-1;
        var c2=-1;

        for (var i = 0; i < columns; i++) {
            for (var j = 0; i < data[i].idxVec().length; j++) {
                if (data[i].at(j) == n1)
                    c1 = i;
                if (data[i].at(j) == n2)
                    c2 = i;
            }
        }

        g.addEdge(c1, c2);

        br++;
    }

    return g;
};
