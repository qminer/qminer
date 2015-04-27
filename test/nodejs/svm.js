/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
console.log(__filename)
var assert = require('assert');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

var vec = new la.Vector({vals:4});
var mat = new la.Matrix({rows:2, cols:4});

var vec = new la.Vector({vals:4});
var mat = new la.Matrix({rows:2, cols:4});
var x = new la.Vector({vals:2});

var SVC = new analytics.SVC({verbose:true});
SVC.fit(mat,vec);
SVC.save('svc.bin');

var y1 = SVC.predict(x);

var SVR = new analytics.SVR({verbose:true});
SVR.fit(mat,vec);
SVR.save('svr.bin');

var y1 = SVR.predict(x);