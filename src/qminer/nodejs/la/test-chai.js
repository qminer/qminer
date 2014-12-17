//TO ADD: edge cases, ex. IndexOutOfRange, negative index, sum of empty vector

var la = require("./build/Release/la.node")

//importing assert and expect functions from chai assertion library
var assert = require('chai').assert
var expect = require('chai').expect


describe('Import test', function(){
    it('if import of la.node succeeds, return true', function(){
      expect(1).to.equal(1);
    })
})

var v = new la.vector();

var int1 = 3;
var int2 = 2;

var float1 = 18.2
var float2 = 3.6;

var negNum1 = -2;
var negNum2 = -4; 

//tests if length property is exposed correctly
describe('Properties Tests', function(){
  describe('Vector Length Test', function(){
    it('check if v has property length and has 0 elements', function(){
      //check if vector v has property length. Also check if it equals 0 (empty vector)
      expect(v).to.have.property('length');
      expect(v.length).to.equal(0);
    })
  })
})

describe('Functions Tests', function(){
  //tests if pushing elements into a vector works correctly
  describe('Push Test', function(){
    it('should push two numbers into vector', function(){
      v.push(int1);
      v.push(float2);
      assert(v.length == 2,'Not enough elements');
    })
  })

  describe('At Test', function(){
    //testes if recovering elements at given index works correctly
    it('returns element in with indexes 0 (3.2) and 1 (4)', function(){
      assert.equal(int1,v.at(0));
      assert.equal(float2,v.at(1));
    })
  })
  
  describe('Sum Test', function(){
    //tests if summation of vecotr elements works correctly
    it('should test integer sum', function(){
      var intVector = la.vector();
      intVector.push(int1);
      intVector.push(int2);
      expect(intVector.sum()).to.equal(int1+int2);
    })
    it('should test float sum', function(){
      var floatVector = la.vector();
      floatVector.push(float1);
      floatVector.push(float2);
      expect(floatVector.sum()).to.equal(float1+float2);
    })
    it('should test mixed sum', function(){
      var mixedVector = la.vector();
      mixedVector.push(int1);
      mixedVector.push(float1);
      expect(mixedVector.sum()).to.equal(int1+float1);
    })
    it('should test sum of negative numbers', function(){
      var negVector = la.vector();
      negVector.push(negNum2);
      negVector.push(negNum1);
      expect(negVector.sum()).to.equal(negNum2+negNum1);
    })
  })

<<<<<<< .merge_file_a01560
<<<<<<< .merge_file_a09896
  describe('Shuffle Test', function(){
    //tests shuffling elemnts inside a vector

    //NOTE: test needs to be refined to correctly test vectors with multiple elements of same value
    it('should shuffle elements inside a vector randomly', function(){
      
      var compare = la.vector();

      vec.push(3);
      vec.push(1);
      vec.push(15.5);
      vec.push(8);

      compare.push(3);
      compare.push(1);
      compare.push(15.5);
      compare.push(8);

      vec.shuffle();
      for (var i = 0; i<vec.length; i++) {
        expect(vec.at(i)).to.not.equal(compare.at(i));
      }
    })
  })
=======
=======
>>>>>>> .merge_file_a01448
//describe('Shuffle Test', function(){
//	//tests shuffling elemnts inside a vector
//
//	//NOTE: test needs to be refined to correctly test vectors with multiple elements of same value
//	it('should shuffle elements inside a vector randomly', function(){
//
//		var compare = la.vector();
//
//		vec.push(3);
//		vec.push(1);
//		vec.push(15.5);
//		vec.push(8);
//
//		compare.push(3);
//		compare.push(1);
//		compare.push(15.5);
//		compare.push(8);
//
//		vec.shuffle();
//		//for (var i = 0; i<vec.length; i++) {
//		//  expect(vec.at(i)).to.not.equal(compare.at(i));
//		//}
//	})
//})
<<<<<<< .merge_file_a01560
>>>>>>> .merge_file_a09212
=======
>>>>>>> .merge_file_a01448
  
  describe('Trunc Test', function(){
    //tests vector truncating
    it('should return shorter vector', function(){
      var truncVector = la.vector();
      truncVector.push(1);
      truncVector.push(2);
      truncVector.push(3);
      truncVector.push(4);
      expect(truncVector.length).to.equal(4);
      truncVector.trunc(2);

      expect(truncVector.length).to.equal(2);

    })

    it('trunc vector to same length', function(){

      var truncVector2 = la.vector();
      truncVector2.push(3);
      truncVector2.trunc(1);
      expect(truncVector2.length).to.equal(1);

    })

    it('trunc vector to zero length', function(){
      var truncVector = la.vector();
      truncVector.push(1);
      truncVector.push(2);
      truncVector.push(3);

      truncVector.trunc(0);
      expect(truncVector.length).to.equal(0);
    })
  })

  describe('GetMaxIdx Test', function(){
    //tests if returning index of last element in vector works correctly
    it('should return index of last element in vector.', function(){
      v.push(15);
      expect(v.length-1).to.equal(v.getMaxIdx());
    })
  })

  describe('Put Test', function(){
    it('should put three numbers in the beginning, middle and end of a vector', function(){
      var putVector = la.vector();

      for (var i = 1; i<21; i++) {
        putVector.push(i);
      }

      putVector.put(0,33);
      putVector.put(putVector.length-1,45);
      putVector.put(8,44);

      expect(putVector.at(0)).to.equal(33);
      expect(putVector.at(putVector.getMaxIdx())).to.equal(45);
      expect(putVector.at(8)).to.equal(44);

    })
  })

  describe('Sort Test', function(){

    //TO ADD: sorting negative values

    var sortVector = la.vector();

    //adds 9 elements to vector v
    for (var i = 1; i<10; i++) {
      sortVector.push(i);
    }

    sortVector.push(3);
    sortVector.push(2);

    describe('Ascending sort test', function() {
      it('should return vector sorted by descending values', function() {
<<<<<<< .merge_file_a01560
<<<<<<< .merge_file_a09896
        sortVector.sort(true)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(sortVector.at(i)).to.be.at.most(sortVector.at(i+1));
=======
        var res = sortVector.sort(true)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(res.at(i)).to.be.at.most(res.at(i+1));
>>>>>>> .merge_file_a09212
=======
        var res = sortVector.sort(true)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(res.at(i)).to.be.at.most(res.at(i+1));
>>>>>>> .merge_file_a01448
        }
      })
    })
    describe('Descending sort test', function() {
      it('should return vector sorted by ascending values', function() {
<<<<<<< .merge_file_a01560
<<<<<<< .merge_file_a09896
        sortVector.sort(false)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(sortVector.at(i)).to.be.at.least(sortVector.at(i+1));
=======
        var res = sortVector.sort(false)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(res.at(i)).to.be.at.least(res.at(i+1));
>>>>>>> .merge_file_a09212
=======
        var res = sortVector.sort(false)
        for (var i = 0; i < sortVector.length-1; i++) {
          expect(res.at(i)).to.be.at.least(res.at(i+1));
>>>>>>> .merge_file_a01448
        }
      })
    }) 
  })
});

