var la = require('./build/Debug/la.node');
var stat = require('./build/Debug/stat.node');

///////////////////////////
// Return a new vector or a new matrix 
//la.newVector = function(args) { return new la.vector(args); }
//la.newIntVector = function(args) { return new la.vector(args); }
//la.newMatrix = function(args) { return new la.matrix(args); }
//la.newSparseVector = function(args) { return new la.sparseVector(args); }
//la.newSparseMatrix = function(args) { return new la.sparseMatrix(agrs); }


/////////////////////////////
//// TESTING la.mean() 


//console.log("== Testing la.mean() functionality ==\n")

//var testMat = la.newMatrix([[1, 2, 3], [2, 3, 4]])
//console.log(testMat.toString());

//console.log("testing la.mean(testMat)");
//console.log(stat.mean(testMat).toString());

////try {
////    console.log("testing la.mean(testMat)");
////    console.log(stat.mean(testMat).toString());
////} catch (e) {
////    console.log("FEFE" + e);
////}

//console.log(stat.mean(testVec).toString());

////console.log("testing la.mean(testVec)");
////var testVec = la.newVector([1, 2, 3]);
////console.log(testVec.toString());
////try {
////    console.log(stat.mean(testVec).toString());
////} catch (e) { console.log(e) }

//console.log(stat.mean(testNum).toString());

//console.log("testing error")
//var testNum = 2;
//try {
//    console.log(stat.mean(testNum).toString());
//} catch (e) {
//    console.log("FEFE" + e);
//}

//console.log("testing la.mean(testMat,2)");
//la.mean(testMat, 2).print();