module.exports = exports = require('bindings')('snap.node');
var fs = require('bindings')('fs.node');

exports.UndirectedGraph.prototype.draw = function (fnm) {	
	drawGraph(fnm, this), "templateGraphDraw.html";
}

exports.DirectedGraph.prototype.draw = function (fnm) {	
	drawGraph(fnm, this, "templateDGraphDraw.html");
}

exports.DirectedMultigraph.prototype.draw = function (fnm) {	
	drawGraph(fnm, this, "templateDGraphDraw.html");
}

function drawGraph(fnm, graph, template) {
	var br = 0;
	var json_out = {};
    var json_out_edges = new Array();
	
	graph.eachEdge(function(E){
		var id1 = E.srcId;
        var id2 = E.dstId;
		json_out_edges.push({ source: id1, target: id2 });
        br++;
	});
    br = 0;
    var json_out_data = {};
	
	graph.eachNode(function(N){
		var id = N.id;
		var size_var = N.deg;
		json_out_data[id] = { size: size_var };
		br++;
	});

    json_out["edges"] = json_out_edges;
    json_out["data"] = json_out_data;
	
    var template = fs.openRead(template).readAll();
    var output = template.replace("{{{data}}}", JSON.stringify(json_out));
    fs.openWrite(fnm).write(output).close();
	
	return json_out;
}