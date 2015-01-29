global
dir()
dir(obj, printVals, depth, width, prefix, showProto)
printj(obj)
exejs(fnm)
exejslocal(fnm)
rec = store[recId]
store.each(function (rec) { console.log(JSON.stringify(rec)); })
store.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })
arr = store.map(function (rec) { return JSON.stringify(rec); })
arr = store.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })
qm.storeProto
rec = rs[n]
rs.each(function (rec) { console.log(JSON.stringify(rec)); })
rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })
arr = rs.map(function (rec) { return JSON.stringify(rec); })
arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })
rec['fieldName'] = val
val = rec['fieldName']
rs = rec['joinName']
rec2 = rec['joinName']
vec = la.newVec({"vals":num, "mxvals":num2})
intVec = la.newIntVec({"vals":num, "mxvals":num2})
strVec = la.newStrVec({"vals":num, "mxvals":num2})
mat = la.newMat({"rows":num, "cols":num2, "random":bool})
spMat = la.newSpMat({"rows":num, "cols":num2})
svdRes = la.svd(mat, k, {"iter":num, "tol":num2})
svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})
num = vec[idx]; vec[idx] = num
spVec = spMat[colIdx]; spMat[colIdx] = spVec
la.spMat
tokenizer = analytics.newTokenizer({ type: <type>, ...})
result.target
result.confusion
result = newPULearning(trainMat, posVec, params)
gracePeriod
splitConfidence
tieBreaking
conceptDriftP
driftCheck
windowSize
maxNodes
attrDiscretization
clsAttrHeuristic
clsLeafModel
regLeafModel
sdrTreshold
phAlpha
phLambda
phInit
process.returnCode
utilities = require('utilities.js')
graph.eachNode(function (node) { console.log(node.id); })
graph.eachEdge(function (edge) { console.log(edge.srcId+" "+edge.dstId); })
