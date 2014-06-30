analytics = require('analytics');

// NEURAL NETWORKS XOR EXAMPLE ----------------------------------
// Create a new net, specify it's layout, eg. how many neurons do we want in each layer. There are more options available which are explained in the next example.
var NN = analytics.newNN({"layout": [2,4,1]});
// create a loop for learning the net
for(var i = 0; i < 2000; ++i){
    // get two random numbers 0 or 1, this is the input data
    var in1 = Math.round(Math.random())
    var in2 = Math.round(Math.random())
    // perform an xor on the variables, this will be the target value
    var out1 = 0
    if(!in1 ^ !in2)
        out1 = 1
    // for learning the net the data must be in vector form, so we create vectors.
    // dimensions of the vectors should match the dimensions of the input and output layers.
    var inVec = linalg.newVec([in1, in2])
    var outVec = linalg.newVec([out1])
    //console.log("In 1: " + in1 + " In 2: " + in2)
    //console.log("Target: " + out1)
    // first we predict based on the inputs (feed-forward)
    var predictions = NN.predict(inVec)
    //console.log("Result: " + predictions[0])
    //console.log("Diff: " + (out1 - predictions[0]))
    // then we learn the net with inputs and expected outputs (back propagation)
    NN.learn(inVec,outVec);
}

// NEURAL NETWORKS SINE EXAMPLE ----------------------------------
// Create a new net, set the layout, activation functions in hidden and output layer, learning rate and momentum
var NN = analytics.newNN({"layout": [1,4,1], "tFuncHidden":"tanHyper", "tFuncOut":"linear", "learnRate":0.2, "momentum":0.5});
// create a loop for learning
for(var i = 0; i < 100; i += 0.01){
    // calculate target value
    var out = Math.sin(i) * 6 + 30
    // create vectors to feed the net
    var inVec = linalg.newVec([i])
    var outVec = linalg.newVec([out])

    console.log("In 1: " + i)
    console.log("Target: " + out)
    // predict based on the inputs
    var predictions = NN.predict(inVec)
    console.log("Result: " + predictions[0])
    console.log("Diff: " + (out - predictions[0]))
    // learn based on inputs and expected outputs
    NN.learn(inVec,outVec);
}