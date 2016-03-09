snap = require('../../index.js').snap;

console.log("creating 1) undirected, 2) directed and  3) directe-multi graph");

// creating 1) undirected, 2) directed and  3) directed-multi graph
g1 = new snap.UndirectedGraph;
g2 = new snap.DirectedGraph;
g3 = new snap.DirectedMultigraph;

//  adding nodes
g1.addNode(1); g2.addNode(1); g3.addNode(1);
g1.addNode(2); g2.addNode(2); g3.addNode(2);
g1.addNode(3); g2.addNode(3); g3.addNode(3);
g1.addNode(4); g2.addNode(4); g3.addNode(4);

// adding edges
g1.addEdge(1, 2); g2.addEdge(1, 2); g3.addEdge(1, 2);
g1.addEdge(1, 2); g2.addEdge(1, 2); g3.addEdge(1, 2);
g1.addEdge(2, 1); g2.addEdge(2, 3); g3.addEdge(2, 3);
g1.addEdge(4, 3); g2.addEdge(1, 3); g3.addEdge(1, 3);
g1.addEdge(3, 4); g2.addEdge(1, 4); g3.addEdge(1, 4);

// iteration nodes and edges
console.log("UNDIRECTED GRAPH (g1):");
console.log("nodes:");
g1.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
console.log("edges:")
g1.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

console.log("DIRECTED GRAPH (g2):");
console.log("nodes:");
g2.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
console.log("edges:")
g2.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

console.log("DIRECTED-MULTIGRAPH (g3):");
console.log("nodes:")
g3.eachNode(function (N) { console.log("id: " + N.id + " deg: " + N.deg); });
console.log("edges:")
g3.eachEdge(function (E) { console.log("n1: " + E.srcId + " n2: " + E.dstId); });

// connected components
components1 = g1.components()
console.log("CONNECTED COMPONENTS (g1)");
for (var i=0; i<components1.cols; i++) {
    idx = components1[i].idxVec();
    console.log("component: " + i);
    for (var j=0; j<idx.length; j++) {
        console.log("node " + idx[j]);
    }
}

// clustering coefficient
ccf1 = g1.clusteringCoefficient();
ccf2 = g2.clusteringCoefficient();
ccf3 = g3.clusteringCoefficient();
console.log("CLUSTERING COEFFICIENT (g1)");
console.log(ccf1);
console.log("CLUSTERING COEFFICIENT (g2): ");
console.log(ccf2);
console.log("CLUSTERING COEFFICIENT (g3): ");
console.log(ccf3);
