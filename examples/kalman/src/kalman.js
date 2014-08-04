// Import analytics module
var la = require('linalg.js');
var analytics = require('analytics.js');
var tm = require('time.js');

// initialize KF with a dynamic model (2nd degree)
var _DP = 3, _MP = 1, _CP = 0;
var kf = new la.kalmanFilter(_DP, _MP, _CP);
kf.transitionMatrix = la.eye(3);
kf.measurementMatrix = la.eye(1);
kf.processNoiseCov = la.newMat([[1E-7, 0, 0], [0, 1E-7, 0], [0, 0, 1E-7]]);
kf.measurementNoiseCov = la.newMat([[0.3]]);
kf.errorCovPost = la.newMat([[0.1, 0, 0], [0, 0.1, 0], [0, 0, 0.1]]);
   
// Load training data from CSV file.
var fin = fs.openRead("./sandbox/kalman/series.csv");
var header = fin.readLine(); var lines = 0;

// read first measurement
lines = lines + 1;
var line = fin.readLine();
var vals = line.split(',');
var lastTm = tm.parse(vals[1]);
var controlV = la.newFltVec(_CP);  // we don't need this
// set initial state to initial measurement (!) 
kf.statePost = [parseFloat(vals[0]), 0, 0];

while (!fin.eof) {
    lines = lines + 1;
    if (lines % 1000 == 0) { console.log("Loaded: " + lines); }
    var line = fin.readLine();
    if (line == "") { continue; }
    try {
        var vals = line.split(',');
        
        // handle time
        var newTm = tm.parse(vals[1]);
        var deltaT = newTm - lastTm;
        var deltaT2 = deltaT * deltaT;
        kf.transitionMatrix = la.newMat([[1, deltaT, deltaT2], [0, 1, deltaT], [0, 0, 1]]);

        // handle value
        var value = la.newFltVec([parseFloat(vals[0]), 0, 0]);

        var predicted = kf.predict(controlV);
        var corrected = kf.correct(measurementV)

        // TODO: save value, prediction + correction

        //console.pause();
        lastTm = newTm;

    } catch (err) {
        console.say("Kalman", err);
    }
}

// Start console
console.say("Interactive mode: empty line to release");
console.start();

