var snap = require('./build/Release/snap.node');
console.log('hello');
snap.newDGraph = function() { return new snap.dgraph(); }
var g1 = new snap.ugraph();
var g2 = snap.newDGraph();
var g3 = new snap.dmgraph();

console.log("g1: "+g1);
console.log("g2: "+g2);
console.log("g3: "+g3);

g1.addNode(1);
g1.addNode(2);
g1.addNode(3);
g1.addNode(4);

g2.addNode(1);
g2.addNode(2);
g2.addNode(3);
g2.addNode(4);

g3.addNode(1);
g3.addNode(2);
g3.addNode(3);
g3.addNode(4);

g1.addEdge(1, 2);
g1.addEdge(2, 3);
g1.addEdge(3, 4);
g1.addEdge(4, 1);
g1.addEdge(4, 2);

g2.addEdge(1, 2);
g2.addEdge(2, 3);
g2.addEdge(3, 4);
g2.addEdge(4, 1);
g2.addEdge(4, 2);

g3.addEdge(1, 2);
g3.addEdge(2, 3);
g3.addEdge(3, 4);
g3.addEdge(4, 1);
g3.addEdge(4, 2);

g1.delNode(1);
g2.delEdge(2,3);

console.log('nodes: '+g1+" "+g1.nodes);
console.log('edges: '+g1+" "+ g1.edges);
console.log('nodes: '+g2+" "+g2.nodes);
console.log('edges: '+g2+" "+g2.edges);
console.log('nodes: '+g3+" "+g3.nodes);
console.log('edges: '+g3+" "+g3.edges);

console.log('is node(g1) 2: '+g1.isNode(2));
console.log('is edge(g1) 2 - 3: '+g1.isEdge(2, 3));
console.log('is edge(g1) 20 - 30: '+g1.isEdge(20, 30));

//var n = g1.node(1);
console.log('before creating a node');
var n = g1.node(2);
console.log("id: "+ n.id);
console.log("this is the created node: "+ n);
console.log('degree of the node is: '+n.deg);
for (var i=0; i<n.deg; i++) {
	console.log('nbr ' + (i+1) +'. is ' +n.nbrId(i));
}

/*
console.log('created g1 and g2');
console.log('nodes: '+ g2.nodes());
g1.addNode(1);
g2.addNode(1);
g3.addNode(1);
console.log(g1);
console.log('b');
console.log('a');
*/
//console.log(g);