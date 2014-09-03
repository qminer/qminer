// import libraries
snap = require('snap.js');
viz = require('visualization.js');

// Some basic operations

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

// dump graph
gTest.dump();

// dump graph to a file
gTest.dump("gtest.txt");

console.log("end basic operations part");

// Reading graphs from files, detecting communities, computing community evolution and plotting

// loading graphs
var g1999 = snap.newUGraph("data\\1999.edg");
var g2000 = snap.newUGraph("data\\2000.edg");
var g2001 = snap.newUGraph("data\\2001.edg");
var g2002 = snap.newUGraph("data\\2002.edg");
var g2003 = snap.newUGraph("data\\2003.edg");
var g2004 = snap.newUGraph("data\\2004.edg");
var g2005 = snap.newUGraph("data\\2005.edg");
var g2006 = snap.newUGraph("data\\2006.edg");

// storing graphs int an array
var graphs = new Array();
graphs.push(g1999); graphs.push(g2000); graphs.push(g2001); graphs.push(g2002); graphs.push(g2003); graphs.push(g2004); graphs.push(g2005); graphs.push(g2006);

// determining communities for the array of graphs and storing the results in array of sparse vectors
var communities = new Array();
for (var i = 0; i < graphs.length; i++)
    communities.push(snap.CommunityDetection(graphs[i], "gn"));

// return json string of graph evolution
var json = snap.evolutionJs(communities, 0.5, 0.75);

// plot the evolution graph
viz.drawCommunityEvolution(json, "plot.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });


console.log("end loading graphs from files and community evolution plotting");

// Constructing graphs, detecting communities, computing evolution and plotting

g1 = snap.newUGraph();
g1.addNode(0); g1.addNode(1); g1.addNode(2);
g1.addNode(3); g1.addNode(4); g1.addNode(5);
g1.addEdge(0, 1); g1.addEdge(0, 2); g1.addEdge(2, 1);
g1.addEdge(4, 3); g1.addEdge(5, 3); g1.addEdge(4, 5);

g2 = snap.newUGraph();
g2.addNode(0); g2.addNode(1); g2.addNode(2);
g2.addNode(3); g2.addNode(4); g2.addNode(5);
g2.addEdge(0, 1); g2.addEdge(0, 2); g2.addEdge(2, 1);
g2.addEdge(4, 3); g2.addEdge(5, 3); g2.addEdge(4, 5);
g2.addEdge(0, 5);

g3 = snap.newUGraph(); g3.addNode(0); g3.addNode(1);
g3.addNode(2); g3.addNode(3); g3.addNode(4); g3.addNode(5);
g3.addEdge(0, 1); g3.addEdge(0, 2); g3.addEdge(2, 1);
g3.addEdge(4, 3); g3.addEdge(5, 3);
g3.addEdge(4, 5); g3.addEdge(0, 5);

g4 = snap.newUGraph();
g4.addNode(0); g4.addNode(1);
g4.addNode(2); g4.addNode(3); g4.addNode(4); g4.addNode(5);
g4.addNode(6); g4.addNode(7); g4.addNode(8);
g4.addEdge(0, 1); g4.addEdge(0, 2); g4.addEdge(2, 1);
g4.addEdge(4, 3); g4.addEdge(5, 3); g4.addEdge(4, 5);
g4.addEdge(0, 5); g4.addEdge(5, 6); g4.addEdge(5, 7);
g4.addEdge(5, 8);

g5 = snap.newUGraph();
g5.addNode(0); g5.addNode(1); g5.addNode(2); g5.addNode(3);
g5.addNode(4); g5.addNode(5); g5.addNode(6); g5.addNode(7);
g5.addNode(8); g5.addEdge(0, 1); g5.addEdge(2, 3);
g5.addEdge(4, 5); g5.addEdge(5, 3); g5.addEdge(4, 5);
g5.addEdge(0, 5); g5.addEdge(7, 6); g5.addEdge(5, 7);
g5.addEdge(4, 8);

var gs = new Array();
gs.push(g1); gs.push(g2); gs.push(g3); gs.push(g4); gs.push(g5);

// determining communities for the arrey of graphs storing the results (array of sparse vectors)
var cms = new Array();
for (var i = 0; i < gs.length; i++)
    cms.push(snap.CommunityDetection(gs[i], "gn"));

// return json string of evolution graph
var json_gs = snap.evolutionJs(cms, 0.5, 0.75);

// plot the evolution graph
viz.drawCommunityEvolution(json_gs, "plot_gs.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });

console.log("end graph construction and community evolution plotting");

// Returning Json string from c++ implementation for community evolution

var json_ce = snap.CommunityEvolution("ego_evo.edg");
// plot the evolution graph
viz.drawCommunityEvolution(json_ce, "plot_ce.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });

console.log("end the complete c++ implementation for community evolution");

eval(breakpoint);

