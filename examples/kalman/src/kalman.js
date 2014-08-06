// Import analytics module
var analytics = require('analytics.js');
var tm = require('time.js');

// Kalman filter - linear model
console.say("*** KALMAN FILTER ***");
// initialize KF with a dynamic model (2nd degree)
var _DP = 3, _MP = 1, _CP = 0;
var kf = new analytics.kalmanFilter(_DP, _MP, _CP);
kf.setTransitionMatrix(la.eye(_DP));  // not needed
kf.setMeasurementMatrix(la.newMat([[1, 0, 0]]));
kf.setProcessNoiseCov(la.newMat([[1E-7, 0, 0], [0, 1E-7, 0], [0, 0, 1E-7]]));
kf.setMeasurementNoiseCov(la.newMat([[0.3]]));
kf.setErrorCovPre(la.newMat({ "cols": _DP, "rows": _DP, "random": false }));
kf.setErrorCovPost(la.newMat([[0.1, 0, 0], [0, 0.1, 0], [0, 0, 0.1]]));
kf.setControlMatrix(la.newMat());
   
// Load training data from CSV file.
var fin = fs.openRead("./sandbox/kalman/series.csv");
var header = fin.readLine(); var lines = 0;

// read first measurement
lines = lines + 1;
var line = fin.readLine();
var vals = line.split(',');
var lastTm = tm.parse(vals[1]);
var controlV = la.newVec();  // we don't need this
// set initial state to initial measurement (!) 
var value = parseFloat(vals[0]);
kf.setStatePost(la.newVec([value, 0, 0]));

while (!fin.eof) {
    lines = lines + 1;
    if (lines % 100 == 0) { console.log("Loaded: " + lines); }
    var line = fin.readLine();
    if (line == "") { continue; }

    var vals = line.split(',');
        
    // handle time
    
    var newTm = tm.parse(vals[1]);
    var deltaT = newTm.timestamp - lastTm.timestamp;
    var deltaT2 = deltaT * deltaT;
    kf.setTransitionMatrix(la.newMat([[1, deltaT, deltaT2], [0, 1, deltaT], [0, 0, 1]]));

    // handle value
    var value = parseFloat(vals[0]);
    var measurementV = la.newVec([value]);

    var predicted = kf.predict(controlV);
    var corrected = kf.correct(measurementV);

    // TODO: save value, prediction + correction
    var diff = value - predicted.at(0);
    console.log("Diff: " + diff + ", Value: " +  value.toFixed(6) + ", Prediction: " + predicted.at(0).toFixed(6) + ", Corr: " + corrected.at(0).toFixed(6));
        
    lastTm = newTm;
}


// Extended Kalman filter - non-linear model
console.say("*** EXTENDED KALMAN FILTER ***");
// we will simulate 2 * sin(3x) + N(x), where N(x) represents the noise
// initialize EKF with model A * sin(kf), which has 2 dynamic parameters
_DP = 2; _MP = 1; _CP = 0;
var _P = 1;
console.say("new");
var ekf = new analytics.extendedKalmanFilter(_DP, _MP, _CP, _P);

console.say("function defs");
var transitionEq = function () {   
    return this.getStatePost();
}

var observationEq = function () {
    myStatePost = this.getStatePost();
    myParameterV = this.getParameterV();
    o = myStatePost.at(0) * Math.sin(myStatePost.at(1) * myParameterV.at(0));
    return la.newVec([o]);
}

ekf.setTransitionEq(transitionEq);
ekf.setObservationEq(observationEq);

ekf.setTransitionMatrix(la.eye(_DP));  // not needed
ekf.setMeasurementMatrix(la.newMat([[1, 0]]));
ekf.setProcessNoiseCov(la.newMat([[1E-6, 0], [0, 1E-6]]));
ekf.setMeasurementNoiseCov(la.newMat([[0.1]]));
ekf.setErrorCovPre(la.newMat({ "cols": _DP, "rows": _DP, "random": false }));
ekf.setErrorCovPost(la.newMat([[1E-11, 0], [0, 1E-11]]));
ekf.setControlMatrix(la.newMat());

// initialization
var timestampN = 0;
ekf.setStatePost(la.newVec([1, 3.5]));
var controlV = la.newVec();  // we don't need this

var N = 0;
var DeltaT = 0.02;
while (N < 10000) {
    N++;
    x = N * DeltaT;
    // setting the parameter vector
    ekf.setParameterV(la.newVec([x]));
    myStatePost = ekf.getStatePost();
    ekf.setMeasurementMatrix(la.newMat([[Math.sin(myStatePost.at(0) * x), myStatePost.at(0) * x * Math.cos(myStatePost.at(1) * x)]]));

    // simulation
    var measurementV = la.newVec([2 * Math.sin(3 * x) + (Math.random() - 0.5) / 2]);

    var predicted = ekf.predict(controlV);
    var corrected = ekf.correct(measurementV);

    // TODO: save value, prediction + correction    
    console.log("A (2): " + predicted.at(0).toFixed(6) + ", k (3): " + predicted.at(1).toFixed(6));

}

// Start console
console.say("Interactive mode: empty line to release");
console.start();

