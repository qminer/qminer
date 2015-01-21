// 
// Sample unit test using standard assert JS library 
// 

var assert = require("assert")
var la = require("../../build/Release/la.node")

	
describe('Import test', function(){
    it('if import of qminer.node succeeds, return true', function(){
      assert.equal(1,1);
    })
})

var v = new la.Vector();

describe('Property Tests', function(){
  describe('Vector Length Test', function(){
    it('should return 0 for vector v', function(){
      assert.equal(v.length,0);
    })
  })
})

describe('Functions Tests', function(){
  describe('Push Test', function(){
    it('pushing 3.1 and 4 into a vector v, v.length should return 2', function(){
      v.push(3.2);
      v.push(4);
      assert.equal(v.length,2);
    })
  })

  describe('At Test', function(){
    it('returns element in with indexes 0 (3.2) and 1 (4)', function(){
      assert.equal(3.2,v.at(0));
      assert.equal(4,v.at(1));
    })
  })
  
  describe('Sum Test', function(){
    it('should return a sum of 3.2 and 4', function(){
      assert.equal(3.2+4,v.sum());
    })
  })
  
  describe('getMaxIdx Test', function(){
    it('should return index of last element in vector, 1.', function(){
      assert.equal(v.length-1,v.getMaxIdx());
    })
  })
});

