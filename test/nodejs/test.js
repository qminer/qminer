console.log(__filename)
// 
// Sample unit test using standard assert JS library 
// 
var assert = require("../../src/nodejs/scripts/assert.js")
var la = require('../../').la;

	
describe('Import test', function(){
    it('if import of qminer.node succeeds, return true', function(){
      assert.equal(1,1);
    })
})

var v = new la.Vector();

describe('Vector Tests', function () { 

    describe('Property Tests', function(){
      describe('Length Test', function(){
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
          assert.equal(4, v.at(1));
        })
      })

      describe('At "Out of Bound" Test', function () {
          it('should give an error for accessing an element out of bound', function () {
              assert.throws(function () {
                  var n = v.at(-1);
              })
          })
      })

      describe('Sum Test', function(){
        it('should return a sum of 3.2 and 4', function(){
          assert.equal(3.2+4,v.sum());
        })
      })
  
      describe('Sum "Zero" Test', function () {
          it('the sum returned should be zero', function () {
              var vec = la.Vector([3.14 * 0.0001, 3.14 * -0.0001]);
              assert.equal(vec.sum(), 0);
          })
      })

      describe('getMaxIdx Test', function(){
        it('should return index of last element in vector, 1.', function(){
          assert.equal(v.length-1,v.getMaxIdx());
        })
      })

        // ??
      describe('getMaxIdx "Problem" Test', function () {
          it('what if all elements are the same, 1', function () {
              var vec = new la.Vector([1, 1, 1, 1]);
              assert.equal(vec.getMaxIdx(), 0);

              // assert.equal(vec.getMaxIdx(), 1); // should also work, but doesn't 
          })
      })
  
      describe('Clone Test', function () {
          it('should return true if v is equal w', function () {
              var w = new la.Vector(v);

              assert.deepEqual(v, w);
              for (var i = 0; i < v.length; i++) {
                  assert.equal(w.at(i), v.at(i));
              }
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

        // isn't implemented
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
              for (var i = 0; i < vec.length; i++) {
                  assert.equal(vec.at(i), sortedVec.at(i));
              }
          })
      })

      describe('Plus Test', function () {
          it('should sum the vectors', function () {
              var vec1 = new la.Vector([1, 3, 4, 8]);
              var vec2 = new la.Vector([4, 3, 8, 2.1]);
              var vec = vec1.plus(vec2);
              var controlVec = new la.Vector([5, 6, 12, 10.1]);

              assert.deepEqual(vec, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.equal(vec.at(i), controlVec.at(i));
              }
          })
      })

      describe('Minus Test', function () {
          it('should substract the vectors', function () {
              var vec1 = new la.Vector([1, 3, 4, 8]);
              var vec2 = new la.Vector([4, 3, 8, 2.1]);
              var vec = vec1.minus(vec2);
              var controlVec = new la.Vector([-3, 0, -4, 5.9]);

              assert.deepEqual(vec, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.equal(vec.at(i), controlVec.at(i));
              }
          })
      })

      describe('Multiply Test', function () {
          it('should multiply vector vec with scalar 3.14', function () {
              var vec = new la.Vector([3, 0, -12, 0.0001]);
              var vec2 = vec.multiply(3.14);
              var controlVec = new la.Vector([9.42, 0, -37.68, 0.000314]);

              assert.deepEqual(vec2, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.eqtol(vec2.at(i), controlVec.at(i));
              }
          })
      })

      describe('Multiply "Small scalar" Test', function () {
          it('should multiply vector vec with scalar close to 0: Number.MIN_VALUE', function () {
              var vec = new la.Vector([3, 0, 1, 0.0001]);
              var vec2 = vec.multiply(Math.pow(10, -5));
              var controlVec = new la.Vector([0.00003, 0, 0.00001, 0.000000001]);

              assert.deepEqual(vec2, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.eqtol(vec2.at(i), controlVec.at(i));
              }
          })
      })

      describe('Subvector Test', function () {
          it('should return the subvector of vector vec', function () {
              var vec = new la.Vector([3, -51, 22, 19]);
              var indVec = new la.IntVector([1, 3, 2, 0]);
              vec = vec.subVec(indVec);
              var controlVec = new la.Vector([-51, 19, 22, 3]);

              assert.deepEqual(vec, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.equal(vec.at(i), controlVec.at(i));
              }
          })
      })

      describe('At [] Test', function () {
          it('should return elements with indexes 0 (3.2) and 1 (4)', function () {
              assert.equal(3.2, v[0]);
              assert.equal(4, v[1]);
          })
          it('should save new value at index 0 (12)', function () {
              v[0] = 12;
              assert.equal(12, v[0]);
          })
      })

      describe('Put Test', function () {
          it('should put the value -21 at index 1', function () {
              v.put(1, -21);
              assert.equal(-21, v[1]);
          })
      })

      describe('Diag Test', function () {
          it('should return a matrix with the vector v on it\'s diagonal', function () {
              var mat = v.diag();
              for (var i = 0; i < mat.rows; i++) {
                  for (var j = 0; j < mat.cols; j++) {
                      if (i == j) { assert.equal(mat.at(i, j), v[i]); }
                      else { assert.equal(mat.at(i, j), 0); }
                  }
              }
          })
      })

      describe('PushV Test', function () {
          it('should return v with appended vector [1, 2, 3]', function () {
              var w = new la.Vector([1, 2, 3]);
              v.pushV(w);
              assert.equal(v.length, 5);
              var controlVec = new la.Vector([12, -21, 1, 2, 3]);
 
              assert.deepEqual(v, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.equal(v.at(i), controlVec.at(i));
              }
          })
      })

      describe('Norm Test', function () {
          it('should return the norm of vec 5', function () {
              var vec = new la.Vector([3, 4]);
              var n = vec.norm();
              assert.eqtol(n, 5);
          })
      })

      describe('Normalize Test', function () {
          it('should normalize vec and return vector [3/5, 4/5]', function () {
              var vec = new la.Vector([3, 4]);
              vec.normalize();
              var controlVec = new la.Vector([3 / 5, 4 / 5]);

              assert.deepEqual(vec, controlVec);
              for (var i = 0; i < controlVec.length; i++) {
                  assert.eqtol(vec.at(i), controlVec.at(i));
              }
          })
      })

      describe('Normalize "Zer0 Vector" Test', function () {
          it('should return the same vector', function () {
              var vec = new la.Vector([0, 0, 0, 0]);
              vec.normalize();
              var controlVec = new la.Vector([0, 0, 0, 0]);

              assert.deepEqual(vec, controlVec);
              for (var i = 0; i < vec.length; i++) {
                  assert.equal(vec.at(i), controlVec.at(i));
              }
          })
      })

      describe('ToMat Test', function () {
          it('should return matrix with a single column that equals vec', function () {
              var mat = v.toMat();
              for (var i = 0; i < v.length; i++) {
                  assert.equal(mat.at(i, 0), v.at(i));
              }
          })
      })

    })
});

