// 
// Unit tests for LA addon using standard JS assert library 
// 
// Need to:
// (*) add seperate unit tests for int and str vectors where appropriate;
// (*) rethink organization;
// (*) add missing unit tests for vectors;
// (*) add unit tests for matrix, sparse vector, and sparse matrix. 
//

var assert = require("assert")
var la = require("../../../build/Release/la.node")

describe('TNodeJsVec tests', function() {
    describe('Property Tests', function() {
        describe('Vector Length Test', function() {
            it('[FltV] should return 0 for empty vector', function() {
                var v = la.newVec();
                assert.equal(v.length, 0);
            })
            it('[FltV] should return 0 for vector [1, 1.1, 1.2]', function() {
                var v = la.newVec([1, 1.1, 1.2]);
                assert.equal(v.length, 3);
            })
            it('[IntV] should return 0 for empty int vector', function() {
                var v = la.newIntVec();
                assert.equal(v.length, 0);
            })
            it('[IntV] should return 3 for int vector [1, 2, 3]', function() {
                var v = la.newIntVec([1, 2, 3]);
                assert.equal(v.length, 3);
            })
        })
    })

    describe('Functions Tests', function() {
        describe('Push Test', function() {
            it('pushing 3.1 and 4 into a vector v, v.length should return 2', function() {
                var v = la.newVec();
                v.push(3.2);
                v.push(4);
                assert.equal(v.length, 2);
            })
        })

        describe('At Test', function() {
            it('[FltV] returns element in with indexes 0 (3.2) and 1 (4)', function() {
                var v = la.newVec([3.2, 4]);
                assert.equal(3.2, v.at(0));
                assert.equal(4, v.at(1));
            })
            it('[IntV] returns element in with indexes 0 (3) and 1 (4)', function() {
                var u = la.newIntVec([3, 4]);                
                
                assert.equal(3, u.at(0));
                assert.equal(4, u.at(1));
            })
        })

        describe('Sum Test', function() {
            it('should return a sum of 3.2 and 4', function() {
                var v = la.newVec([3.2, 4]);
                assert.equal(3.2+4, v.sum());
            })
        })

        describe('getMaxIdx Test', function() {
            it('should return index of last element in vector, 1.', function() {
                var v = la.newVec([3.2, 4]);
                assert.equal(v.length-1, v.getMaxIdx());
            })
        })

        describe('subVec Test', function() {
            var v = la.newVec([3.2, 4]);
            var u = la.newIntVec([1, 2]);
            it('[FltV] should return [3.2,4,3.2]', function() {
                var subV = v.subVec([0, 1, 0]);
                assert(subV.at(0) == subV.at(2) && subV.at(0) == v.at(0) && subV.at(1) == v.at(1));
            })
            it ('[FltV] should return empty vector', function() {
                var subV = v.subVec([]);
                assert(subV.length == 0);
            })
            it('[IntV] should return [1,2,1]', function() {
                var subV = u.subVec([0, 1, 0]);
                assert(subV.at(0) == subV.at(2) && subV.at(0) == u.at(0) && subV.at(1) == u.at(1));
            })
            it ('[IntV] should return empty vector', function() {
                var subV = u.subVec([]);
                assert(subV.length == 0);
            })
        })

        describe('put Test', function() {
            var v = la.newVec([3.2, 4]);
            var u = la.newIntVec([1, 2]);
            it('[FltV] should set v.at(1) to 2', function() {
                var retVal = v.put(1, 2);
                assert(v.at(1) == 2);
            })
            it('[IntV] should set v.at(1) to 5', function() {
                var retVal = v.put(1, 5);
                assert(v.at(1) == 5);
            })
        })

        describe('unshift Test', function() {
            var v = new la.vector([1, 1.5, 2.0]);
            var u = new la.vector([1, 2, 3]);
            it('[FltV] should add 5 to the beggining of v', function() {
                var lenV = v.unshift(5);
                assert(v.at(0) == 5);
                assert(lenV == v.length);
            })
            it('[IntV] should add 5 to the beggining of v', function() {
                var lenV = v.unshift(-2);
                assert(v.at(0) == -2);
                assert(lenV == v.length);
            })
        })

        describe('pushV Test', function() {
            it('[FltV] v should be [1,2,1,2]', function() {
                var u = la.newVec([1, 2]);
                var w = la.newVec([1, 2]);
                u.pushV(w);
                assert(u.length == 4 && u.at(0) == u.at(2) && u.at(1) == u.at(3));
            })
            it('[IntV] v should be [1,2,1,2]', function() {
                var u = la.newIntVec([1, 2]);
                var w = la.newIntVec([1, 2]);
                u.pushV(w);
                assert(u.length == 4 && u.at(0) == u.at(2) && u.at(1) == u.at(3));
            })
        })

        describe('sort Test', function() {
            var w = new la.vector([-3, -4, 1, 2, 3, 2, 5]);
            var u = w.sort();
            it('should return a sorted copy of the vector', function() {
                // Expect u to be a sorted copy of v 
                assert(u.at(0) == -4);
                assert(u.at(1) == -3);
                assert(u.at(2) == 1);
                assert(u.at(3) == 2);
                assert(u.at(4) == 2);
                assert(u.at(5) == 3);
                assert(u.at(6) == 5);
            })
            it('should leave the original vector unchanged', function() {
                // Leave w unchaged 
                assert(w.at(0) == -3);
                assert(w.at(1) == -4);
                assert(w.at(2) == 1);
                assert(w.at(3) == 2);
                assert(w.at(4) == 3);
                assert(w.at(5) == 2);
                assert(w.at(6) == 5);
            })
        })

        describe('sortPerm Test', function() {
            var w = new la.vector([-3, -4, 1, 2, 3, 2, 5]);
            var u = w.sortPerm();
            it('should return a sorted copy of the vector', function() {
                // Expect u.vec to be a sorted copy of v 
                assert(u.vec.at(0) == -4);
                assert(u.vec.at(1) == -3);
                assert(u.vec.at(2) == 1);
                assert(u.vec.at(3) == 2);
                assert(u.vec.at(4) == 2);
                assert(u.vec.at(5) == 3);
                assert(u.vec.at(6) == 5);
            })
            it('should return a permutation of the indices of the vector', function() {
                // Expect u.perm to be the permutation of sorted indices 
                assert(u.perm.at(0) == 1);
                assert(u.perm.at(1) == 0);
                assert(u.perm.at(2) == 2);
                assert(u.perm.at(3) == 3);
                assert(u.perm.at(4) == 5);
                assert(u.perm.at(5) == 4);
                assert(u.perm.at(6) == 6);
            })
            it('should leave the original vector unchanged', function() {
                // Leave w unchaged 
                assert(w.at(0) == -3);
                assert(w.at(1) == -4);
                assert(w.at(2) == 1);
                assert(w.at(3) == 2);
                assert(w.at(4) == 3);
                assert(w.at(5) == 2);
                assert(w.at(6) == 5);
            })
        })

        /*
        // XXX: Very "basic" 
        describe('shuffle Test', function() {
            it('todo', function(){ assert(false); })
        })

        describe('trunc Test', function() {
            it('todo', function(){ assert(false); })
        })

        describe('outer Test', function() {
            it('todo', function(){ assert(false); })
        })
        */

        describe('inner Test', function() {
            var u1 = new la.vector([1, 2, 3]);
            var u2 = new la.vector([1, 2, 3]);
            it('should return 14', function() {
                assert(u1.inner(u2) == u2.inner(u1));
                assert(u1.inner(u2) == 14);
            })
            it('Should return 0', function() {
                var w1 = new la.vector();
                var w2 = new la.vector();
                assert(w1.inner(w2) == 0);
            })
        })

        describe('plus Test', function() {
            it('should return [2,2,2]', function() {
                var v1 = la.newVec([1, 0.5, 2.1]);
                var v2 = la.newVec([1, 1.5, -0.1]);
                var u = v1.plus(v2);
                assert(u.at(0) == 2 && u.at(1) == 2 && u.at(2) == 2);
            })
        })

        describe('minus Test', function() {
            it('should return [0,0,0]', function() {
                var v1 = la.newVec([1, 1.5, -1.2]);
                var u = v1.minus(v1);
                assert(u.at(0) == 0 && u.at(1) == 0 && u.at(2) == 0);
            })
        })

        describe('multiply Test (mupltiplying vector with a scalar)', function() {
            it('should return [3, 6, 9]', function() {
                var v1 = la.newVec([1, 2, 3]);
                var u = v1.multiply(3.0);
                assert(u.at(0) == 3.0 && u.at(1) == 6.0 && u.at(2) == 9.0);
            })
        })

        describe('normalize Test', function() {
            it('should scale [1.0, -1.0] by 1/sqrt(2)', function() {
                var v1 = la.newVec([1.0, -1.0]);
                v1.normalize();
                assert(Math.abs(v1.norm()-1) < 1e-6 && v1.at(0) == -v1.at(1));
            })
        })

        describe('length Test', function() {
            it('should return 3', function() {
                var v1 = la.newVec([1, 2, 3]);
                assert(v1.length == 3);
            })
        })

        describe('toString Test', function() {
            it('should return [1, 1.5, 2]', function() {
                var v1 = la.newVec([1, 1.5, 2]);
                assert(v1.toString() == "[1, 1.5, 2]");
            })
            it('should return [1, 1, 2]', function() {
                var v1 = la.newIntVec([1, 1, 2]);
                assert(v1.toString() == "[1, 1, 2]");
            })
        })

        // TODO: Add test for empty vector
        describe('diag Test', function(){
            var u = new la.vector([0,1,2,3,4,5]);
            var M = u.diag();
            it('the number of rows and number of columns should equal the number of elements in u', function(){
                assert(M.cols = u.length && M.rows == u.length);
            })
            it('should return a diagnoal matrix', function(){
                for (var i = 0; i < M.rows; ++i) {
                    for (var j = 0; j < M.cols; ++j) {
                        if (i == j) { assert(M.at(i,j) == u.at(i)); }
                        else { assert(M.at(i, j) == 0); }
                    }
                }
            })
        })

        /*
        describe('spDiag Test', function(){
            it('todo', function(){ assert(false); })
        })
        */

        // TODO: Add test for empty vector 
        describe('norm Test', function(){
            var u = new la.vector([1, 1]);
            it('should be ||u|| = sqrt(2)', function(){
                assert(u.norm() == Math.sqrt(2));
            })
        })

        /*
        describe('sparse Test', function(){
            it('todo', function(){ assert(false); })
        })

        describe('toMat Test', function(){
            it('todo', function(){ assert(false); })
        })
        */
    })
});

