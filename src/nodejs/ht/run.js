var ht = require("../../../build/Release/ht.node");

var StrIntH = new ht.newStrFltH();

StrIntH.put("Robert Sedgewick", 65);
StrIntH.put("Tim Roughgarden", 30); // in the ballpark of 30 :) 
StrIntH.put("Donald Knuth", 76); 
StrIntH.put("Allen Hatcher", 67); 

console.log("Prof. Hatcher is " + StrIntH.get("Allen Hatcher") + " years old");

