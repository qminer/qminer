var ht = require("../../../build/Release/ht.node");
var fs = require("../../../build/Release/fs.node");

var StrIntH = ht.newStrIntH();

// .get and .put 
StrIntH.put("Robert Sedgewick", 65);
StrIntH.put("Tim Roughgarden", 30); // in the ballpark of 30 :) 
StrIntH.put("Donald Knuth", 76); 
StrIntH.put("Allen Hatcher", 67); 

console.log("Prof. Hatcher is " + StrIntH.get("Allen Hatcher") + " years old");

// hasKey
console.log("Wait, what? We know how old Don Knuth is? " + StrIntH.hasKey("Donald Knuth"));

// length 
console.log("Number of entries: " + StrIntH.length);

// key
console.log("Key on index 1: " + StrIntH.key(1));

// dat 
console.log("Dat on index 1: " + StrIntH.dat(1));

// save 
var fout = fs.openWrite("StrIntH.bin");
StrIntH.save(fout);
fout.close();

// load 
var fin = fs.openRead("StrIntH.bin");
var StrIntH_in = ht.newStrIntH();
StrIntH_in.load(fin);

console.log("Prof. Hatcher is still " + StrIntH_in.get("Allen Hatcher"));

fs.del("StrIntH.bin");

