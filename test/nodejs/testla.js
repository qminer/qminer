var la = require("../../src/nodejs/la/la.js");
var spmat = new la.sparseColMatrix([[[0,2.2]],[[2,3.3]]]);
spmat.frob() //native addon
spmat.frob2() //js code in la.js