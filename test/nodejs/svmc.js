var assert = require('assert');
var analytics = require('../../').analytics;
var la = require('../../').la;

console.log('hej');

var vec = new la.Vector({vals:4});
var mat = new la.Matrix({rows:2, cols:4});


var vec = new la.Vector({vals:4});
var mat = new la.Matrix({rows:2, cols:4});
var x = new la.Vector({vals:2});

var SVC = new analytics.SVC({verbose:true});
SVC.fit(mat,vec);
SVC.save('fif.bin');

var y1 = SVC.predict(x);

console.log('fifa');



var SVR = new analytics.SVR({verbose:true});
SVR.fit(mat,vec);
SVR.save('fif.bin');

var y1 = SVR.predict(x);