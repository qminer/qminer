module.exports = exports = require('bindings')('snap.node');
var fs = require('bindings')('fs.node');

exports.UndirectedGraph.prototype.draw = function (fnm) {	
	drawGraph(fnm, this, "templateGraphDraw.html");
}

exports.DirectedGraph.prototype.draw = function () {
	if (arguments.length == 0) {
		drawGraph('graph.html', this, 'templateDGraphDraw.html');
	}
	else if (arguments.length == 1) {
		drawGraph(arguments[0], this, 'templateDGraphDraw.html');
	}
	else if (arguments.length == 2) {
		drawGraph(arguments[0], this, 'templateDGraphDraw.html', arguments[1]);
	}
	else {
		drawGraph('graph.html', this, 'templateDGraphDraw.html');
	}
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
	
	if (arguments.length == 4) { //var labels = getLabels(arguments[3]);
		var labels = arguments[3];
		graph.eachNode(function(N){
			var id = N.id;
			var size_var = N.deg;
			if (N.deg > 0) {
				json_out_data[id] = { size: size_var, lbl: labels[id] };
				br++;
			}
		});
	}
	else {
		graph.eachNode(function(N){
			var id = N.id;
			var size_var = N.deg;
			if (N.deg > 0) {
				json_out_data[id] = { size: size_var, lbl: id };
				br++;
			}
		});
	}

    json_out["edges"] = json_out_edges;
    json_out["data"] = json_out_data;
	
    var template = fs.openRead(template).readAll();
    var output = template.replace("{{{data}}}", JSON.stringify(json_out));
    fs.openWrite(fnm).write(output).close();
	
	return json_out;
}