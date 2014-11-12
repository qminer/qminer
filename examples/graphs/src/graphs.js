// import libraries
snap = require('snap.js');
viz = require('visualization.js');
utilities = require('utilities.js');

// BASIC OPERATIONS
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

///
/// END - BASIC OPERATIONS


///
/// COMMUNITY EVOLUTION
///

// creating hash tables for community evolultion
var t = utilities.newIntIntH(); // time
var c = utilities.newIntIntH(); // community membership
var s = utilities.newIntIntH(); // community size
var e = la.newIntVec() // vector of edge weights (should be changed to triples or similar)
var m = la.newSpMat(); // community member ids from original graph
var graph = snap.newDGraph(); // directed graph of evolution

var gs = new Array();

// community evolution algorithm
snap.communityEvolution(gs, 0.5, 0.5, graph, t, c, s, e, m, "./data/evo/"); //


// hash table for evolution nodes (communities) text
var txtHash = utilities.newIntStrH();
var txt = utilities.newIntStrH();

// reading text data
fin = fs.openRead("data\\data.txt");
while (!fin.eof) {
    var line = fin.readLine();
    var vals = line.split('\t');
    var id = -1;
    var text = "";
    if (vals.length == 2) {
        id = parseInt(vals[0]);
        text = String(vals[1]);
        txtHash.put(id, text);
    }
}

// assigning text to communities based on the members
graph.eachNode(function (N) {
    var id = N.id;
    var someText = "";
    var spVec = m[id].idxVec();
    for (var i = 0; i < spVec.length; i++) {
        if (txtHash.hasKey(spVec[i])) {
            someText += txtHash.get(spVec[i]) + " ";
        }
    }
    txt.put(id, someText);
    
});

// drawing community evolution graph
var json_string = snap.evolutionJson(graph, t, c, s, e, txt);
var obj_out = eval("(" + json_string + ')');
viz.drawCommunityEvolution(JSON.stringify(obj_out), "./out/cmty.html", { title: { text: "Community evolution" } });

// END - Community evolution


//
// OTHER
//

// load a new graph from a file
console.log("Loading cobiss graph 1970-1975.edg");
var g = snap.newUGraph("./data/researchersBib_1970-1975.edg");
console.log("Done loading graph. N = " + g.nodes+ ", E = " + g.edges);
var g = snap.removeNodes(g, 3);

// detect communities
var CmtyCNM = snap.communityDetection(g, "cnm");

// draw the graph using two different colorings
viz.drawGraph(g, "./out/gCNM.html", { "color": CmtyCNM });
//viz.drawGraph(g, "./out/gCNM.html", {});


// END  - other

console.log("Done");
eval(breakpoint);