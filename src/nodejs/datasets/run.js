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
var qm = require('qminer');
var datasets = qm.datasets;

qm.delLock();
qm.config('qm.conf', true, 8080, 1024);

var base = qm.create('qm.conf', '', true); // 2nd arg: schema, 3rd arg: clear db folder = true

// Test desctiprion function
console.log(datasets.description());

//// Load Iris dataset in async way
//datasets.loadIris(base, function (err, store) {
//    if (err) throw err;
//    console.log("\nSucessfully loaded database: " + store.name)
//    console.log(JSON.stringify(store.first, null, 2));
//})

// Load Iris in sync way.
var Iris = datasets.loadIrisSync(base);
console.log("\nSucessfully loaded database: " + Iris.name)
console.log(JSON.stringify(Iris.first, null, 2));


console.log("\nTESTING TESTING\n")
// If loaded with async way, this output should be displayed before Iris is loaded.
// If loaded with sync way, this output should be displayed after Iris is loaded.

//base.close()
