/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
var snap = require('./build/Release/snap.node');
//var snap = require('./build/Release/la.node');
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

//g1.delNode(1);
//g2.delEdge(2,3);

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
var br=0;
g1.eachEdge(function(E){ console.log(E.srcId+' - '+E.dstId);});
g1.addNode(7);
g1.addNode(8);
g1.addEdge(7,8);
var c = g1.components();
n.eachNbr(function(id){console.log(id)});
n = g2.node(1);
n.eachInNbr(function(id){console.log(id)});
n.eachOutNbr(function(id){console.log(id)});
n.eachNbr(function(id){console.log(id)});
n = g3.node(1);
n.eachEdge(function(id){console.log(id)});
n.eachInEdge(function(id){console.log(id)});
n.eachOutEdge(function(id){console.log(id)});
