var ht = require("../../../build/Debug/ht.node");
var fs = require("../../../build/Debug/fs.node");

var map = new ht.StrIntMap({key: 'string', value: 'int'});

// .get and .put 
map.put("Robert Sedgewick", 65);
map.put("Tim Roughgarden", 30); // in the ballpark of 30 :) 
map.put("Donald Knuth", 76); 
map.put("Allen Hatcher", 67); 

console.log("Prof. Hatcher is " + map.get("Allen Hatcher") + " years old");

// hasKey
console.log("Wait, what? We know how old Don Knuth is? " + map.hasKey("Donald Knuth"));

// length 
console.log("Number of entries: " + map.length);

// key
console.log("Key on index 1: " + map.key(1));

// dat 
console.log("Dat on index 1: " + map.dat(1));

// save 
var fout = fs.openWrite("map.bin");
map.save(fout);
fout.close();

// load 
var fin = fs.openRead("map.bin");
var mapIn = new ht.StrIntMap(fin);
mapIn.load(fin);

console.log("Prof. Hatcher is still " + mapIn.get("Allen Hatcher"));

fs.del("map.bin");

