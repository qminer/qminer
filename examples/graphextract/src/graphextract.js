var snap = require('snap.js');


var pubs = qm.store("Publications");
var auths = qm.store("Authors");

// read data and fill stores
var fin = fs.openRead("cobiss_pub_auth.txt");
fin.readLine();
while (!fin.eof) {
    var line = fin.readLine();
    var arr = line.split("\t");
    if (arr.length == 2) {
        var cId = parseInt(arr[0]);
        var mid = parseInt(arr[1]);
        // try to add cobissId and mstid to their stores
        var cRecId = pubs.add({ cobissId: cId });
        var aRecId = auths.add({ mstid: mid });
        
        pubs[cRecId].addJoin("authors", auths[aRecId]);
        auths[aRecId].addJoin("publications", pubs[cRecId]);

    }
}

// node store, link store
function buildGraph(nstore, lstore, nstoreFieldName,  njoinName, ljoinName) {
    var graph = snap.newUGraph();
    var recs = nstore.recs; // rec set in nstore
    // iterate over all nstore records
    for (var i = 0; i < recs.length; i++) {
        console.say(i + "/" + recs.length);
        var rec = recs[i];
        var recId = rec[nstoreFieldName];
        // is node Id? TODO
        graph.addNode(recId);
        // iterate over all recs in the other table
        var joinRecSet = rec[njoinName]; // rec set in lstore
        for (var j = 0; j < joinRecSet.length; j++) {
            var joinRec = joinRecSet[j];
            var joinjoinRecSet = joinRec[ljoinName]; // rec set in nstore
            for (var k = 0; k < joinjoinRecSet.length; k++) {
                var joinjoinRec = joinjoinRecSet[k];
                var rec2Id = joinjoinRec[nstoreFieldName]; // get the id of the author at hop 2
                // is node Id? TODO
                graph.addNode(rec2Id);
                graph.addEdge(recId, rec2Id);                
            }
        }
    }
    return graph;
}

console.log("lala");

var G = buildGraph(auths, pubs, "mstid", "publications", "authors");
G.dump("cobisg.txt");

eval(breakpoint);