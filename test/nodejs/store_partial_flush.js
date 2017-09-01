/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// console.log(__filename)
var assert = require('../../src/nodejs/scripts/assert.js');     //adds assert.run function
var qm = require('qminer');
var fs = qm.fs;

describe('Partial-flush tests', function () {
    it('should perform a simple load-store-close-open test', function (done) {
        this.timeout(300 * 1000);

        var rec_cnt = 100 * 1000;
        var tab1_name = "Store1";
        var tab2_name = "Store2";

        var base = new qm.Base({ mode: 'createClean' });
        base.createStore([{
            "name": tab1_name,
            "fields": [
                { "name": "Name", "type": "string", "primary": true }
            ],
            "joins": [
                { "name": "Directed", "type": "index", "store": tab2_name }
            ],
            "keys": [
            ],
            "options": {
                "storage_location": "cache"
            }
        },
        {
            "name": tab2_name,
            "fields": [
                { "name": "Name", "type": "string", "primary": true },
                { "name": "Age", "type": "byte" }
            ],
            "joins": [
                { "name": "Director", "type": "index", "store": tab1_name, "inverse": "Directed" }
            ],
            "keys": [
                { "field": "Name", "type": "value" },
                { "field": "Age", "type": "linear" }
            ],
            "options": {
                "storage_location": "cache"
            }
        },
        {
            "name": tab2_name + "XX",
            "fields": [
                { "name": "Name", "type": "string", "primary": true },
                { "name": "Age", "type": "byte" }
            ],
            "joins": [
                { "name": "Director", "type": "index", "store": tab1_name }
            ],
            "keys": [
                { "field": "Name", "type": "value" },
                { "field": "Age", "type": "linear" }
            ],
            "options": {
                "storage_location": "cache"
            }
        }]);

        for (var i = 0; i < rec_cnt; i++) {
            var director = { Name: "Name " + i };
            base.store(tab2_name).push({
                Name: "Another name " + i,
                Age: (i * 17 + 13) % 97,
                Director: [director]
            });
            if (i % 10000 == 0) {
                //console.log("partial flush ", i);
                base.partialFlush(100);
                //console.log(".");
            }
        }
        //console.log("Closing base");
        base.close();

        //console.log("Opening base");
        var base2 = new qm.Base({ mode: 'open' });
        var store2 = base2.store(tab2_name);
        var recs2 = store2.allRecords;
        assert.equal(recs2.length, rec_cnt);

        //console.log("Checking records");
        for (var i = 0; i < rec_cnt; i++) {
            var rec2 = recs2[i];
            assert.equal(rec2.Name, "Another name " + i);
        }
        for (var i = rec_cnt; i < 2 * rec_cnt; i++) {
            var director2 = { Name: "Name " + i };
            //var actors = [];
            base2.store(tab2_name).push({
                Name: "Another name " + i,
                Age: (i * 17 + 13) % 97,
                Director: [director2]
            });
            if (i % 10000 == 0) {
                //console.log("partial flush ", i);
                base2.partialFlush(100);
                //console.log(".");
            }
        }
        base2.close();
        done();
    });
    it('should reproduce error from production 26.02.2016', function (done) { 
        this.timeout(300 * 1000);

        var rec_cnt = 100 * 1000;
        var tab1_name = "ProcessRegister";
        var tab2_name = "Logs";

        var base = new qm.Base({ mode: 'createClean' });
        base.createStore([
            {
                "name": tab1_name,
                "fields": [ { "name": "name", "type": "string", "primary": true } ],
                "joins": [],
                "keys": [ { "field": "name", "type": "value" } ]
            },
            {
                "name": tab2_name,
                "fields": [
                    { "name": "procid", "type": "uint64", "null": true },
                    { "name": "ts", "type": "datetime" },
                    { "name": "threadid", "type": "int", "null": true },
                    { "name": "type", "type": "string", "codebook": true },
                    { "name": "sfile", "type": "string", "codebook": true, "null": true },
                    { "name": "sfile_lnum", "type": "int", "null": true },
                    { "name": "slfile", "type": "string", "codebook": true, "null": true },
                    { "name": "slfile_lnum", "type": "int", "null": true },
                    { "name": "context_id", "type": "string", "null": true },
                    { "name": "msg", "type": "string", "store": "cache" },
                    { "name": "server", "type": "string", "codebook": true },
                    { "name": "log_file_name", "type": "string", "codebook": true },
                    { "name": "group", "type": "string", "codebook": true, "null": true }
                ],
                "joins": [
                    { "name": "process", "type": "field", "store": tab1_name, "storage": "int16-" }
                ],
                "keys": [
                    { "field": "sfile", "type": "value" },
                    { "field": "context_id", "type": "value" },
                    { "field": "ts", "type": "linear" }
                ],
                "options": {
                    "storage_location": "cache"
                }
            }
        ]);
        
        var now = new Date();
        for (var i = 0; i < rec_cnt; i++) {
            var rec = { 
                procid: (i * 57398) % 882191, 
                ts: now.getTime(),
                threadid: i % 42,
                type: (i % 4 == 0 ? "ERROR" : "WARN"),
                sfile: (i % 4 == 0 ? "a.cpp" : "b.cpp"),
                sfile_lnum: (i * 43 % 23392),
                slfile: (i % 4 == 0 ? "c.cpp" : "s.cpp"),
                slfile_lnum: (i * 41 % 13392),
                //context_id
                msg: "Error message " + (i * 98 % 1234),
                server: (i % 2 == 0 ? "s1" : "s2"),
                log_file_name: "log_file_" + (i % 5),
                // group
                process: { name: "process" + (i % 5)}
            };
            //console.log(rec)
            base.store(tab2_name).push(rec);
            if (i % 10000 == 0) {
                //console.log("partial flush ", i);
                base.partialFlush(100);
                //console.log(".");
            }
        }
        //console.log("Closing base");
        base.close();
        
        //console.log("Opening base");
        var base2 = new qm.Base({ mode: 'open' });
        var store2 = base2.store(tab2_name);
        var recs2 = store2.allRecords;
        assert.equal(recs2.length, rec_cnt);
        
        //console.log("Checking records");
        for (var i = 0; i < rec_cnt; i++) {
            var rec2 = recs2[i];
            assert.equal(rec2.server, (i % 2 == 0 ? "s1" : "s2"));
        }
        
        //console.log("Adding new data");
        now = new Date();
        for (var i = rec_cnt; i < 2 * rec_cnt; i++) {
            var rec2 = { 
                procid: (i * 57398) % 882191, 
                ts: now.getTime(),
                threadid: i % 42,
                type: (i % 4 == 0 ? "ERROR" : "WARN"),
                sfile: (i % 4 == 0 ? "a.cpp" : "b.cpp"),
                sfile_lnum: (i * 43 % 23392),
                slfile: (i % 4 == 0 ? "c.cpp" : "s.cpp"),
                slfile_lnum: (i * 41 % 13392),
                //context_id
                msg: "Error message " + (i * 98 % 1234),
                server: (i % 2 == 0 ? "s1" : "s2"),
                log_file_name: "log_file_" + (i % 5),
                // group
                process: { name: "process" + (i % 5)}
            };
            base2.store(tab2_name).push(rec2);
            if (i % 10000 == 0) {
                //console.log("partial flush ", i);
                base2.partialFlush(100);
                //console.log(".");
            }
        }
        base2.close();
        done();
    });    
})