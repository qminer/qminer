require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the analyticsdoc.js file', function () {
describe("Analytics module, number 1", function () {
it("should make test number 1", function () {

  // import module, load dataset, create model, evaluate model
 
});
});
describe("SV, number 2", function () {
it("should make test number 2", function () {

 // import modules
 var la = require('qminer').la;
 var analytics = require('qminer').analytics;
 // CLASSIFICATION WITH SVC
 // set up fake train and test data
 // four training examples with number of features = 2
 var featureMatrix = new la.Matrix({ rows: 2, cols: 4, random: true });
 // classification targets for four examples
 var targets = new la.Vector([-1, -1, 1, 1]);
 // set up the classification model
 var SVC = new analytics.SVC({ verbose: false });
 // train classifier
 SVC.fit(featureMatrix, targets);
 // set up a fake test vector
 var test = new la.Vector([1.1, -0.5]);
 // predict the target value
 var prediction = SVC.predict(test);

});
});
describe("Returns the SVC parameters, number 3", function () {
it("should make test number 3", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVC model with json
	 var SVC = new analytics.SVC({ c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true });
	 // get the parameters of the SVC model
	 // returns { algorithm: 'SGD' c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true }
	 var json = SVC.getParams(); 
	
});
});
describe("Sets the SVC parameters, number 4", function () {
it("should make test number 4", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a default SVC model
	 var SVC = new analytics.SVC();
	 // change the parameters of the SVC with the json { j: 5, maxIterations: 12000, minDIff: 1e-10 }
	 SVC.setParams({ j: 5, maxIterations: 12000, minDiff: 1e-10 }); // returns self
	
});
});
describe("Gets the vector of coefficients of the linear model, number 5", function () {
it("should make test number 5", function () {
 
	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix.
	 var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	 var vec = new la.Vector([1, 1, -1, -1]);
	 // fit the model
	 SVC.fit(matrix, vec);
	 // get the weights
	 var weights = SVC.weights; // returns the coefficients of the normal vector of the hyperplane gained from the model: [1, 1]
	
});
});
describe("Saves model to output file stream, number 6", function () {
it("should make test number 6", function () {

	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 var fs = require('qminer').fs;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix column.
	 var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);	
	 var vec = new la.Vector([1, 0, -1, -2]);
	 // fit the model
	 SVC.fit(matrix, vec);
	 // create output stream
	 var fout = fs.openWrite('svc_example.bin');
	 // save SVC object (model and parameters) to output stream and close it
	 SVC.save(fout);
	 fout.close();
	 // create input stream
	 var fin = fs.openRead('svc_example.bin');
	 // create a SVC object that loads the model and parameters from input stream
	 var SVC2 = new analytics.SVC(fin);	
	
});
});
describe("Sends vector through the model and returns the distance to the decision boundery, number 7", function () {
it("should make test number 7", function () {

	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix
	 var matrix = new la.Matrix([[1, 0], [0, -1]]);
	 var vec = new la.Vector([1, -1]);
	 // fit the model
	 SVC.fit(matrix, vec);
	 // create the vector you want to get the distance from the model
	 var vec2 = new la.Vector([2, 3]);
	 // use the decisionFunction to get the distance of vec2 from the model
	 var distance = SVC.decisionFunction(vec2); // returns something close to 5
	
});
});
describe("Sends vector through the model and returns the prediction as a real number, number 8", function () {
it("should make test number 8", function () {

	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix
	 var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	 var vec = new la.Vector([1, 1, -1, -1]);
	 // fit the model
	 SVC.fit(matrix, vec);
	 // create a vector you want to predict 
	 var vec2 = new la.Vector([3, 5]);
	 // predict the vector
	 var prediction = SVC.predict(vec2); // returns 1
	
});
});
describe("Fits a SVM classification model, given column examples in a matrix and vector of targets, number 9", function () {
it("should make test number 9", function () {

	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix.
	 var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	 var vec = new la.Vector([1, 1, -1, -1]);
	 // fit the model
	 SVC.fit(matrix, vec); // creates a model, where the hyperplane has the normal semi-equal to [1, 1]
	
});
});
describe("SV, number 10", function () {
it("should make test number 10", function () {

 // import module
 var analytics = require('qminer').analytics;
 var la = require('qminer').la;
 // REGRESSION WITH SVR
 // Set up fake train and test data.
 // Four training examples with, number of features = 2
 var featureMatrix = new la.Matrix({ rows: 2, cols: 4, random: true });
 // Regression targets for four examples
 var targets = new la.Vector([1.1, -2, 3, 4.2]);
 // Set up the regression model
 var SVR = new analytics.SVR({ verbose: false });
 // Train regression
 SVR.fit(featureMatrix, targets);
 // Set up a fake test vector
 var test = new la.Vector([1.1, -0.8]);
 // Predict the target value
 var prediction = SVR.predict(test);

});
});
describe("Returns the SVR parameters, number 11", function () {
it("should make test number 11", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVR object
	 var SVR = new analytics.SVR({ c: 10, eps: 1e-10, maxTime: 12000, verbose: true });
	 // get the parameters of SVR
	 var params = SVR.getParams();
	
});
});
describe("Sets the SVR parameters, number 12", function () {
it("should make test number 12", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVR object
	 var SVR = new analytics.SVR();
	 // set the parameters of the SVR object
	 SVR.setParams({ c: 10, maxTime: 12000 });
	
});
});
describe("Saves model to output file stream, number 13", function () {
it("should make test number 13", function () {

	 // import the modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 var fs = require('qminer').fs;
	 // create a new SVR object
	 var SVR = new analytics.SVR({ c: 10 });
	 // create a matrix and vector for the model
	 var matrix = new la.Matrix([[1, -1], [1, 1]]);
	 var vector = new la.Vector([1, 1]);
	 // create the model by fitting the values
	 SVR.fit(matrix, vector);
	 // save the model in a binary file
	 var fout = fs.openWrite('svr_example.bin');
	 SVR.save(fout);
	 fout.close();
	 // construct a SVR model by loading from the binary file
	 var fin = fs.openRead('svr_example.bin');
	 var SVR2 = new analytics.SVR()
	
});
});
describe("Sends vector through the model and returns the scalar product as a real number, number 14", function () {
it("should make test number 14", function () {

	  // import the modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create a new SVR object
	  var SVR = new analytics.SVR({ c: 10 });
	  // create a matrix and vector for the model
	  var matrix = new la.Matrix([[1, -1], [1, 1]]);
	  var vector = new la.Vector([1, 1]);
	  // create the model by fitting the values
	  SVR.fit(matrix, vector);
	  // get the distance between the model and the given vector
	  var vec2 = new la.Vector([-5, 1]);
	  var distance = SVR.decisionFunction(vec2);
     
});
});
describe("Sends vector through the model and returns the prediction as a real number, number 15", function () {
it("should make test number 15", function () {

	 // import the modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVR object
	 var SVR = new analytics.SVR({ c: 10 });
	 // create a matrix and vector for the model
	 var matrix = new la.Matrix([[1, -1], [1, 1]]);
	 var vector = new la.Vector([1, 1]);
	 // create the model by fitting the values
	 SVR.fit(matrix, vector);
	 // predict the value of the given vector
	 var vec2 = new la.Vector([-5, 1]);
	 var prediction = SVR.predict(vec2);
	
});
});
describe("fits an SVM regression model, given column examples in a matrix and vector of target, number 16", function () {
it("should make test number 16", function () {

	 // import the modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new SVR object
	 var SVR = new analytics.SVR({ c: 10 });
	 // create a matrix and vector for the model
	 var matrix = new la.Matrix([[1, -1], [1, 1]]);
	 var vector = new la.Vector([1, 1]);
	 // create the model by fitting the values
	 SVR.fit(matrix, vector);
	
});
});
describe("Ridge regression. Minimizes: ||A' x - b||^2 + ||gamma x||^, number 17", function () {
it("should make test number 17", function () {

  // import modules
  la = require('qminer').la;
  analytics = require('qminer').analytics;
  // create a new model with gamma = 1.0
  var regmod = new analytics.RidgeReg({ gamma: 1.0 });
  // generate a random feature matrix
  var A = la.randn(10,100);
  // generate a random model
  var w = la.randn(10);
  // generate noise
  var n = la.randn(100).multiply(0.01);
  // generate responses (model'*data + noise)
  var b = A.transpose().multiply(w).plus(n);
  // fit model
  regmod.fit(A, b);
  // compare
  // true model
  w.print();
  // trained model');
  regmod.weights.print();
  // cosine between the true and the estimated model should be close to 1 if the fit succeeded
  var cos = regmod.weights.cosine(w);
 
});
});
describe("Gets the parameters, number 18", function () {
it("should make test number 18", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Ridge Regression object
	 var regmod = new analytics.RidgeReg({ gamma: 5 });
	 // get the parameters
	 // returns a json object { gamma: 5 }
	 var param = regmod.getParams();
	
});
});
describe("Set the parameters, number 19", function () {
it("should make test number 19", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Ridge Regression object
	 var regmod = new analytics.RidgeReg({ gamma: 5 });
	 // set the parameters of the object
	 var param = regmod.setParams({ gamma: 10 });
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y, number 20", function () {
it("should make test number 20", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create a new Ridge Regression object
	  var regmod = new analytics.RidgeReg();
	  // create the test matrix and vector
	  var X = new la.Matrix([[1, 2], [1, -1]]);
	  var y = new la.Vector([3, 3]);
	  // fit the model with X and y
	  // the weights of the model are 2, 1
	  regmod.fit(X, y);
     
});
});
describe("Returns the expected response for the provided feature vector, number 21", function () {
it("should make test number 21", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create a new Ridge Regression object
	  var regmod = new analytics.RidgeReg();
	  // create the test matrix and vector
	  var X = new la.Matrix([[1, 2], [1, -1]]);
	  var y = new la.Vector([3, 3]);
	  // fit the model with X and y
	  regmod.fit(X, y);
	  // create a new vector for the prediction
	  var vec = new la.Vector([3, 4]);
	  // create the prediction
	  // returns the value 10
	  var prediction = regmod.decisionFunction(vec);
     
});
});
describe("Returns the expected response for the provided feature vector, number 22", function () {
it("should make test number 22", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create a new Ridge Regression object
	  var regmod = new analytics.RidgeReg();
	  // create the test matrix and vector
	  var X = new la.Matrix([[1, 2], [1, -1]]);
	  var y = new la.Vector([3, 3]);
	  // fit the model with X and y
	  regmod.fit(X, y);
	  // create a new vector for the prediction
	  var vec = new la.Vector([3, 4]);
	  // create the prediction
	  // returns the value 10
	  var prediction = regmod.predict(vec);
     
});
});
describe("Saves the model into the output stream, number 23", function () {
it("should make test number 23", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  var fs = require('qminer').fs;
	  // create a new Ridge Regression object
	  var regmod = new analytics.RidgeReg();
	  // create the test matrix and vector
	  var X = new la.Matrix([[1, 2], [1, -1]]);
	  var y = new la.Vector([3, 3]);
	  // fit the model with X and y
	  regmod.fit(X, y);
	  // create an output stream object and save the model
	  var fout = fs.openWrite('regmod_example.bin');
	  regmod.save(fout);
	  fout.close();
	  // create a new Ridge Regression model by loading the model
	  var fin = fs.openRead('regmod_example.bin');
	  var regmod2 = new analytics.RidgeReg(fin);
     
});
});
describe("Sigmoid function (y = 1/[1 + exp[, number 24", function () {
it("should make test number 24", function () {

  // import modules
  la = require('qminer').la;
  analytics = require('qminer').analytics;
  // create a new model
  var sigmoid = new analytics.Sigmoid();
  // generate a random predictions
  var x = new la.Vector([0.5, 2.3, -0.1, 0.5, -7.3, 1.2]);
  // generate a random labels
  var y = new la.Vector([1, 1, -1, 1, -1, -1]);
  // fit model
  sigmoid.fit(x, y);
  // get predictions
  var pred1 = sigmoid.predict(1.2);
  var pred2 = sigmoid.predict(-1.2);
 
});
});
describe("Get the parameters. It doesn't do anything, it's only for consistency for constructing pipeline, number 25", function () {
it("should make test number 25", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // get the parameters
	 // returns an empty Json object
	 var param = s.getParams();
	
});
});
describe("Sets the parameters. It doesn't do anything, it's only for consistency for constructing pipeline, number 26", function () {
it("should make test number 26", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // set the parameters 
	 // doesn't change the model
	 s.setParams({});
	
});
});
describe("Gets the model, number 27", function () {
it("should make test number 27", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // get the model parameters
	 // returns a Json object { A: 0, B: 0 }
	 var model = s.getModel();
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y, number 28", function () {
it("should make test number 28", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the Sigmoid model
	  var s = new analytics.Sigmoid();
	  // create the predicted values and the binary labels
	  var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	  var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	  // fit the model
	  // changes the internal A and B values of the model 
	  // (these values can be obtained with the getModel method)
	  s.fit(X, y);
     
});
});
describe("Returns the expected response for the provided feature vector, number 29", function () {
it("should make test number 29", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the Sigmoid model
	  var s = new analytics.Sigmoid();
	  // create the predicted values and the binary labels
	  var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	  var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	  // fit the model
	  s.fit(X, y);
	  // predict the probability of the value 0 on this model
	  // returns 0.5
	  var prediction = s.decisionFunction(0.5);
     
});
});
describe("Returns the expected response for the provided feature vector, number 30", function () {
it("should make test number 30", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the Sigmoid model
	  var s = new analytics.Sigmoid();
	  // create the predicted values and the binary labels
	  var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	  var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	  // fit the model
	  s.fit(X, y);
	  // predict the probability of the value 0 on this model
	  // returns 0.5
	  var prediction = s.predict(0.5);
     
});
});
describe("Saves the model into the output stream, number 31", function () {
it("should make test number 31", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  var fs = require('qminer').fs;
	  // create the Sigmoid model
	  var s = new analytics.Sigmoid();
	  // create the predicted values and the binary labels
	  var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
	  var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
	  // fit the model
	  s.fit(X, y);
	  // create an output stream object and save the model
	  var fout = fs.openWrite('sigmoid_example.bin');
	  s.save(fout);
	  fout.close();
	  // create a new Sigmoid model by loading the model
	  var fin = fs.openRead('sigmoid_example.bin');
	  var s2 = new analytics.Sigmoid(fin);
     
});
});
describe("Nearest Neighbour Anomaly Detection, number 32", function () {
it("should make test number 32", function () {

  // import modules
  var analytics = require('qminer').analytics;
  var la = require('qminer').la;
  // create a new NearestNeighborAD object
  var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
  // create a sparse matrix 
  var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
  // fit the model with the matrix
  neighbor.fit(matrix);
  // create a new sparse vector
  var vector = new la.SparseVector([[0, 4], [1, 0]]);
  // predict if the vector is an anomaly or not
  var prediction = neighbor.predict(vector);
 
});
});
describe("Sets parameters, number 33", function () {
it("should make test number 33", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // set it's parameters to rate: 0.1
	 neighbor.setParams({ rate: 0.1 });
	
});
});
describe("Returns parameters, number 34", function () {
it("should make test number 34", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // get the parameters of the object
	 // returns a json object { rate: 0.05 }
	 var params = neighbor.getParams();
	
});
});
describe("Save model to provided output stream, number 35", function () {
it("should make test number 35", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  var fs = require('qminer').fs;
	  // create a new NearestNeighborAD object
	  var neighbor = new analytics.NearestNeighborAD();
	  // create a new sparse matrix
	  var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	  // fit the model with the matrix
	  neighbor.fit(matrix);
	  // create an output stream object and save the model
	  var fout = fs.openWrite('neighbor_example.bin');
	  neighbor.save(fout);
	  fout.close();
	  // create a new Nearest Neighbor Anomaly model by loading the model
	  var fin = fs.openRead('neighbor_example.bin');
	  var neighbor2 = new analytics.NearestNeighborAD(fin);
     
});
});
describe("Returns the model, number 36", function () {
it("should make test number 36", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
	 // get the model of the object
	 // returns a json object { rate: 0.1, window: 0 }
	 var model = neighbor.getModel();
	
});
});
describe("Adds a new point to the known points and recomputes the threshold, number 37", function () {
it("should make test number 37", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // create a new sparse matrix
	 var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	 // fit the model with the matrix
	 neighbor.fit(matrix);
	 // create a new sparse vector
	 var vector = new la.SparseVector([[0, 2], [1, 5]]);
	 // update the model with the vector
	 neighbor.partialFit(vector);
	
});
});
describe("Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter, number 38", function () {
it("should make test number 38", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // create a new sparse matrix
	 var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	 // fit the model with the matrix
	 neighbor.fit(matrix);
	
});
});
describe("Compares the point to the known points and returns distance to the nearest one, number 39", function () {
it("should make test number 39", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create a new NearestNeighborAD object
	  var neighbor = new analytics.NearestNeighborAD();
	  // create a new sparse matrix
	  var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	  // fit the model with the matrix
	  neighbor.fit(matrix);
	  // create a new sparse vector
	  var vector = new la.SparseVector([[0, 4], [1, 0]]);
	  // get the distance of the vector from the model
	  var prediction = neighbor.decisionFunction(vector); // returns 1
	 
});
});
describe("Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold), number 40", function () {
it("should make test number 40", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // create a new sparse matrix
	 var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	 // fit the model with the matrix
	 neighbor.fit(matrix);
	 // create a new sparse vector
	 var vector = new la.SparseVector([[0, 4], [1, 0]]);
	 // check if the vector is an anomaly
	 var prediction = neighbor.predict(vector); // returns 1
	
});
});
describe("Returns a JSON object that encodes the ID of the nearest neighbor and the features that contributed to the distanc, number 41", function () {
it("should make test number 41", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD({rate:0.05, windowSize:3});
	 // create a new sparse matrix
	 var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
	 // fit the model with the matrix and provide a vector record IDs
	 neighbor.fit(matrix, new la.IntVector([3541,1112,4244]));
	 // create a new sparse vector
	 var vector = new la.SparseVector([[0, 4], [1, 0]]);
	 // check if the vector is an anomaly
	 var explanation = neighbor.explain(vector); // returns an explanation
	
});
});
describe("Recursive Linear Regressio, number 42", function () {
it("should make test number 42", function () {

 // import analytics module
 var analytics = require('qminer').analytics;
 // create the recursive linear regression model holder
 var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });

});
});
describe("Creates a partial fit of the input, number 43", function () {
it("should make test number 43", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create the Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 3.0 });
	 // create a new dense vector
	 var vec = new la.Vector([1, 2, 3]);
	 // fit the model with the vector
	 linreg.partialFit(vec, 6);
	
});
});
describe("Creates a fit of the input, number 44", function () {
it("should make test number 44", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create the Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 2.0 });
	 // create a new dense matrix and target vector
	 var mat = new la.Matrix([[1, 2, 3], [3, 4, 5]]);
	 var vec = new la.Vector([3, 5, -1]);
	 // fit the model with the matrix
	 linreg.fit(mat, vec);
	
});
});
describe("Puts the vector through the model and returns the prediction as a real number, number 45", function () {
it("should make test number 45", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create the Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 2.0, recFact: 1e-10 });
	 // create a new dense matrix and target vector
	 var mat = new la.Matrix([[1, 2], [1, -1]]);
	 var vec = new la.Vector([3, 3]);
	 // fit the model with the matrix
	 linreg.fit(mat, vec);
	 // create the vector to be predicted
	 var pred = new la.Vector([1, 1]);
	 // predict the value of the vector
	 var prediction = linreg.predict(pred); // returns something close to 3.0
	
});
});
describe("Sets the parameters of the model, number 46", function () {
it("should make test number 46", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 10 });
	 // set the parameters of the model
	 linreg.setParams({ dim: 3, recFact: 1e2, forgetFact: 0.5 });
	
});
});
describe("Returns the parameters, number 47", function () {
it("should make test number 47", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 10 });
	 // get the parameters of the model
	 var params = linreg.getParams(); // returns { dim: 10, recFact: 1.0, forgetFact: 1.0 }
	
});
});
describe("Save model to provided output stream, number 48", function () {
it("should make test number 48", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 var fs = require('qminer').fs;
	 // create the Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 2.0, recFact: 1e-10 });
	 // create a new dense matrix and target vector
	 var mat = new la.Matrix([[1, 2], [1, -1]]);
	 var vec = new la.Vector([3, 3]);
	 // fit the model with the matrix
	 linreg.fit(mat, vec);
	 // create an output stream object and save the model
	 var fout = fs.openWrite('linreg_example.bin');
	 linreg.save(fout);
	 fout.close();
	 // create a new Nearest Neighbor Anomaly model by loading the model
	 var fin = fs.openRead('linreg_example.bin');
	 var linreg2 = new analytics.RecLinReg(fin);
	
});
});
describe("Logistic regression model. Uses Newtons method to compute the weights, number 49", function () {
it("should make test number 49", function () {

  // import analytics module
  var analytics = require('qminer').analytics;
  // create the Logistic Regression model
  var logreg = new analytics.LogReg({ lambda: 2 });
 
});
});
describe("Gets the parameters, number 50", function () {
it("should make test number 50", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Logistic Regression model
	 var logreg = new analytics.LogReg({ lambda: 10 });
	 // get the parameters of the model
	 var param = logreg.getParams(); // returns { lambda: 10, intercept: false }
	
});
});
describe("Set the parameters, number 51", function () {
it("should make test number 51", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a logistic regression model
	 var logreg = new analytics.LogReg({ lambda: 10 });
	 // set the parameters of the model
	 logreg.setParams({ lambda: 1 });
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y, number 52", function () {
it("should make test number 52", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the logistic regression model
	  var logreg = new analytics.LogReg();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	  var vec = new la.Vector([1, 0, -1, -2]);
	  // if openblas is used, fit the model
	  if (require('qminer').flags.blas) {
	      logreg.fit(mat, vec);
	  }
	 
});
});
describe("Returns the expected response for the provided feature vector, number 53", function () {
it("should make test number 53", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the logistic regression model
	  var logreg = new analytics.LogReg();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	  var vec = new la.Vector([1, 0, -1, -2]);
	  // if openblas is used
	  if (require('qminer').flags.blas) {
	      // fit the model
	      logreg.fit(mat, vec);
	      // create the vector for the prediction
	      var test = new la.Vector([1, 1]);
	      // get the prediction
	      var prediction = logreg.predict(test);
	  };
	 
});
});
describe("Saves the model into the output stream, number 54", function () {
it("should make test number 54", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  var fs = require('qminer').fs;
	  // create the logistic regression model
	  var logreg = new analytics.LogReg();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	  var vec = new la.Vector([1, 0, -1, -2]);
	  // if openblas is used, fit the model
	  if (require('qminer').flags.blas) {
	      logreg.fit(mat, vec);
	  };
	  // create an output stream object and save the model
	  var fout = fs.openWrite('logreg_example.bin');
	  logreg.save(fout);
	  fout.close();
	  // create input stream
	  var fin = fs.openRead('logreg_example.bin');
	  // create a Logistic Regression object that loads the model and parameters from input stream
	  var logreg2 = new analytics.LogReg(fin);
	 
});
});
describe("Proportional Hazards Model with a constant hazard function, number 55", function () {
it("should make test number 55", function () {

  // import analytics module
  var analytics = require('qminer').analytics;
  // create a Proportional Hazard model
  var hazard = new analytics.PropHazards();
 
});
});
describe("Gets the parameters of the model, number 56", function () {
it("should make test number 56", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Proportional Hazard model
	 var hazard = new analytics.PropHazards({ lambda: 5 });
	 // get the parameters of the model
	 var param = hazard.getParams();
	
});
});
describe("Sets the parameters of the model, number 57", function () {
it("should make test number 57", function () {
 
	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Proportional Hazard model
	 var hazard = new analytics.PropHazards({ lambda: 5 });
	 // set the parameters of the model
	 hazard.setParams({ lambda: 10 });
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y, number 58", function () {
it("should make test number 58", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the Proportional Hazards model
	  var hazards = new analytics.PropHazards();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	  var vec = new la.Vector([1, 0, -1, -2]);
	  // if openblas used, fit the model
	  if (require('qminer').flags.blas) {
	      hazards.fit(mat, vec);
	  };
	 
});
});
describe("Returns the expected response for the provided feature vector, number 59", function () {
it("should make test number 59", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  // create the Proportional Hazards model
	  var hazards = new analytics.PropHazards();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 1], [1, -1]]);
      var vec = new la.Vector([3, 3]);
	  // if openblas used
	  if (require('qminer').flags.blas) {
	      // fit the model
	      hazards.fit(mat, vec);       
	      // create a vector for the prediction
	       var test = new la.Vector([1, 2]);
	      // predict the value
	      var prediction = hazards.predict(test);
	  };
	 
});
});
describe("Saves the model into the output stream, number 60", function () {
it("should make test number 60", function () {

	  // import modules
	  var analytics = require('qminer').analytics;
	  var la = require('qminer').la;
	  var fs = require('qminer').fs;
	  // create the Proportional Hazards model
	  var hazards = new analytics.PropHazards();
	  // create the input matrix and vector for fitting the model
	  var mat = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);
	  var vec = new la.Vector([1, 0, -1, -2]);
	  // if openblas used, fit the model
	  if (require('qminer').flags.blas) {
	      hazards.fit(mat, vec);
	  };
	  // create an output stream and save the model
	  var fout = fs.openWrite('hazards_example.bin');
	  hazards.save(fout);
	  fout.close();
	  // create input stream
	  var fin = fs.openRead('hazards_example.bin');
	  // create a Proportional Hazards object that loads the model and parameters from input stream
	  var hazards2 = new analytics.PropHazards(fin);	
	 
});
});
describe("Get the parameters of the model, number 61", function () {
it("should make test number 61", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Neural Networks model
	 var nnet = new analytics.NNet();
	 // get the parameters
	 var params = nnet.getParams();
	
});
});
describe("Sets the parameters of the model, number 62", function () {
it("should make test number 62", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Neural Networks model
	 var nnet = new analytics.NNet();
	 // set the parameters
	 nnet.setParams({ learnRate: 1, momentum: 10, layout: [1, 4, 3] });
	
});
});
describe("Fits the model, number 63", function () {
it("should make test number 63", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a Neural Networks model
	 var nnet = new analytics.NNet({ layout: [2, 3, 4] });
	 // create the matrices for the fitting of the model
	 var matIn = new la.Matrix([[1, 0], [0, 1]]);
	 var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
	 // fit the model
	 nnet.fit(matIn, matOut);
	
});
});
describe("Sends the vector through the model and get the prediction, number 64", function () {
it("should make test number 64", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a Neural Networks model
	 var nnet = new analytics.NNet({ layout: [2, 3, 4] });
	 // create the matrices for the fitting of the model
	 var matIn = new la.Matrix([[1, 0], [0, 1]]);
	 var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
	 // fit the model
	 nnet.fit(matIn, matOut);
	 // create the vector for the prediction
	 var test = new la.Vector([1, 1]);
	 // predict the value
	 var prediction = nnet.predict(test);
	
});
});
describe("Saves the model, number 65", function () {
it("should make test number 65", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 var fs = require('qminer').fs;
	 // create a Neural Networks model
	 var nnet = new analytics.NNet({ layout: [2, 3, 4] });
	 // create the matrices for the fitting of the model
	 var matIn = new la.Matrix([[1, 0], [0, 1]]);
	 var matOut = new la.Matrix([[1, 1], [1, 2], [-1, 8], [-3, -3]]);
	 // fit the model
	 nnet.fit(matIn, matOut);
	 // create an output stream object and save the model
	 var fout = fs.openWrite('nnet_example.bin');
	 nnet.save(fout);
	 fout.close();
	 // load the Neural Network model from the binary
	 var fin = fs.openRead('nnet_example.bin');
	 var nnet2 = new analytics.NNet(fin);
	
});
});
describe("Tokenize, number 66", function () {
it("should make test number 66", function () {

  // import analytics module
  var analytics = require('qminer').analytics;
  // construct model
  var tokenizer = new analytics.Tokenizer({ type: "simple" })
 
});
});
describe("This function tokenizes given strings and returns it as an array of strings, number 67", function () {
it("should make test number 67", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // construct model
	 var tokenizer = new analytics.Tokenizer({ type: "simple" });
	 // string you wish to tokenize
	 var string = "What a beautiful day!";
	 // tokenize string using getTokens
	 var tokens = tokenizer.getTokens(string);
	 // output:
	 tokens = ["What", "a", "beautiful", "day"];
	
});
});
describe("This function breaks text into sentences and returns them as an array of strings, number 68", function () {
it("should make test number 68", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // construct model
	 var tokenizer = new analytics.Tokenizer({ type: "simple" });
	 // string you wish to tokenize
	 var string = "C++? Alright. Let's do this!";
	 // tokenize text using getSentences
	 var tokens = tokenizer.getSentences(string);
	 // output:
	 tokens = ["C++", " Alright", " Let's do this"];
	
});
});
describe("This function breaks text into paragraphs and returns them as an array of strings, number 69", function () {
it("should make test number 69", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // construct model
	 var tokenizer = new analytics.Tokenizer({ type: "simple" });
	 // string you wish to tokenize
	 var string = "Yes!\t No?\n Maybe...";
	 // tokenize text using getParagraphs
	 var tokens = tokenizer.getParagraphs(string);
	 // output:
	 tokens = ["Yes", " No", " Maybe"];
	
});
});
describe("@clas, number 70", function () {
it("should make test number 70", function () {

 // import analytics module
 var analytics = require('qminer').analytics;
 // construct a MDS instance
 var mds = new analytics.MDS({ maxStep: 300, distType: 'Cos' });

});
});
describe("Get the parameters, number 71", function () {
it("should make test number 71", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a MDS instance
	 var mds = new analytics.MDS();
	 // get the (default) parameters of the instance
	 // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
	 var params = mds.getParams();
	
});
});
describe("Set the parameters, number 72", function () {
it("should make test number 72", function () {

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a MDS instance
	 var mds = new analytics.MDS();
	 // get the (default) parameters of the instance
	 // returns { maxStep: 5000, maxSecs: 300, minDiff: 1e-4, distType: "Euclid" }
	 var params = mds.getParams();
	
});
});
describe("Get the MDS of the given matrix, number 73", function () {
it("should make test number 73", function () {

	 // import the modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a MDS instance
	 var mds = new analytics.MDS();
	 // create the multidimensional matrix
	 var mat = new la.Matrix({ rows: 50, cols: 10, random: true });
	 // get the 2d representation of mat 
	 mds.fitTransformAsync(mat, function (err, res) {
	    if (err) { console.log(err); return }
	    // successful calculation
	    var mat2d = res;
	 }); 
	
});
});
describe("Get the MDS of the given matrix, number 74", function () {
it("should make test number 74", function () {

	 // import the modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 // create a MDS instance
	 var mds = new analytics.MDS();
	 // create the multidimensional matrix
	 var mat = new la.Matrix({ rows: 50, cols: 10, random: true });
	 // get the 2d representation of mat 
	 var mat2d = mds.fitTransform(mat); 
	
});
});
describe("Save the MDS, number 75", function () {
it("should make test number 75", function () {

	 // import modules
	 var analytics = require('qminer').analytics;
	 var fs = require('qminer').fs;
	 // create a MDS instance
	 var mds = new analytics.MDS({ iter: 200, MaxStep: 10 });
	 // create the file output stream
	 var fout = new fs.openWrite('MDS.bin');
	 // save the MDS instance
	 mds.save(fout);
	 fout.close();
	 // load the MDS instance
	 var fin = fs.openRead('MDS.bin');
	 var mds2 = new analytics.MDS(fin);
	
});
});
describe("Transforming arrays with labels to vector appropriate for binary classifiers, number 76", function () {
it("should make test number 76", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create binarizer with 'b' as positive label
     var binarizer = new analytics.preprocessing.Binarizer('b');
     // get vector with binarized labels
     var bins = binarizer.transform(['a','b','a','c']);
    
});
});
describe("Get the model, number 77", function () {
it("should make test number 77", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a SVC model
     var SVC = new analytics.SVC();
     // get the properties of the model
     var model = SVC.getModel(); // returns { weight: new require('qminer').la.Vector(); }
	
});
});
describe("Get the model, number 78", function () {
it("should make test number 78", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a SVR model
     var SVR = new analytics.SVR();
     // get the properties of the model
     var model = SVR.getModel(); // returns { weights: new require('qminer').la.Vector(); }
	
});
});
describe("Gets the model, number 79", function () {
it("should make test number 79", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create the Ridge Regression model
     var regmod = new analytics.RidgeReg();
     // get the model
     var model = regmod.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    
});
});
describe("Gets Recursive Linear Regression mode, number 80", function () {
it("should make test number 80", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create the Recursive Linear Regression model
     var linreg = new analytics.RecLinReg({ dim: 10 });
     // get the model
     var model = linreg.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    
});
});
describe("@classdesc One vs. all model for multiclass prediction. Builds binary mode, number 81", function () {
it("should make test number 81", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a new OneVsAll object with the model analytics.SVC
     var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
    
});
});
describe("Gets the parameters, number 82", function () {
it("should make test number 82", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new OneVsAll object with the model analytics.SVC
         var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         // get the parameters
         // returns the JSon object
         // { model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2, models: [] }
         var params = onevsall.getParams();
        
});
});
describe("Sets the parameters, number 83", function () {
it("should make test number 83", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new OneVsAll object with the model analytics.SVC
         var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         // set the parameters
         var params = onevsall.setParams({ model: analytics.SVR, modelParam: { c: 12, maxTime: 10000}, cats: 3, verbose: true });
        
});
});
describe("Apply all models to the given vector and returns a vector of scores, one for each category, number 84", function () {
it("should make test number 84", function () {

          // import modules
          var analytics = require('qminer').analytics;
          var la = require('qminer').la;
          // create a new OneVsAll object with the model analytics.SVC
          var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
          // create the data (matrix and vector) used to fit the model
          var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
          var vector = new la.Vector([0, 0, 1, 1]);
          // fit the model
          onevsall.fit(matrix, vector);
          // create the vector for the decisionFunction
          var test = new la.Vector([1, 2]);
          // give the vector to the decision function
          var prediction = onevsall.predict(test); // returns the vector of scores
         
});
});
describe("Apply all models to the given vector and returns category with the highest score, number 85", function () {
it("should make test number 85", function () {

          // import modules
          var analytics = require('qminer').analytics;
          var la = require('qminer').la;
          // create a new OneVsAll object with the model analytics.SVC
          var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
          // create the data (matrix and vector) used to fit the model
          var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
          var vector = new la.Vector([0, 0, 1, 1]);
          // fit the model
          onevsall.fit(matrix, vector);
          // create the vector for the prediction
          var test = new la.Vector([1, 2]);
          // get the prediction of the vector
          var prediction = onevsall.predict(test); // returns 0
         
});
});
describe("Apply all models to the given vector and returns category with the highest score, number 86", function () {
it("should make test number 86", function () {

          // import modules
          var analytics = require('qminer').analytics;
          var la = require('qminer').la;
          // create a new OneVsAll object with the model analytics.SVC
          var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
          // create the data (matrix and vector) used to fit the model
          var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
          var vector = new la.Vector([0, 0, 1, 1]);
          // fit the model
          onevsall.fit(matrix, vector);
         
});
});
describe("Metric, number 87", function () {
it("should make test number 87", function () {

     // import metrics module
     var analytics = require('qminer').analytics;
    
     // true and predicted lables
     var true_lables = [0, 1, 0, 0, 1];
     var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    
     // compute ROC curve
     var roc = analytics.metrics.rocCurve(true_lables, pred_prob);
    
});
});
describe("Metric, number 88", function () {
it("should make test number 88", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // true and predicted lables
     var true_lables = [0, 1, 0, 0, 1];
     var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    
     // create predictionCurve instance
     var predictionCurve = new analytics.metrics.PredictionCurve();
    
     // simulate data flow
     for (var i in true_lables) {
        // push new value
        predictionCurve.push(true_lables[i], pred_prob[i]);
    }
    
     var roc = predictionCurve.roc(); // get ROC
    
});
});
describe("Metric, number 89", function () {
it("should make test number 89", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // true and predicted data
     var true_vals = [1, 2, 3, 4, 5];
     var pred_vals = [3, 4, 5, 6, 7];
    
     // use batch MAE method
     analytics.metrics.meanAbsoluteError(true_vals, pred_vals);
    
});
});
describe("Metric, number 90", function () {
it("should make test number 90", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // true and predicted data
     var true_vals = [1, 2, 3, 4, 5];
     var pred_vals = [3, 4, 5, 6, 7];
    
     // create online MAE metric instance
     var mae = new analytics.metrics.MeanAbsoluteError();
    
     // simulate data flow
     for (var i in true_vals) {
       // push new value
       mae.push(true_vals[i], pred_vals[i]);
     }
     // get updated error
     mae.getError();
    
});
});
describe("Class implements several prediction curve measures (ROC, AOC, Precision-Recall, ..., number 91", function () {
it("should make test number 91", function () {

     // import metrics module
     var metrics = require('qminer').analytics.metrics;
    
     // true and predicted lables
     var true_lables = [0, 1, 0, 0, 1];
     var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    
     // create predictionCurve instance
     var predictionCurve = new metrics.PredictionCurve();
    
     // simulate data flow
     for (var i in true_lables) {
        // push new value
        predictionCurve.push(true_lables[i], pred_prob[i]);
    }
    
     var roc = predictionCurve.roc(); // get ROC
     var auc = predictionCurve.auc(); // get AUC
     var pr = predictionCurve.precisionRecallCurve() // get precision-recall curve
    
});
});
describe("Get ROC parametrization sampled on `sample` point, number 92", function () {
it("should make test number 92", function () {

     // import metrics module
     var metrics = require('qminer').analytics.metrics;
    
     // true and predicted lables
     var true_lables = [0, 1, 0, 0, 1];
     var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    
     // compute ROC curve
     var roc = metrics.rocCurve(true_lables, pred_prob); // output: [ [ 0, 0 ], [0, 0.5], [[ 0.34, 1 ],], [ 0.67, 0 ], [ 1, 1 ] ]
    
});
});
describe("Get AUC of the current curv, number 93", function () {
it("should make test number 93", function () {

     // import metrics module
     var metrics = require('qminer').analytics.metrics;
    
     // true and predicted lables
     var true_lables = [0, 1, 0, 0, 1];
     var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    
     // compute ROC curve
     var auc = metrics.rocAucScore(true_lables, pred_prob); // output: 0.92
    
});
});
describe("@classdesc Principal components analysi, number 94", function () {
it("should make test number 94", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // construct model
     var pca = new analytics.PCA();
    
});
});
describe("@classdesc Principal components analysi, number 95", function () {
it("should make test number 95", function () {

     // import analytics module
     var analytics = require('qminer').analytics;
     // construct model
     var pca = new analytics.PCA({ k: 5, iter: 50 });
    
});
});
describe("Returns the mode, number 96", function () {
it("should make test number 96", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit matrix before getting the model
         pca.fit(matrix)
         // get your model using function getModel
         var model = pca.getModel();
        
});
});
describe("Saves the model, number 97", function () {
it("should make test number 97", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit matrix
         pca.fit(matrix);
         var model = pca.getModel();
         // save model
         pca.save(require('qminer').fs.openWrite('pca_test.bin')).close();
        
});
});
describe("Sets parameter, number 98", function () {
it("should make test number 98", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // set 5 eigenvectors and 10 iterations using setParams
         pca.setParams({iter: 10, k: 5});
        
});
});
describe("Gets parameter, number 99", function () {
it("should make test number 99", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // check the constructor parameters
         var paramvalue = pca.getParams();
        
});
});
describe("Gets parameter, number 100", function () {
it("should make test number 100", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // set parameters
         pca.setParams({iter: 10, k: 5});
         // check the changed parameters
         var paramvalue = pca.getParams();
        
});
});
describe("Finds the eigenvectors of the variance matrix, number 101", function () {
it("should make test number 101", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit the matrix
         pca.fit(matrix);
        
});
});
describe("Projects the example(s) and expresses them as coefficients in the eigenvector basis this.P, number 102", function () {
it("should make test number 102", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit the matrix
         pca.fit(matrix);
         var model = pca.getModel();
         // transform matrix
         var transform = pca.transform(matrix);
        
});
});
describe("Projects the example(s) and expresses them as coefficients in the eigenvector basis this.P, number 103", function () {
it("should make test number 103", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create vector you wish to transform
         var vector = new la.Vector([0, -1]);
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit the matrix
         pca.fit(matrix);
         var model = pca.getModel();
         // transform vector
         var transform = pca.transform(vector);
        
});
});
describe("Reconstructs the vector in the original space, reverses centerin, number 104", function () {
it("should make test number 104", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit the matrix
         pca.fit(matrix);
         var model = pca.getModel();
         // use inverseTransform on matrix
         var invTransform = pca.inverseTransform(matrix);
        
});
});
describe("Reconstructs the vector in the original space, reverses centerin, number 105", function () {
it("should make test number 105", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // construct model
         var pca = new analytics.PCA();
         // create vector
         var vector = new la.Vector([0, -1]);
         // create matrix
         var matrix = new la.Matrix([[0, 1], [-1, 0]]);
         // fit the matrix
         pca.fit(matrix);
         var model = pca.getModel();
         // use inverseTransform on vector
         var invTransform = pca.inverseTransform(vector);
        
});
});
describe("@classdesc KMeans clusterin, number 106", function () {
it("should make test number 106", function () {

     // import analytics and la modules
     var analytics = require('qminer').analytics;
     var la = require('qminer').la;
     // create a KMeans object
     var KMeans = new analytics.KMeans();
     // create the matrix to be fitted
     var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     // create the model
     KMeans.fit(X);
    
});
});
describe("Returns the mode, number 107", function () {
it("should make test number 107", function () {

         // import modules
         var analytics = require('qminer').analytics;
         var la = require('qminer').la;
         // create the KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000 });
         // create a matrix to be fitted
         var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
         // create the model
         KMeans.fit(X);
         // get the model
         var model = KMeans.getModel();
        
});
});
describe("Sets the parameters, number 108", function () {
it("should make test number 108", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans();
         // change the parameters of the KMeans object
         KMeans.setParams({ iter: 1000, k: 5 });
        
});
});
describe("Returns the parameters, number 109", function () {
it("should make test number 109", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 5 });
         // get the parameters
         var json = KMeans.getParams();
        
});
});
describe("Computes the centroids, number 110", function () {
it("should make test number 110", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
         // create a matrix to be fitted
         var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
         // create the model with the matrix X
         KMeans.fit(X);
        
});
});
describe("Returns an vector of cluster id assignments, number 111", function () {
it("should make test number 111", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
         // create a matrix to be fitted
         var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
         // create the model with the matrix X
         KMeans.fit(X);
         // create the matrix of the prediction vectors
         var pred = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
         // predict the values
         var prediction = KMeans.predict(pred);
        
});
});
describe("Returns the IDs of the nearest medoid for each example, number 112", function () {
it("should make test number 112", function () {

         // import analytics module
         var analytics = require('qminer').analytics;
         // import linear algebra module
         var la = require('qminer').la;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
         // create a matrix to be fitted
         var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
         // create the model with the matrix X using the column IDs [0,1,2]
         KMeans.fit(X, [1234,1142,2355]);
         // create the matrix of the prediction vectors
         var test = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
         // predict/explain - return the closest medoids
         var explanation = KMeans.explain(test);
        
});
});
describe("Transforms the points to vectors of squared distances to centroids, number 113", function () {
it("should make test number 113", function () {

         // import modules
         var analytics = require('qminer').analytics;
         var la = require('qminer').la;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
         // create a matrix to be fitted
         var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
         // create the model with the matrix X
         KMeans.fit(X);
         // create the matrix of the transform vectors
         var matrix = new la.Matrix([[-2, 0], [0, -3]]);
         // get the transform values of matrix
         // returns the matrix
         //  10    17
         //   1    20
         //  10     1
         KMeans.transform(matrix);
        
});
});

});
