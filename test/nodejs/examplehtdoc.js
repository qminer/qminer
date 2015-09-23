describe('example tests for the htdoc.js file', function () {
describe("String-string hashmap", function () {
it('should make test number 1', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.StrStrMap();
	 // Adding two key/dat pairs
	 h.put('foo', 'bar');
	 h.put('dog', 'tisa');
	 // Getting data
	 h.hasKey('foo'); // returns true
	 h.get('dog'); // returns 'tisa'
	 h.key(1); // returns 'dog'
	 h.dat(1); // returns 'tisa'
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.StrStrMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});
describe("String-Integer hashmap", function () {
it('should make test number 2', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.StrIntMap();
	 // Adding two key/dat pairs
	 h.put('foo', 10);
	 h.put('bar', 20);
	 // Getting data
	 h.hasKey('foo'); // returns true
	 h.get('bar'); // returns 20
	 h.key(1); // returns 'bar'
	 h.dat(1); // returns 20
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.StrIntMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});
describe("String-Float hashmap", function () {
it('should make test number 3', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.StrFltMap();
	 // Adding two key/dat pairs
	 h.put('foo', 10.5);
	 h.put('bar', 20.2);
	 // Getting data
	 h.hasKey('foo'); // returns true
	 h.get('bar'); // returns 20.2
	 h.key(1); // returns 'bar'
	 h.dat(1); // returns 20.2
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.StrFltMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});
describe("Int-string hashmap", function () {
it('should make test number 4', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.IntStrMap();
	 // Adding two key/dat pairs
	 h.put(10, 'foo');
	 h.put(20, 'bar');
	 // Getting data
	 h.hasKey(10); // returns true
	 h.get(20); // returns 'bar'
	 h.key(1); // returns 20
	 h.dat(1); // returns 'bar'
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.IntStrMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});
describe("Integer-Integer hashmap", function () {
it('should make test number 5', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.IntIntMap();
	 // Adding two key/dat pairs
	 h.put(5, 10);
	 h.put(15, 20);
	 // Getting data
	 h.hasKey(5); // returns true
	 h.get(15); // returns 20
	 h.key(1); // returns 15
	 h.dat(1); // returns 20
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.IntIntMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});
describe("Integer-Float hashmap", function () {
it('should make test number 6', function () {
 this.timeout(10000); 

	 // create a new hashtable
	 ht = require('qminer').ht;
	 var h = new ht.IntFltMap();
	 // Adding two key/dat pairs
	 h.put(5, 10.5);
	 h.put(15, 20.2);
	 // Getting data
	 h.hasKey(5); // returns true
	 h.get(15); // returns 20.2
	 h.key(1); // returns 15
	 h.dat(1); // returns 20.2
	 h.length; // returns 2	
	 // Saving and loading:
	 var fs = require('qminer').fs;
	 fout = fs.openWrite('map.dat'); // open write stream
	 h.save(fout).close(); // save and close write stream
	 var h2 = new ht.IntFltMap(); // new empty table
	 var fin = fs.openRead('map.dat'); // open read stream
	 h2.load(fin); // load

});
});

});
