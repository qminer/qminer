var assert = require('assert');
var analytics = require('../../').analytics;
var la = require('../../').la;

var vec = new la.Vector({vals:4});
var mat = new la.Matrix({rows:2, cols:4});



var SVC = new analytics.SVC({verbose:true});
SVC.fit(mat,vec);
SVC.save('fif.bin');

console.log('bye');