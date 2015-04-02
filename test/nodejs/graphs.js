console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');
var snap = require('../../').snap;

var g = new snap.UndirectedGraph();

g.addNode(1);
g.addNode(2);
g.addNode(3);
g.addNode(4);
g.addNode(5);
g.addNode(6);
g.addNode(7);

g.addEdge(1, 2);
g.addEdge(2, 3);
g.addEdge(3, 4);
g.addEdge(4, 1);
g.addEdge(4, 2);
g.addEdge(5, 6);
g.addEdge(5, 7);

console.log('Edge iterator:')
g.eachEdge(function(E){
	console.log(E.srcId+'-'+E.dstId);
});

var m = g.adjMat();
console.log('Adjacency matrix:');
m.print();

console.log('Drawing graph to \'g.html\'');
g.draw('g.html');