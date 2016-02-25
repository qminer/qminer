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
    describe('simple load-store-close-open test', function () { 
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
                //{ "name": "ActedIn", "type": "index", "store": tab2_name, "inverse": "Actor" },
                { "name": "Directed", "type": "index", "store": tab2_name }
            ],
            "keys": [
                //{ "field": "Name", "type": "text" },
                //{ "field": "Desc", "type": "value" }
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
                //{ "name": "Actor", "type": "index", "store": tab1_name, "inverse": "ActedIn" },

                //{ "name": "Director", "type": "field", "store": tab1_name }
                { "name": "Director", "type": "index", "store": tab1_name, "inverse": "Directed" }
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
            var director = {  Name: "Name " + i };
            //var actors = [];
            base.store(tab2_name).push({ 
                Name: "Another name " + i, 
                Age: (i * 17 + 13) % 97,
                //Director: director
                Director: [director]
            });
            if (i % 10000 == 0) {
                console.log("partial flush ", i);
                base.partialFlush(100);
                console.log(".");
            }
        }
        console.log("Closing base");
        base.close();
        
        console.log("Opening base");
        var base2 = new qm.Base({ mode: 'open' });
        var store2 = base2.store(tab2_name);
        var recs2 = store2.allRecords;
        assert.equal(recs2.length, rec_cnt);
        
        console.log("Checking records");
        for (var i = 0; i < rec_cnt; i++) {
            var rec2 = recs2[i];
            assert.equal(rec2.Name, "Another name " + i);
        }
        for (var i = rec_cnt; i < 2 * rec_cnt; i++) {
            var director2 = {  Name: "Name " + i };
            //var actors = [];
            base2.store(tab2_name).push({ 
                Name: "Another name " + i, 
                Age: (i * 17 + 13) % 97,
                Director: [director2]
            });
            if (i % 10000 == 0) {
                console.log("partial flush ", i);
                base2.partialFlush(100);
                console.log(".");
            }
        }
        base2.close();
    });
})