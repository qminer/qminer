// import libraries
snap = require('snap.js');
viz = require('visualization.js');
utilities = require('utilities.js');

// Creating a new graph, adding nodes and edges

console.log("creating 1) undirected, 2) directed and  3) directe-multi graph");

// creating 1) undirected, 2) directed and  3) directe-multi graph

g1 = snap.newUGraph();
g2 = snap.newDGraph();
g3 = snap.newDMGraph();

//  adding nodes
g1.addNode(1); g2.addNode(1); g3.addNode(1);
g1.addNode(2); g2.addNode(2); g3.addNode(2);
g1.addNode(3); g2.addNode(3); g3.addNode(3);
g1.addNode(4); g2.addNode(4); g3.addNode(4);

// adding edges
g1.addEdge(1, 2); g2.addEdge(1, 2); g3.addEdge(1, 2);
g1.addEdge(1, 2); g2.addEdge(1, 2); g3.addEdge(1, 2);
g1.addEdge(2, 1); g2.addEdge(2, 1); g3.addEdge(2, 1);
g1.addEdge(1, 3); g2.addEdge(1, 3); g3.addEdge(1, 3);
g1.addEdge(1, 4); g2.addEdge(1, 4); g3.addEdge(1, 4);

// iterating graph and returning node ids, degree and degree centrality

// iteration can be executed using eachNode and eachEdge
console.log("UNDIRECTED GRAPH (g1):");
g1.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
g1.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

console.log("DIRECTED GRAPH (g2):");
g2.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
g2.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

console.log("DIRECTED-MULTIGRAPH (g3):");
g3.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
g3.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

var br = 0;
console.say("UNDIRECTED GRAPH (g1):");
for (var i = g1.firstNode ; br < g1.nodes; i.next()) {
    console.log("id: " + i.id + ", deg: " + i.deg + ", inDeg: " + i.inDeg + ", outDeg: " + i.outDeg);
    br++;
}
br = 0;
console.say("DIRECTED GRAPH (g2):");
for (var i = g2.firstNode ; br < g2.nodes; i.next()) {
    console.log("id: " + i.id + ", deg: " + i.deg + ", inDeg: " + i.inDeg + ", outDeg: " + i.outDeg);
    br++;
}
br = 0;
console.say("DIRECTED-MULTIGRAPH (g3):");
for (var i = g3.firstNode ; br < g3.nodes; i.next()) {
    console.log("id: " + i.id + ", deg: " + i.deg + ", inDeg: " + i.inDeg + ", outDeg: " + i.outDeg);
    br++;
}

// Reading graphs from files, drawing graphs, detecting communities, computing community evolution and plotting

// loading graphs
var g1999 = snap.newUGraph("./data/evo/1999.edg"); var g2000 = snap.newUGraph("./data/evo/2000.edg");
var g2001 = snap.newUGraph("./data/evo/2001.edg"); var g2002 = snap.newUGraph("./data/evo/2002.edg");
var g2003 = snap.newUGraph("./data/evo/2003.edg"); var g2004 = snap.newUGraph("./data/evo/2004.edg");
var g2005 = snap.newUGraph("./data/evo/2005.edg"); var g2006 = snap.newUGraph("./data/evo/2006.edg");

// storing graphs int an array
var graphs = new Array();
graphs.push(g1999); graphs.push(g2000);
graphs.push(g2001); graphs.push(g2002);
graphs.push(g2003); graphs.push(g2004);
graphs.push(g2005); graphs.push(g2006);


// determining communities for the array of graphs and storing the results in array of sparse vectors
var communities = new Array();
for (var i = 0; i < graphs.length; i++) {
    communities.push(snap.communityDetection(graphs[i], "gn"));
}

//console.log("Have a break");
//eval(breakpoint);

// return json string of graph evolution
var json = snap.evolutionJs(communities, 0.5, 0.75);

console.log("heyo");

// plot the community evolution graph
viz.drawCommunityEvolution(JSON.stringify(json), "./out/cmty_evolution.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });

// load a new graph from a file
console.log("Loading cobiss graph 1970-1975.edg");
var g = snap.newUGraph("./data/researchersBib_1970-1975.edg");
console.log("Done loading graph. N = " + g.nodes+ ", E = " + g.edges);
var g = snap.removeNodes(g, 3);

// detect communities using 2 different algorithms
console.log("Calculating communities using Clauset-Newman-Moore community detection method");
var CmtyCNM = snap.communityDetection(g, "cnm");
console.log("communities count: " + CmtyCNM.cols);

//console.log("Calculating communities using Info-map community detection method");
//var CmtyImap = snap.communityDetection(g, "imap");

// core periphery
var CP = snap.corePeriphery(g, "lip");

// draw the graph using two different colorings

viz.drawGraph(g, "./out/gCNM.html", { "color": CmtyCNM });
viz.drawGraph(g, "./out/gImap.html", { "color": CmtyImap });

// directed graphs

var dirGraph = snap.newDGraph("data\\dg1.txt");
//var dirGraph = snap.newDGraph("data\\graph.txt");


var t = utilities.newIntIntH();

fin = fs.openRead("data\\t1.txt");
//fin = fs.openRead("data\\times.txt");
while (!fin.eof) {
    var line = fin.readLine();
    var vals = line.split('\t');
    if (vals.length >= 2) {
        t.put(parseInt(vals[0]), parseFloat(vals[1]));
    }
}

// cmty graph
var cmtyGraph = snap.newDGraph("data\\graph.txt");


var cmtyt = utilities.newIntIntH();

fin = fs.openRead("data\\times.txt");
while (!fin.eof) {
    var line = fin.readLine();
    var vals = line.split('\t');
    if (vals.length >= 2) {
        cmtyt.put(parseInt(vals[0]), parseFloat(vals[1]));
    }
}

var dirJson = snap.directedJson(cmtyGraph, cmtyt);
viz.drawCommunityEvolution(dirJson, "out\\cmty_graph.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });


console.log("Done");
eval(breakpoint);

