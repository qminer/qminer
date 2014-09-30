var snap = require('snap.js');
var viz = require('visualization.js');
var pubs = qm.store("Publications");
var auths = qm.store("Authors");

function Table(n1, n2) {
    this.n1 = n1;
    this.n2 = n2;
}

function Table3(n1, n2, att) {
    this.n1 = n1;
    this.n2 = n2;
    this.att = att;
}

var sort_by = function (field, reverse, primer) {
    var key = function (x) { return primer ? primer(x[field]) : x[field] };

    return function (a, b) {
        var A = key(a), B = key(b);
        return ((A < B) ? -1 : ((A > B) ? 1 : 0)) * [-1, 1][+!!reverse];
    }
}

// read data and fill authors store

console.log("start reading files");

var fin = fs.openRead("cobiss_author.txt");
fin.readLine();
while (!fin.eof) {
    var line = fin.readLine();
    var arr = line.split("\t");
    if (arr.length >= 2) {
        var mid = String(arr[0]);
        var iid = String(arr[1]);
        var aRecId = auths.add({ mstid: mid, izumid: iid });
    }
}

// read data and fill publications store

console.log("reading pubs");
var fin = fs.openRead("cobiss_publication1.txt");
fin.readLine();
while (!fin.eof) {
    var line = fin.readLine();
    var arr = line.split("\t");
    if (arr.length >= 6) {
        var cid = String(arr[0]);
        var title_ = arr[1];
        var desc_ = arr[2];
        var d = new Date();
        d.setYear(parseInt(arr[5]));
        var year_ = d.toISOString();
        var cRecId = pubs.add({ cobissId: cid, title: title_, description: desc_, year: year_ });
    }
}

// read data and fill authors - publications relation store

console.log("reading pubs auth");
var fin = fs.openRead("cobiss_pub_auth1.txt");
fin.readLine();
while (!fin.eof) {
    var line = fin.readLine();
    var arr = line.split("\t");
    if (arr.length == 2) {
        var cid = String(arr[0]);
        var mid = String(arr[1]);

        //var aRecId = qm.search({ $from: "Authors", "mstidkey": mid.toString() })[0].$id;
        //var cRecId = qm.search({ $from: "Publications", "cobisskey": cid.toString() })[0].$id;

        var aRec = auths.rec(mid);
        var cRec = pubs.rec(cid);

        if (aRec && cRec) {
            cRec.addJoin("authors", aRec);
            aRec.addJoin("publications", cRec);
        }
        //console.log(auths[aRecId] + "  -  " + pubs[cRecId]);
    }
}

console.log("done reading");
// node store, link store
function buildGraph(nodeStore, linkStore, nodeKey,  nodeJoin, linkJoin) {
    var graph = snap.newUGraph();
    var recs = nodeStore.recs; // rec set in nstore
    // iterate over all nstore records
    for (var i = 0; i < recs.length; i++) {
        var rec = recs[i];
        var recId = rec[nodeKey];
        if (!graph.isNode(recId)) {
            graph.addNode(recId);
        }
        // iterate over all recs in the other table
        var joinRecSet = rec[nodeJoin]; // rec set in lstore
        for (var j = 0; j < joinRecSet.length; j++) {
            var joinRec = joinRecSet[j];
            var joinjoinRecSet = joinRec[linkJoin]; // rec set in nstore
            for (var k = 0; k < joinjoinRecSet.length; k++) {
                var joinjoinRec = joinjoinRecSet[k];
                var rec2Id = joinjoinRec[nodeKey]; // get the id of the author at hop 2
                if (!graph.isNode(rec2Id)) {
                    graph.addNode(rec2Id);
                }
                graph.addEdge(recId, rec2Id);                
            }
        }
    }
    return graph;
}

