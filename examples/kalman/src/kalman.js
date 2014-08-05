// Import analytics module
var analytics = require('analytics.js');
var tm = require('time.js');

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
    
    //console.pause();
    lastTm = newTm;
}

// Start console
console.say("Interactive mode: empty line to release");
console.start();

