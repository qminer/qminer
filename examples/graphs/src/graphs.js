// import libraries
snap = require('snap.js');
viz = require('visualization.js');
utilities = require('utilities.js');

// Creating a new graph, adding nodes and edges

// creating a new graph
gTest = snap.newUGraph();

//  adding nodes
gTest.addNode(0); gTest.addNode(1); gTest.addNode(2);
gTest.addNode(3); gTest.addNode(4); gTest.addNode(5);

// adding edges
gTest.addEdge(0, 1); gTest.addEdge(2, 1); gTest.addEdge(2, 4);
gTest.addEdge(4, 3); gTest.addEdge(5, 3); gTest.addEdge(4, 5);
gTest.addEdge(3, 1); gTest.addEdge(3, 2);

// iterating graph and returning node ids, degree and degree centrality
var br = 0;
for (var i = gTest.getFirstNode() ; br < gTest.nodeCount() ; i.getNext()) {
    console.log("id: " + i.getId() + ", deg: " + i.getDeg() + ", deg centrality: " + snap.DegreeCentrality(gTest, i.getId()));
    br++;
}

// Reading graphs from files, drawing graphs, detecting communities, computing community evolution and plotting

// loading graphs
var g1999 = snap.newUGraph("data\\evo\\1999.edg"); var g2000 = snap.newUGraph("data\\evo\\2000.edg");
var g2001 = snap.newUGraph("data\\evo\\2001.edg"); var g2002 = snap.newUGraph("data\\evo\\2002.edg");
var g2003 = snap.newUGraph("data\\evo\\2003.edg"); var g2004 = snap.newUGraph("data\\evo\\2004.edg");
var g2005 = snap.newUGraph("data\\evo\\2005.edg"); var g2006 = snap.newUGraph("data\\evo\\2006.edg");

// storing graphs int an array
var graphs = new Array();
graphs.push(g1999); graphs.push(g2000);
graphs.push(g2001); graphs.push(g2002);
graphs.push(g2003); graphs.push(g2004);
graphs.push(g2005); graphs.push(g2006);

// determining communities for the array of graphs and storing the results in array of sparse vectors
var communities = new Array();
for (var i = 0; i < graphs.length; i++) {
    communities.push(snap.CommunityDetection(graphs[i], "gn"));
}

// return json string of graph evolution
var json = snap.evolutionJs(communities, 0.5, 0.75);

// plot the community evolution graph
viz.drawCommunityEvolution(json, "out\\cmty_evolution.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });

// load a new graph from a file
console.log("Loading cobiss graph 1970-1975");
var g = snap.newUGraph("data\\researchersBib_1970-1975.edg");
console.log("Done loading graph. N = " + g.nodeCount() + ", E = " + g.edgeCount());

// detect communities using 2 different algorithms
console.log("Calculating communities using Clauset-Newman-Moore community detection method");
var CmtyCNM = snap.CommunityDetection(g, "cnm");
console.log("Calculating communities using Info-map community detection method");
var CmtyImap = snap.CommunityDetection(g, "imap");

// draw the graph using two different colorings
viz.drawGraph(g, "out\\gCNM.html", { "color": CmtyCNM });
viz.drawGraph(g, "out\\gImap.html", { "color": CmtyImap });

console.log("Done");
eval(breakpoint);

