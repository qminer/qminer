// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

var util = require("utilities.js");

module.exports = require("__snap__");
exports = module.exports; // re-establish link

// class for storing community sizes
function CmtySizes(CommunityId, Size) {
    this.CommunityId = CommunityId;
    this.Size = Size;
}
// class for storing community assignements
function CmtyC(NodeId, CommunityId) {
    this.NodeId = NodeId;
    this.CommunityId = CommunityId;
}
// class for community distribution statistics
function TransformationStat(IdPrev, Id, PrevP, CurrP) {
    this.IdPrev = IdPrev;
    this.Id = Id;
    this.PrevP = PrevP;
    this.CurrP = CurrP;
}

function getLabel(c, dist, prev_sizes, prev, alpha, beta, first_new_id) {
    // return value - initialized to first free id
    var returnName = first_new_id;
    // determine the size the current community
    var sizeT=0;
    for (var i = 0; i < dist.length; i++)
        sizeT += dist[i].Size;
    // TransformationStat - transformation statistics
    var stat = new Array();
    for (var i = 0; i < dist.length; i++) {
        if (dist[i].Size > 0) {
            var s = dist[i].Size;
            var c_1 = dist[i].CommunityId;
            for (var j = 0; j < prev_sizes.length; j++) {
                if (c_1 == prev_sizes[j].CommunityId) {
                    var sizeT_1 = prev_sizes[j].Size;
                    stat.push(new TransformationStat(c_1, c, s / sizeT_1, s / sizeT));
                }
            }
        }
    }

    // analyse the statistics - logic for determining label
    var count_curr_g_beta = new Array();
    var prev_g_alpha = new Array();
    for (var i = 0; i < stat.length; i++) {
        if (stat[i].PrevP > alpha && stat[i].CurrP > beta)
            prev_g_alpha.push(i);
    }
    if (prev_g_alpha.length == 1) {
        returnName = stat[prev_g_alpha[0]].IdPrev;
    }
    return returnName;
}

