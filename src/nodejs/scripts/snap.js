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

exports.DirectedGraph.prototype.drawJson = function (json) {
	if (arguments.length == 1) {
		drawGraphJson(this, json);
	}
	else if (arguments.length == 2) {
		drawGraphJson(this, json, arguments[1]);
	}
	else {
		drawGraphJson(this, json);
	}	
}

exports.DirectedGraph.prototype.drawTempJson = function (json) {
	if (arguments.length == 1) {
		drawGraphTempJson(this, json);
	}
	else if (arguments.length == 2) {
		drawGraphTempJson(this, json, arguments[1]);
	}
	else {
		drawGraphTempJson(this, json);
	}
	
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

function drawGraphTempJson(graph, json_out) {
	var br = 0;
	//var json_out = {};
    var json_out_edges = new Array();
	var json_out_data = {};
	
	graph.eachEdge(function(E){
		var id1 = E.srcId;
        var id2 = E.dstId;
		json_out_edges.push({ source: id1, target: id2 });
        br++;
	});
    br = 0;
    
	if (arguments.length == 3) { 
		var articles = arguments[2];
		graph.eachNode(function(N){
			var id = N.id;
			
			var article = articles[id];
			var ms = new Date(article.date+' '+article.time).getTime();
			
			var size_var = N.deg;
			if (N.deg > 0 && typeof article !== 'undefined') {
				json_out_data[id] = { size: size_var, lbl: id, outDeg: N.outDeg, inDeg: N.inDeg, ms: ms };
				br++;
			}
		});
	}
	else {
		graph.eachNode(function(N){
			var id = N.id;
			var size_var = N.outDeg+1;
			if (N.deg > 0) {
				json_out_data[id] = { size: size_var, lbl: id, outDeg: N.outDeg, inDeg: N.inDeg };
				br++;
			}
		});
	}

    json_out["edges"] = json_out_edges;
    json_out["data"] = json_out_data;

	return json_out;
}

function drawGraphJson(graph, json_out) {
	var br = 0;
	//var json_out = {};
    var json_out_edges = new Array();
	var json_out_data = {};
	
	graph.eachEdge(function(E){
		var id1 = E.srcId;
        var id2 = E.dstId;
		json_out_edges.push({ source: id1, target: id2 });
        br++;
	});
    br = 0;
    
	if (arguments.length == 3) { 
		var labels = arguments[2];
		graph.eachNode(function(N){
			var id = N.id;
			var publisher = labels[br];
			
			if (id != labels[br].id) {
				for (var i=0; i<labels.length; i++) {
					if (labels[i].id == id) {
						publisher = labels[i];
						break;
					}
				}
			}
			
			var size_var = N.outDeg+1;
			if (N.deg > 0 && typeof publisher !== 'undefined') {
				json_out_data[id] = { size: size_var, lbl: publisher.name };
				br++;
			}
		});
	}
	else {
		graph.eachNode(function(N){
			var id = N.id;
			var size_var = N.outDeg+1;
			if (N.deg > 0) {
				json_out_data[id] = { size: size_var, lbl: id};
				br++;
			}
		});
	}

    json_out["edges"] = json_out_edges;
    json_out["data"] = json_out_data;

	return json_out;
}