function biPartiteEdgeList(nodeStore, nodeKey, nodeJoin, linkJoin) {
    var table = new Array();
    var recs = nodeStore.recs; // rec set in nstore
    // iterate over all nstore records
    for (var i = 0; i < recs.length; i++) {
        //console.say(i + "/" + recs.length);
        var rec = recs[i];
        var recId = rec[nodeKey];

        // iterate over all recs in the other table
        var joinRecSet = rec[nodeJoin]; // rec set in lstore
        for (var j = 0; j < joinRecSet.length; j++) {
            var joinRec = joinRecSet[j];
            var joinjoinRecSet = joinRec[linkJoin]; // rec set in nstore
            for (var k = 0; k < joinjoinRecSet.length; k++) {
                var joinjoinRec = joinjoinRecSet[k];
                var rec2Id = joinjoinRec[nodeKey]; // get the id of the author at hop 2
                table.push(new Table(parseInt(recId), parseInt(rec2Id)));
            }
        }
    }
    return table;
}

function biPartiteEdgeListTime(nodeStore, nodeKey, nodeJoin, linkJoin) {
    var table = new Array();
    var recs = nodeStore.recs; // rec set in nstore
    // iterate over all nstore records
    for (var i = 0; i < recs.length; i++) {
        var rec = recs[i];
        var recId = rec[nodeKey];

        // iterate over all recs in the other table
        var joinRecSet = rec[nodeJoin]; // rec set in lstore

        for (var j = 0; j < joinRecSet.length; j++) {
            var joinRec = joinRecSet[j];
            var joinjoinRecSet = joinRec[linkJoin]; // rec set in nstore
            var time = joinRec["year"]["year"];
            for (var k = 0; k < joinjoinRecSet.length; k++) {
                var joinjoinRec = joinjoinRecSet[k];
                var rec2Id = joinjoinRec[nodeKey]; // get the id of the author at hop 2
                table.push(new Table3(parseInt(recId), parseInt(rec2Id), parseInt(time)));
            }
        }
    }
    return table;
}

function tableToGraph(table) {
    var graph = snap.newUGraph();
    for (var i = 0; i < table.length; i++) {
        var n1 = table[i].n1;
        var n2 = table[i].n2;
        if (!graph.isNode(n1))
            graph.addNode(n1);
        if (!graph.isNode(n2))
            graph.addNode(n2);
        graph.addEdge(n1, n2);
    }
    return graph;
}

function tableToGraphArrayAggTime(table, step) {
    var graph = snap.newUGraph();
    var graphs = new Array();

    if (table.length == 0)
        return null;

    var old = table[0].att;

    var br=0;
    for (var i = 0; i < table[table.length - 1].att - table[0].att; i+=step) {
        var graph = snap.newUGraph();
        graphs.push(graph);
        br++;
    }
    var j = 0;

    for (var i = 0; i < table.length; i++) {
        if (table[i].att >= old + step) {
            old = table[i].att;
            j++;
            //var graph = snap.newUGraph();
        }

        var n1 = table[i].n1;
        var n2 = table[i].n2;
        for (var k = j; k < br; k++) {
            if (!graphs[k].isNode(n1))
                graphs[k].addNode(n1);
            if (!graphs[k].isNode(n2))
                graphs[k].addNode(n2);
            graphs[k].addEdge(n1, n2);
        }
    }
    return graphs;
}

//var G = buildGraph(auths, pubs, "mstid", "publications", "authors");
//G.dump("cobisg.txt");

eval(breakpoint);

console.log("start creating graph...");
var t = biPartiteEdgeListTime(auths, "mstid", "publications", "authors");
//t.sort(function (a, b) { return parseInt(a.att) - parseInt(b.att) });
console.say("now sort");
t.sort(sort_by('att', true, parseInt));
console.say("now table to graph");
var g = tableToGraph(t);
var gs = tableToGraphArrayAggTime(t,6);
console.log("finished creating graph - test t");

var communities = new Array();
for (var i = 0; i < gs.length; i++) {
    console.say("doing g" + i);
    communities.push(snap.CommunityDetection(gs[i], "cnm"));
}
var json = snap.evolutionJs(communities, 0.5, 0.75);
viz.drawCommunityEvolution(json, "cmty_evolution.html", { title: { text: "Community evolution - GirvanNewman, small graphs 8 years, alpha=0.5. beta=0.75" } });

eval(breakpoint);