//#- `JSON = snap.evolutionJs(data, alpha, beta)` -- returns JSON of nodes and edges of community evolution
exports.evolutionJs = function (data, alpha, beta) {
    // containers for edges and nodes of the output json string
    var edgesJson = "";
    var communitiesJson = "";
    // container for community sizes in t-1
    var prev_sizes = new Array();
    // container for communities
    var prev = new Array();
    for (var br = 0; br < data.length; br++) {
        // data object assignement - CmtyV is a sparse vector
        var CmtyV = data[br];
        //  nodes
        cVal = CmtyV.valVec();
        // communities
        cKey = CmtyV.idxVec();
        // container for sizes of communities
        var prev_sizes_temp = new Array();
        // container for communities
        var prev_temp = new Array();
        // get unique communites
        cUnique = new Array();
        var temp_prev = -2;
        for (var i = 0; i < cKey.length; i++) {
            if (cKey[i] != temp_prev) {
                cUnique.push(cKey[i]);
                temp_prev = cKey[i];
            }
        }
        // first iteration
        if (br == 0) {
            // counter for storing community size
            count = 0;
            // initial state - store the communities and their sizes
            for (var i = 0; i < cKey.length; i++) {
                prev.push(new CmtyC(cVal[i], cKey[i]));
                if (i == 0) { count++; }
                else {
                    if (cKey[i] == cKey[i - 1]) { count++; }
                    else { prev_sizes.push(new CmtySizes(cKey[i - 1], count)); count = 1; }
                }
            }
            prev_sizes.push(new CmtySizes(cKey[cKey.length - 1], count));
            
            for (var i = 0; i < prev_sizes.length; i++)
                communitiesJson += ",{\"id\":" + prev_sizes[i].CommunityId + ", \"size\":" + prev_sizes[i].Size + ", \"t\":" + br + "}\n";
        }
        // later iterations (br>0)
        else {
            // ditribution of communities in t
            var dist = new Array();
            // map community names
            var map = new Array();
            // getting first new free community id
            var first_new_c_id = -1;
            for (var i = 0; i < prev_sizes.length; i++)
                if (prev_sizes[i].CommunityId > first_new_c_id)
                    first_new_c_id = prev_sizes[i].CommunityId;
            if (cUnique.length - 1 > first_new_c_id)
                first_new_c_id = cUnique.length - 1;
            first_new_c_id += 1;
            // needed for sorted printing out communities (nodes)  after labeling
            var cJ = new Array();
            // iterate each community in t
            for (var c = 0; c < cUnique.length; c++) {
                // current community
                var C = cUnique[c];
                // initialize distributions to 0
                dist = [];
                for (var i = 0; i < prev_sizes.length; i++)
                    dist.push(new CmtySizes(prev_sizes[i].CommunityId, 0));
                // for new communities
                dist.push(new CmtySizes(-1, 0));
                // iterate all nodes in community c in time t
                for (var i = 0; i < cVal.length; i++) {
                    if (cKey[i]==C) {
                        // node id
                        var id = cVal[i];
                        // prev_comm is the community of node i in t-1
                        var prev_comm = -1;
                        for (var j = 0; j < prev.length; j++)
                            if (id == prev[j].NodeId)
                                prev_comm = prev[j].CommunityId;
                        // update the distribution of communities
                        var tmp_val = 0, tmp_id=-1;
                        for (var j = 0; j < dist.length; j++)
                            if (dist[j].CommunityId == prev_comm) {
                                dist[j].Size += 1 
                                tmp_id = j;
                                tmp_val = dist[j].Size;
                            }                               
                        dist[tmp_id].Size = tmp_val + 1;
                    }
                } // end iterate all nodes

                // to get the size of the current community
                var sum_temp = 0;
                // determining label based on communities distribution
                var label = getLabel(C, dist, prev_sizes, prev, alpha, beta, first_new_c_id);
                // if the label is new, update the first free label
                if (label == first_new_c_id) first_new_c_id++;
                for (var i = 0; i < dist.length; i++) {
                    sum_temp += dist[i].Size;
                    if (dist[i].Size > 0 && dist[i].CommunityId != -1)
                        edgesJson += ",{ \"n1\":" + dist[i].CommunityId + ", \"n2\":" + label + ", \"w\": " + dist[i].Size + ", \"t0\":"+(br-1)+", \"t1\":"+br+"}\n";
                }
                // first adding to file, then sorting and finally printing to json string 
                cJ.push({ id: label, size: sum_temp, t: br });
                //n saving basic statistic for current time (br)
                prev_sizes_temp.push(new CmtySizes(label, sum_temp));
                for (var i = 0; i < cVal.length; i++)
                    if (cKey[i] == C) 
                        prev_temp.push(new CmtyC(cVal[i], label));
                
            } // end iterate all communties

            // sort the communities
            cJ.sort(function (obj1, obj2) { return obj1.id - obj2.id; });
            // print communites to json string
            for (var i = 0; i < cJ.length; i++)
                communitiesJson += ",{\"id\":" + cJ[i].id + ", \"size\":" + cJ[i].size + ", \"t\":" + cJ[i].t + "}\n";

            // update prev - node community assignements and prev_sizes - communities sizes 
            prev = [];
            prev_sizes = [];
            for (var i = 0; i < prev_sizes_temp.length; i++)
                prev_sizes.push(prev_sizes_temp[i]);
            for (var i = 0; i < prev_temp.length; i++)
                prev.push(prev_temp[i]);

        } // end main iteration for t>0

    } // end main iteration

    // printing all out to json string
    var out = "{ \"edges\": [\n";
    out += edgesJson.substr(1, edgesJson.length-1);
    out += "],\n\"communities\": [\n";
    out += communitiesJson.substr(1, communitiesJson.length-1);
    out += "]\n}";

    return out;
};

//#- `JSON = snap.toJson(graph)` -- returns JSON object of graph with `source` and `target` attributes
exports.toJsonGraph = function (data) {
    var br = 0;
    var json_out = "";
    for (var i = data.getFirstEdge() ; br < data.edgeCount() ; i.getNext()) {
        var n1 = i.getSrcNodeId();
        var n2 = i.getDstNodeId();
        json_out += ",{\"source\":" + n1 + ",\"target\":" + n2 + "}\n";
        br++;
    }
    json_out = "\n[" + json_out.substr(1, json_out.length - 1) + "]";
    var obj_out = eval("(" + json_out + ')');
    return obj_out;
};

//#- `JSON = snap.toJson(graph)` -- returns JSON object of array of graphs with `source` and `target` attributes
exports.toJsonGraphArray = function (data) {
    var json = "";
    var json_out = "";

    for (var j=0; j<data.length; j++) {
        json_out = "";
        var br = 0;
        for (var i = data[j].getFirstEdge() ; br < data[j].edgeCount() ; i.getNext()) {
            var n1 = i.getSrcNodeId();
            var n2 = i.getDstNodeId();
            json_out += ",{\"source\":" + n1 + ",\"target\":" + n2 + "}\n";
            br++;
        }
        json_out = "\n[" + json_out.substr(1, json_out.length - 1) + "]";

        if (j < data.length - 1)
            json_out += ",\n";

        json += json_out;
    }
    json = "\n[" + json + "]";
    var obj_out = eval("(" + json + ')');
    return obj_out;
};