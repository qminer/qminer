analytics = require('analytics');

// NEURAL NETWORKS XOR EXAMPLE ----------------------------------
var NN = analytics.newNN({"layout": [2,4,1]});
for(var i = 0; i < 35000; ++i){
    var in1 = Math.round(Math.random())
    var in2 = Math.round(Math.random())
    var out1 = 0
    if(!in1 ^ !in2)
        out1 = 1
    var inArr = linalg.newVec([in1, in2])
    var outArr = linalg.newVec([out1])
    //console.log("In 1: " + in1 + " In 2: " + in2)
    //console.log("Target: " + out1)
    var predictions = NN.predict(inArr)
    //console.log("Result: " + predictions[0])
    //console.log("Diff: " + (out1 - predictions[0]))
    NN.learn(inArr,outArr);
}

// NEURAL NETWORKS SINE EXAMPLE ----------------------------------

var NN = analytics.newNN({"layout": [1,4,1], "tFuncHidden":"tanHyper", "tFuncOut":"linear", "learnRate":0.2, "momentum":0.5});
for(var i = 0; i < 100; i += 0.01){
    var out = Math.sin(i) * 6 + 30

    var inArr = linalg.newVec([i])
    var outArr = linalg.newVec([out])
    console.log("In 1: " + i)
    console.log("Target: " + out)
    var predictions = NN.predict(inArr)
    console.log("Result: " + predictions[0])
    console.log("Diff: " + (out - predictions[0]))
    NN.learn(inArr,outArr);
}