var la = require('../../index.js').la;
// Generate a normal vector (3 iid standard normal samples, implemented in linalg.js)
var v = la.randn(3);
// Print vector
console.log("Random vector v:");
v.print();
// Generate a random matrix (uniform random entries)
var A = new la.Matrix({ "rows": 3, "cols": 3, "random": true });
// Use power iteration to find the leading eigenvector, with 50 iterations
for (var i = 0; i < 50; i++) {
    v = A.multiply(v);
    v.normalize();
}
// Eigenvalue
var lambda = A.multiply(v).norm();
// Print ||A*v - lambda *v||_2 (should be close to zero if v,lambda is an eigenvector,eigenvalue pair for A
console.log("Norm of residual:\n ||v*lambda - A*v||_2 = " + v.multiply(lambda).minus(A.multiply(v)).norm());
// Truncated singular value decomposition, k = 1 (best rank 1 decomposition):
var res = la.svd(A, 1);
// How close is rank-1 SVD to A?
console.log("Rank-1 SVD: ||A - U*diag(s)*V^T||_F = " + (res.U.multiply(res.s.spDiag()).multiply(res.V.transpose()).minus(A)).frob());
var res = la.svd(A, 2);
// How close is rank-2 SVD to A?
console.log("Rank-2 SVD: ||A - U*diag(s)*V^T||_F = " + (res.U.multiply(res.s.spDiag()).multiply(res.V.transpose()).minus(A)).frob());

// Generate a dense matrix from JS array and convert it to sparse
var B = new la.Matrix([[1, 0], [3, 4]]).sparse();
// Get the number of non-zero elements (implemented in spMat.js)
console.log("B.nnz() = " + B.nnz());
// Get the second column of B(sparse vector) and print it:
console.log("Second column of B (row index, value representation): ");
B[1].print();
// Create a symmetric positive definite matrix (so that we can test the conjugate gradient method):
A = A.multiply(A.transpose());
// Linear system: solve A*x=b
// Generate true solution
var x = la.randn(3);
// Generate RHS
var b = A.multiply(x);
// Solve the system (dense matrix only)
var x2 = A.solve(b);
// Compare the solution x2 to the true x
console.log("Generate x. Set b := A*x; Compute x2 := A^(-1) b")
console.log("||x - x2|| = " + x.minus(x2).norm());
// Solve using CG (works for sparse and dense matrices, since it is a black box method)
// Create a random initial vector
var x0 = la.randn(3);
var x3 = la.conjgrad(A.sparse(), b, x0); // convert A to a sparse matrix and test the CG method
console.log("||x3 - x2|| = " + x3.minus(x2).norm());
// Inverse matrix with SVD
var M = new la.Matrix({ "rows": 3, "cols": 3, "random": true });
var invM = la.inverseSVD(M);
P = M.multiply(invM);
console.log("Calculating M^{-1}. I = M * M^{-1}. Do we get an identity?");
P.print();