//////////////////////////////////////////////////////////////////
// Function tests for matrix

var mat = new la.Matrix();
var mat2 = new la.Matrix([[3, -1], [8, -2]]);

describe('Matrix Test', function () {

    describe('Property Tests', function () {
        describe('Row Test', function () {
            it('should return 0 for matrix mat', function () {
                assert.equal(mat.rows, 0);
            })
        })

        describe('Col Test', function () {
            it('should return 0 for matrix mat', function () {
                assert.equal(mat.cols, 0);
            })
        })
    });

    describe('Functions Test', function () {

        describe('At Test', function () {
            it('should throw error, mat has no elements (assert catches error)', function () {
                assert.throws(function () {
                    var k = mat.at(0, 0);
                })
            })
        })

        describe('At Test', function () {
            it('should return the element at (0, 1): 2', function () {
                mat = new la.Matrix([[1, 2], [3, 4]]);
                assert.equal(mat.at(0, 1), 2);
            });
        })

        // not implemented yet
        //describe('Constructor "Clone" Test', function () {
        //    it('should make the same matrix as mat', function () {
        //        var mat2 = new la.Matrix(mat);

        //        assert.deepEqual(mat, mat2);
        //        for (var i = 0; i < mat2.rows; i++) {
        //            for (var j = 0; j < mat2.cols; j++) {
        //                assert.eqtol(mat2.at(i, j), mat.at(i, j));
        //            }
        //        }
        //    })
        //})

        describe('Put Test', function () {
            it('should put value 10 at (1, 1)', function () {
                mat.put(1, 1, 10);
                assert.equal(mat.at(1, 1), 10);
            })
        })

        describe('Multiply "Scalar" Test', function () {
            it('should multiply matrix with scalar 10', function () {
                var mat3 = mat.multiply(10);
                for (var i = 0; i < mat.rows; i++) {
                    for (var j = 0; j < mat.cols; j++) {
                        assert.equal(mat3.at(i, j), 10*mat.at(i, j));
                    }
                }
            })
        })

        describe('Multiply "Vector" Test', function () {
            it('should multiply mat with vector [1, 2]', function () {
                var vec = mat.multiply(new la.Vector([1, 2]));
                var controlVec = new la.Vector([5, 23]);

                assert.deepEqual(vec, controlVec);
                for (var i = 0; i < controlVec.length; i++) {
                    assert.equal(vec.at(i), controlVec.at(i));
                }
            }) 
        })

        describe('Multiply "Matrix" Test', function () {
            it('should multiply mat with [[3, -1],[8, -2]]', function () {
                var mat3 = mat.multiply(mat2);
                var controlMat = new la.Matrix([[19, -5], [89, -23]]);

                for (var i = 0; i < controlMat.rows; i++) {
                    for (var j = 0; j < controlMat.cols; j++) {
                        assert.equal(mat3.at(i, j), controlMat.at(i, j));
                    }
                }
            })
        })

        describe('MultiplyT "Scalar" Test', function () {
            it('should transpose mat and multiply with scalar 10', function () {
                var mat3 = mat.multiplyT(10);
                for (var i = 0; i < mat.rows; i++) {
                    for (var j = 0; j < mat.cols; j++) {
                        assert.equal(mat3.at(j, i), 10 * mat.at(i, j));
                    }
                }
            })
        })

        describe('MultiplyT "Vector" Test', function () {
            it('should transpose mat and multiply with vector [1, 2]', function () {
                var vec = mat.multiplyT(new la.Vector([1, 2]));
                var controlVec = new la.Vector([7, 22]);

                assert.deepEqual(vec, controlVec);
                for (var i = 0; i < controlVec.length; i++) {
                    assert.equal(vec.at(i), controlVec.at(i));
                }
            })
        })

        describe('MultiplyT "Matrix" Test', function () {
            it('should tranpose mat multiply with matrix [[3, -1],[8, -2]]', function () {
                var mat3 = mat.multiplyT(mat2);
                var controlMat = new la.Matrix([[27, -7], [86, -22]]);

                for (var i = 0; i < controlMat.rows; i++) {
                    for (var j = 0; j < controlMat.cols; j++) {
                        assert.equal(mat3.at(i, j), controlMat.at(i, j));
                    }
                }
            })
        })

        describe('Plus Test', function () {
            it('should add mat and mat2', function () {
                var mat3 = mat.plus(mat2);

                assert.deepEqual(mat3, mat);
                for (var i = 0; i < mat.rows; i++) {
                    for (var j = 0; j < mat.cols; j++) {
                        assert.equal(mat3.at(i, j), mat.at(i, j) + mat2.at(i, j));
                    }
                }
            })
        })

        describe('Minus Test', function () {
            it('should substract mat2 from mat', function () {
                var mat3 = mat.minus(mat2);
                
                assert.deepEqual(mat3, mat);
                for (var i = 0; i < mat.rows; i++) {
                    for (var j = 0; j < mat.cols; j++) {
                        assert.equal(mat3.at(i, j), mat.at(i, j) - mat2.at(i, j));
                    }
                }
            })
        })

        describe('Transpose Test', function () {
            it('should transpose mat', function () {
                var mat3 = mat.transpose();
                assert.deepEqual(mat3, mat);
                for (var i = 0; i < mat.rows; i++) {
                    for (var j = 0; j < mat.cols; j++) {
                        assert.equal(mat3.at(j, i), mat.at(i, j));
                    }
                }
            })
        })

        describe('Solve Test', function () {
            it('should solve linear system A*x = y', function () {
                var vec = mat.solve(new la.Vector([-1, -7]));       // it changes the (1, 1)-th element 10 back to 4?
                var solution = new la.Vector([1, -1]);

                assert.deepEqual(vec, solution);
                for (var i = 0; i < solution.length; i++) {
                    assert.equal(vec.at(i), solution.at(i));
                }
            })
        })

        describe('RowNorms Test', function () {
            it('should return vector with norm of i-th row as i-th element', function () {
                var vec = mat.rowNorms();
                var controlVec = new la.Vector([Math.pow(5, 0.5), Math.pow(25, 0.5)]);

                assert.deepEqual(vec, controlVec);
                for (var i = 0; i < controlVec.length; i++) {
                    assert.eqtol(vec.at(i), controlVec.at(i));
                }
            })
        })

        describe('ColNorms Test', function () {
            it('should return vector with norm of i-th column as i-th element', function () {
                var vec = mat.colNorms();
                var controlVec = new la.Vector([Math.pow(10, 0.5), Math.pow(20, 0.5)]);

                assert.deepEqual(vec, controlVec);
                for (var i = 0; i < controlVec.length; i++) {
                    assert.eqtol(vec.at(i), controlVec.at(i));
                }
            })
        })

        describe('NormalizeCols Test', function () {
            it('should normalize the columns of matrix mat', function () {
                var mat3 = new la.Matrix([[1, 2], [3, 4]]);
                mat3.normalizeCols();

                var controlMat = new la.Matrix([[1 / Math.pow(10, 0.5), 2 / Math.pow(20, 0.5)],
                    [3 / Math.pow(10, 0.5), 4 / Math.pow(20, 0.5)]]);

                assert.deepEqual(mat3, controlMat);
                for (var i = 0; i < controlMat.rows; i++) {
                    for (var j = 0; j < controlMat.cols; j++) {
                        assert.eqtol(mat3.at(i, j), controlMat.at(i, j));
                    }
                }
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



    })
});