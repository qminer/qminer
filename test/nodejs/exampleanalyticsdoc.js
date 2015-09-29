describe('example tests for the analyticsdoc.js file', function () {
describe("Analytics module.", function () {
it('should make test number 1', function () {
 this.timeout(10000); 

  // import module, load dataset, create model, evaluate model
 
});
});
describe("SVC", function () {
it('should make test number 2', function () {
 this.timeout(10000); 

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
 var SVC = new analytics.SVC({ verbose: true });
 // train classifier
 SVC.fit(featureMatrix, targets);
 // set up a fake test vector
 var test = new la.Vector([1.1, -0.5]);
 // predict the target value
 var prediction = SVC.predict(test);

});
});
describe("Returns the SVC parameters.", function () {
it('should make test number 3', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVC model with json
	 var SVC = new analytics.SVC({ c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true });
	 // get the parameters of the SVC model
	 // returns { algorithm: 'SGD' c: 5, j: 10, batchSize: 2000, maxIterations: 12000, maxTime: 2, minDiff: 1e-10, verbose: true }
	 var json = SVC.getParams(); 
	
});
});
describe("Sets the SVC parameters.", function () {
it('should make test number 4', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a default SVC model
	 var SVC = new analytics.SVC();
	 // change the parameters of the SVC with the json { j: 5, maxIterations: 12000, minDIff: 1e-10 }
	 SVC.setParams({ j: 5, maxIterations: 12000, minDiff: 1e-10 }); // returns self
	
});
});
describe("Gets the vector of coefficients of the linear model.", function () {
it('should make test number 5', function () {
 this.timeout(10000); 
 
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
describe("Saves model to output file stream.", function () {
it('should make test number 6', function () {
 this.timeout(10000); 

	 // import the analytics and la modules
	 var analytics = require('qminer').analytics;
	 var la = require('qminer').la;
	 var qmfs = require('qminer').fs;
	 // create a new SVC object
	 var SVC = new analytics.SVC();
	 // create the matrix containing the input features and the input vector for each matrix.
	 var matrix = new la.Matrix([[1, 0, -1, 0], [0, 1, 0, -1]]);	
	 // fit the model
	 SVC.fit(matrix, vec);
	 var fs = require('qminer').fs;
	 // create output stream
	 var fout = fs.openWrite('model.bin');
	 // save SVC object (model and parameters) to output stream and close it
	 SVC.save(fout);
	 fout.close();
	 // create input stream
	 var fin = fs.openRead('tesi.bin');
	 // create a SVC object that loads the model and parameters from input stream
	 var SVC2 = new analytics.SVC(fin);	
	
});
});
describe("Sends vector through the model and returns the distance to the decision boundery.", function () {
it('should make test number 7', function () {
 this.timeout(10000); 

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
describe("Sends vector through the model and returns the prediction as a real number.", function () {
it('should make test number 8', function () {
 this.timeout(10000); 

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
describe("Fits a SVM classification model, given column examples in a matrix and vector of targets.", function () {
it('should make test number 9', function () {
 this.timeout(10000); 

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
describe("SVR", function () {
it('should make test number 10', function () {
 this.timeout(10000); 

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
 var SVR = new analytics.SVR({ verbose: true });
 // Train regression
 SVR.fit(featureMatrix, targets);
 // Set up a fake test vector
 var test = new la.Vector([1.1, -0.8]);
 // Predict the target value
 var prediction = SVR.predict(test);

});
});
describe("Returns the SVR parameters.", function () {
it('should make test number 11', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVR object
	 var SVR = new analytics.SVR({ c: 10, eps: 1e-10, maxTime: 12000, verbose: true });
	 // get the parameters of SVR
	 var params = SVR.getParams();
	
});
});
describe("Sets the SVR parameters.", function () {
it('should make test number 12', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new SVR object
	 var SVR = new analytics.SVR();
	 // set the parameters of the SVR object
	 SVR.setParams({ c: 10, maxTime: 12000 });
	
});
});
describe("Sends vector through the model and returns the scalar product as a real number.", function () {
it('should make test number 13', function () {
 this.timeout(10000); 

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
describe("Sends vector through the model and returns the prediction as a real number.", function () {
it('should make test number 14', function () {
 this.timeout(10000); 

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
describe("fits an SVM regression model, given column examples in a matrix and vector of targets", function () {
it('should make test number 15', function () {
 this.timeout(10000); 

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
describe("Ridge regression. Minimizes: ||A' x - b||^2 + ||gamma x||^2", function () {
it('should make test number 16', function () {
 this.timeout(10000); 

  // import modules
  la = require('qminer').la;
  analytics = require('qminer').analytics;
  // create a new model with gamma = 1.0
  var regmod = new analytics.RidgeReg(1.0);
  // generate a random feature matrix
  var A = la.randn(10,100);
  // generate a random model
  var w = la.randn(10);
  // generate noise
  var n = la.randn(100).multiply(0.01);
  // generate responses (model'data + noise)
  var b = A.transpose().multiply(w).plus(n);
  // fit model
  regmod.fit(A, b);
  // compare
  console.log('true model:');
  w.print();
  console.log('trained model:');
  regmod.weights.print();
  // cosine between the true and the estimated model should be close to 1 if the fit succeeded
  console.log('cosine(w, regmod.weights): ' + regmod.weights.cosine(w));
 
});
});
describe("Gets the parameters.", function () {
it('should make test number 17', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Ridge Regression object
	 var regmod = new analytics.RidgeReg({ gamma: 5 });
	 // get the parameters
	 // returns a json object { gamma: 5 }
	 var param = regmod.getParams();
	
});
});
describe("Set the parameters.", function () {
it('should make test number 18', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Ridge Regression object
	 var regmod = new analytics.RidgeReg({ gamma: 5 });
	 // set the parameters of the object
	 var param = regmod.setParams({ gamma: 10 });
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y.", function () {
it('should make test number 19', function () {
 this.timeout(10000); 

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
describe("Returns the expected response for the provided feature vector.", function () {
it('should make test number 20', function () {
 this.timeout(10000); 

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
describe("Returns the expected response for the provided feature vector.", function () {
it('should make test number 21', function () {
 this.timeout(10000); 

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
describe("Sigmoid function (y = 1/[1 + exp[", function () {
it('should make test number 22', function () {
 this.timeout(10000); 

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
describe("Get the parameters. It doesn't do anything, it's only for consistency for constructing pipeline.", function () {
it('should make test number 23', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // get the parameters
	 // returns an empty Json object
	 var param = s.getParams();
	
});
});
describe("Sets the parameters. It doesn't do anything, it's only for consistency for constructing pipeline.", function () {
it('should make test number 24', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // set the parameters 
	 // doesn't change the model
	 s.setParams({});
	
});
});
describe("Gets the model.", function () {
it('should make test number 25', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Sigmoid model
	 var s = new analytics.Sigmoid();
	 // get the model parameters
	 // returns a Json object { A: 0, B: 0 }
	 var model = s.getModel();
	
});
});
describe("Fits a column matrix of feature vectors X onto the response variable y.", function () {
it('should make test number 26', function () {
 this.timeout(10000); 

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
describe("Returns the expected response for the provided feature vector.", function () {
it('should make test number 27', function () {
 this.timeout(10000); 

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
describe("Returns the expected response for the provided feature vector.", function () {
it('should make test number 28', function () {
 this.timeout(10000); 

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
describe("Nearest Neighbour Anomaly Detection ", function () {
it('should make test number 29', function () {
 this.timeout(10000); 

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
describe("Sets parameters.", function () {
it('should make test number 30', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // set it's parameters to rate: 0.1
	 neighbor.setParams({ rate: 0.1 });
	
});
});
describe("Returns parameters.", function () {
it('should make test number 31', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD();
	 // get the parameters of the object
	 // returns a json object { rate: 0.05 }
	 var params = neighbor.getParams();
	
});
});
describe("Returns the model.", function () {
it('should make test number 32', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new NearestNeighborAD object
	 var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
	 // get the model of the object
	 // returns a json object { rate: 0.1, window: 0 }
	 var model = neighbor.getModel();
	
});
});
describe("Adds a new point (or points) to the known points and recomputes the threshold.", function () {
it('should make test number 33', function () {
 this.timeout(10000); 

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
describe("Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.", function () {
it('should make test number 34', function () {
 this.timeout(10000); 

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
describe("Compares the point to the known points and returns distance to the nearest one.", function () {
it('should make test number 35', function () {
 this.timeout(10000); 

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
describe("Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold).", function () {
it('should make test number 36', function () {
 this.timeout(10000); 

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
describe("Recursive Linear Regression", function () {
it('should make test number 37', function () {
 this.timeout(10000); 

 // import analytics module
 var analytics = require('qminer').analytics;
 // create the recursive linear regression model holder
 var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });

});
});
describe("Creates a partial fit of the input.", function () {
it('should make test number 38', function () {
 this.timeout(10000); 

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
describe("Creates a fit of the input.", function () {
it('should make test number 39', function () {
 this.timeout(10000); 

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
describe("Puts the vector through the model and returns the prediction as a real number.", function () {
it('should make test number 40', function () {
 this.timeout(10000); 

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
describe("Sets the parameters of the model.", function () {
it('should make test number 41', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 10 });
	 // set the parameters of the model
	 linreg.setParams({ dim: 3, recFact: 1e2, forgetFact: 0.5 });
	
});
});
describe("Returns the parameters.", function () {
it('should make test number 42', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a new Recursive Linear Regression model
	 var linreg = new analytics.RecLinReg({ dim: 10 });
	 // get the parameters of the model
	 var params = linreg.getParams(); // returns { dim: 10, recFact: 1.0, forgetFact: 1.0 }
	
});
});
describe("Logistic regression model. Uses Newtons method to compute the weights.", function () {
it('should make test number 43', function () {
 this.timeout(10000); 

  // import analytics module
  var analytics = require('qminer').analytics;
  // create the Logistic Regression model
  var logreg = new analytics.LogReg({ lambda: 2 });
 
});
});
describe("Gets the parameters.", function () {
it('should make test number 44', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create the Logistic Regression model
	 var logreg = new analytics.LogReg({ lambda: 10 });
	 // get the parameters of the model
	 var param = logreg.getParams(); // returns { lambda: 10, intercept: false }
	
});
});
describe("Set the parameters.", function () {
it('should make test number 45', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a logistic regression model
	 var logreg = new analytics.LogReg({ lambda: 10 });
	 // set the parameters of the model
	 logreg.setParams({ lambda: 1 });
	
});
});
describe("Proportional Hazards Model with a constant hazard function.", function () {
it('should make test number 46', function () {
 this.timeout(10000); 

  // import analytics module
  var analytics = require('qminer').analytics;
  // create a Proportional Hazard model
  var hazard = new analytics.PropHazards();
 
});
});
describe("Gets the parameters of the model.", function () {
it('should make test number 47', function () {
 this.timeout(10000); 

	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Proportional Hazard model
	 var hazard = new analytics.PropHazards({ lambda: 5 });
	 // get the parameters of the model
	 var param = hazard.getParams();
	
});
});
describe("Sets the parameters of the model.", function () {
it('should make test number 48', function () {
 this.timeout(10000); 
 
	 // import analytics module
	 var analytics = require('qminer').analytics;
	 // create a Proportional Hazard model
	 var hazard = new analytics.PropHazards({ lambda: 5 });
	 // set the parameters of the model
	 hazard.setParams({ lambda: 10 });
	
});
});
describe("Get the model.", function () {
it('should make test number 49', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a SVC model
     var SVC = new analytics.SVC();
     // get the properties of the model
     var model = SVC.getModel(); // returns { weight: new require('qminer').la.Vector(); }
	
});
});
describe("Get the model.", function () {
it('should make test number 50', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a SVR model
     var SVR = new analytics.SVR();
     // get the properties of the model
     var model = SVR.getModel(); // returns { weights: new require('qminer').la.Vector(); }
	
});
});
describe("Gets the model.", function () {
it('should make test number 51', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // create the Ridge Regression model
     var regmod = new analytics.RidgeReg();
     // get the model
     var model = regmod.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    
});
});
describe("Gets Recursive Linear Regression model", function () {
it('should make test number 52', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // create the Recursive Linear Regression model
     var linreg = new analytics.RecLinReg({ dim: 10 });
     // get the model
     var model = linreg.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    
});
});
describe("@classdesc One vs. all model for multiclass prediction. Builds binary model", function () {
it('should make test number 53', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // create a new OneVsAll object with the model analytics.SVC
     var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
    
});
});
describe("Gets the parameters.", function () {
it('should make test number 54', function () {
 this.timeout(10000); 

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
describe("Sets the parameters.", function () {
it('should make test number 55', function () {
 this.timeout(10000); 

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new OneVsAll object with the model analytics.SVC
         var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         // set the parameters
         var params = onevsall.setParams({ model: analytics.SVR, modelParam: { c: 12, maxTime: 10000}, cats: 3, verbose: true });
        
});
});
describe("Apply all models to the given vector and returns a vector of scores, one for each category.", function () {
it('should make test number 56', function () {
 this.timeout(10000); 

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
describe("Apply all models to the given vector and returns category with the highest score.", function () {
it('should make test number 57', function () {
 this.timeout(10000); 

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
describe("Apply all models to the given vector and returns category with the highest score.", function () {
it('should make test number 58', function () {
 this.timeout(10000); 

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
describe("Metrics", function () {
it('should make test number 59', function () {
 this.timeout(10000); 

     // import analytics module
     var analytics = require('qminer').analytics;
     // true and predicted data
     var true_vals = [1, 2, 3, 4, 5];
     var pred_vals = [3, 4, 5, 6, 7];
    
     // use batch MAE method
     analytics.metrics.meanAbsoluteError(true_vals, pred_vals);
    
});
it('should make test number 60', function () {
 this.timeout(10000); 

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
describe("@classdesc KMeans clustering", function () {
it('should make test number 61', function () {
 this.timeout(10000); 

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
describe("Returns the model", function () {
it('should make test number 62', function () {
 this.timeout(10000); 

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
describe("Sets the parameters.", function () {
it('should make test number 63', function () {
 this.timeout(10000); 

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans();
         // change the parameters of the KMeans object
         KMeans.setParams({ iter: 1000, k: 5 });
        
});
});
describe("Returns the parameters.", function () {
it('should make test number 64', function () {
 this.timeout(10000); 

         // import analytics module
         var analytics = require('qminer').analytics;
         // create a new KMeans object
         var KMeans = new analytics.KMeans({ iter: 1000, k: 5 });
         // get the parameters
         var json = KMeans.getParams();
        
});
});
describe("Computes the centroids", function () {
it('should make test number 65', function () {
 this.timeout(10000); 

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
describe("Returns an vector of cluster id assignments", function () {
it('should make test number 66', function () {
 this.timeout(10000); 

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
describe("Transforms the points to vectors of squared distances to centroids.", function () {
it('should make test number 67', function () {
 this.timeout(10000); 

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
