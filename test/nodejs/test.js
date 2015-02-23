console.log(__filename)
// 
// Sample unit test using standard assert JS library 
// 
var assert = require("assert")
var la = require('../../').la;

	
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
  
  describe('Clone Test', function () {
      it('should return true if v is equal w', function () {
          var w = new la.Vector(v);
          assert.deepEqual(v, w);
    })
  })

  describe('Ones Test', function () {
      it('should return a 5-dimensional vector whose entries are set to 1.0', function () {
          var n = 5;
          var w = new la.ones(n);
          for (var i = 0; i < w.length; i++) {
              assert.equal(w.at(i), 1);
          }
      })
  })

  //describe('Square test', function () {
  //    it('should square all values of vector v', function () {
  //        var array = [1, 2, 3, 4];
  //        var w = new la.Vector(array); la.square(w);
  //        for (var i = 0; i < w.length; i++) {
  //            assert.equal(w.at(i), array[i] * array[i]);
  //        }
  //    })
  //})

  describe('Sort Test', function () {
      it('should sort vector in ascending order [0.11, 0.12, 3.5, 4]', function () {
          var array = [0.11, 0.12, 3.5, 4];
          var sortedVec = new la.Vector(array);

          var vec = new la.Vector([3.5, 0.12, 4, 0.11]);
          var vec = vec.sort();

          assert.deepEqual(vec, sortedVec);
      })
  })

  describe('Plus Test', function () {
      it('should sum the vectors', function () {
          var vec1 = new la.Vector([1, 3, 4, 8]);
          var vec2 = new la.Vector([4, 3, 8, 2.1]);
          var vec = vec1.plus(vec2);
          var controlVec = new la.Vector([5, 6, 12, 10.1]);
          assert.deepEqual(vec, controlVec);
      })
  })

});

//////////////////////////////////////////////////////////////////
// Function tests for matrix

var mat = new la.Matrix();

describe('Property Tests', function () {
    describe('Matrix Row Test', function () {
        it('should return 0 for matrix mat', function () {
            assert.equal(mat.rows, 0);
        })
    })

    describe('Matrix Col Test', function () {
        it('should return 0 for matrix mat', function () {
            assert.equal(mat.cols, 0);
        })
    })

    describe('Matrix At Test', function () {
        it('should return the element at (0, 1): 2', function () {
            mat = new la.Matrix([[1, 2], [3, 4]]);
            assert.equal(mat.at(0,1), 2);
       });
    })

    describe('Matrix Clone Test', function () {
        it('should return true if mat is equal mat2', function () {
            var mat2 = new la.Matrix(mat);
            assert.deepEqual(mat, mat2);
        })
    })

    describe('Eye Test', function () {
        it('should return a 3 x 3 identity matrix', function () {
            var dim = 3;
            var mat3 = new la.eye(dim);
            for (var i = 0; i < mat3.rows; i++) {
                for (var j = 0; j < mat3.cols; j++) {
                    if (i == j) { assert.equal(mat3.at(i, j), 1); }
                    else { assert.equal(mat3.at(i, j), 0); }
                }
            }
        })
    })

    describe('Zero Test', function () {
        it('should return a 3 x 2 zero matrix', function () {
            var nRows = 3;
            var nCols = 2;
            var mat3 = new la.zeros(nRows, nCols);
            for (var i = 0; i < mat3.rows; i++) {
                for (var j = 0; j < mat3.cols; j++) {
                    assert.equal(mat3.at(i, j), 0);
                }
            }
        })
    })

});
