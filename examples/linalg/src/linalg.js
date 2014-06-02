// Generate a normal vector (3 iid standard normal samples, implemented in linalg.js)
var v = la.genRandomVector(3);
// Print vecotr (implemented in qminer_js.h)
console.say("Random vector v:");
v.print();
// Generate a random matrix (uniform random entries)
var A = la.newMat({ "rows": 3, "cols": 3, "random": true });
// Use power iteration to find the leading eigenvector, with 50 iterations
for (var i = 0; i < 50; i++) {
    v = A.multiply(v);
    v.normalize();
}
// Eigenvalue
var lambda = A.multiply(v).norm();
// Print ||A*v - lambda *v||_2 (should be close to zero if v,lambda is an eigenvector,eigenvalue pair for A
console.say("Norm of residual:\n ||v*lambda - A*v||_2 = " + v.multiply(lambda).minus(A.multiply(v)).norm());
// Truncated singular value decomposition, k = 1 (best rank 1 decomposition):
var res = la.svd(A, 1);
// How close is rank-1 SVD to A?
console.say("Rank-1 SVD: ||A - U*diag(s)*V^T||_F = " + (res.U.multiply(res.s.spDiag()).multiply(res.V.transpose()).minus(A)).frob());
var res = la.svd(A, 2);
// How close is rank-2 SVD to A?
console.say("Rank-2 SVD: ||A - U*diag(s)*V^T||_F = " + (res.U.multiply(res.s.spDiag()).multiply(res.V.transpose()).minus(A)).frob());


// Generate a dense matrix from JS array and convert it to sparse
var B = la.newMat([[1, 0], [3, 4]]).sparse();
// Get the number of non-zero elements (implemented in spMat.js)
console.say("B.nnz() = " + B.nnz());
//Get the second column of B(sparse vector) and print it:
console.say("Second column of B (row index, value representation): ");
B[1].print();

//a.print();
//a.toString();
//a.nnz();
console.start();
