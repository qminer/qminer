// import analytics module
var analytics = require('analytics.js');
var assert = require('assert.js');

// This function converts .names string to JSON
// ASSUMES: The first line denotes the name of the target variable
// ASSUMES: The target variable is described last
// ASSUMES: The attribute order in the config is the same as the attribute
// order in the data stream 
// ASSUMES: Each lines "finishes" with a comma (".") 
function names2json(namesFilePath) {
   var fin = fs.openRead(namesFilePath);
   
   var targetNm = fin.getNextLn().trim();
   targetNm = targetNm.substr(0, targetNm.length-1); // Remove the dot 
   
   var jsonCfg = { "dataFormat": [] };
   
   while (!fin.eof) {
      var line = fin.getNextLn().trim().split(":");
      var attributeName = line[0].trim();
      
      jsonCfg.dataFormat.push(attributeName);
      if (line[1].trim() == "continuous.") {
         jsonCfg[attributeName] = { "type": "numeric" };
      } else {
         var valueArr = line[1].trim().split(",");
         var lastVal = valueArr[valueArr.length-1];
         valueArr[valueArr.length-1] = lastVal.substring(0, lastVal.length-1);
         jsonCfg[attributeName] = {
            "type": "discrete",
            "values": valueArr
         };
      }
   }
   
   return jsonCfg;
}

// This functions computes discrete and numeric arrays of attribute values 
// from line given data stream specification in JSON 
// ASSUMES: Line is comma-separated 
function line2array(line, jsonCfg) {
   line = line.trim().split(",");
   var example_discrete = [];
   var example_numeric = [];
   var i = 0;
   for (i = 0; i < jsonCfg["dataFormat"].length-1; ++i) {
      var attributeName = jsonCfg.dataFormat[i];
      if (jsonCfg[attributeName].type == "discrete") {
         example_discrete.push(line[i]);
      } else {
         example_numeric.push(parseFloat(line[i]));
      }
   }
   
   var target = line[i];
   if (jsonCfg[jsonCfg.dataFormat[i]].type == "numeric") {
      target = parseFloat(target);
   }
   
   return {"discrete": example_discrete,
           "numeric": example_numeric,
           "target": target };
}

// 
// Examples using the algorithm on various datasets 
// 

function testClassificationContAttr() {
   // algorithm parameters 
   var htParams = {
      "gracePeriod": 300,
      "splitConfidence": 1e-5,
      "tieBreaking": 0.01,
      "driftCheck": 1000,
      "windowSize": 100000,
      "conceptDriftP": false,
      "clsLeafModel": "naiveBayes",
      "clsAttrHeuristic": "giniGain",
      "maxNodes": 7,
      "attrDiscretization": "histogram"
   };
   
   // describe the data stream 
   var seaCfg = {
      "dataFormat": ["A1", "A2", "A3", "C"], 
      "A1": {
         "type": "numeric",
      }, 
      "A2": {
         "type": "numeric",
      }, 
      "A3": {
         "type": "numeric"
      }, 
      "C": {
         "type": "discrete",
         "values": ["0", "1"]
      }
   };

   // create a new learner 
   var ht = analytics.newHoeffdingTree(seaCfg, htParams);

   // train the model
   var streamData = fs.openRead("./sandbox/ht/sea.dat");
   var examplesN = 0;
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(",");
      // get discrete attributes
      var example_discrete = [];
      // get numeric attributes
      var example_numeric = line.slice(0, 3);
      example_numeric = example_numeric.map(parseFloat);
      // get target
      var target = line[3];
      if (++examplesN % 10000 == 0) {
         console.say("Number of examples processed so far: " + examplesN);
      }
      // if (examplesN >= 200000) { break; } // 200k examples 
      // update the model
      ht.process(example_discrete, example_numeric, target);
   }
   
   // use the model 
   var label = ht.classify([], [6.677259, 5.152133, 2.982455]);
   console.say("f(6.677259, 5.152133, 2.982455) = " + label);
   var label = ht.classify([], [1.848014, 0.041624, 2.913719]);
   console.say("f(1.848014, 0.041624, 2.913719) = " + label);
   
   console.say("Now exporting the model as 'sea.gv'.");
   // export the model 
   ht.exportModel({ "file": "./sandbox/ht/sea.gv", "type": "DOT" });
}

function testClassification() {
   console.say("Testing on a titanic dataset");
   // algorithm parameters 
   var htParams = {
      "gracePeriod": 300,
      "splitConfidence": 1e-5,
      "tieBreaking": 0.005,
      "driftCheck": 1000,
      "windowSize": 100000,
      "conceptDriftP": false,
      "maxNodes": 0,
      "clsLeafModel": "naiveBayes",
      "clsAttrHeuristic": "giniGain",
      "attrDiscretization": "histogram"
   };
   
   // describe the data stream 
   var titanicCfg = {
      "dataFormat": ["status", "age", "sex", "survived"], 
      "sex": {
         "type": "discrete",
         "values": ["male", "female"]
      }, 
      "status": {
         "type": "discrete",
         "values": ["first", "second", "third", "crew"]
      }, 
      "age": {
         "type": "discrete",
         "values": ["child", "adult"]
      }, 
      "survived": {
         "type": "discrete",
         "values": ["yes", "no"]
      }
   };
   
   // create a new learner 
   var ht = analytics.newHoeffdingTree(titanicCfg, htParams);

   // train the model
   var streamData = fs.openRead("./sandbox/ht/titanic-4M.dat");
   var examplesN = 0;
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(","); // male,first,adult,yes
      // get discrete attributes
      var example_discrete = line.slice(0, 3);
      // get numeric attributes
      var example_numeric = [];
      // get target
      var target = line[3];
      if (++examplesN % 10000 == 0) {
         console.say("Number of examples processed so far: " + examplesN);
      }
      if (examplesN > 500000) { break; }
      // update the model
      ht.process(example_discrete, example_numeric, target);
   }
   
   // use the model 
   var label = ht.classify(["first", "adult", "female"], []);
   console.say("Were high society women likely to survive? " + label);
   label = ht.classify(["first", "adult", "male"], []);
   console.say("Were high society men likely to survive? " + label);
   label = ht.classify(["third", "adult", "male"], []);
   console.say("Were 3rd class men likely to survive? " + label);
   label = ht.classify(["second", "adult", "female"], []);
   console.say("Were 3rd class women likely to survive? " + label);
   
   console.say("Now exporting the model as 'titanic.gv'.");
   // export the model 
   ht.exportModel({ "file": "./sandbox/ht/titanic.gv", "type": "DOT" });
}

function testRegressionDisAttr() {
   // algorithm parameters 
   var htParams = {
      "gracePeriod": 300,
      "splitConfidence": 1e-6,
      "tieBreaking": 0.005, 
      "conceptDriftP": false,
      "phAlpha": 0.005,
      "phLambda": 50.0,
      "phInit": 100,
      "maxNodes": 50,
      "regLeafModel": "mean",
      "attrDiscretization": "bst", 
      "sdrThreshold": 0.1,
      "sdThreshold": 0.01
   };
   
   // describe the data stream 
   var regTestCfg = {
      "dataFormat": ["A", "B", "Y"],
      "A": {
         "type": "discrete",
         "values": ["t", "f"]
      },
      "B": {
         "type": "discrete",
         "values": ["t", "f"]
      },
      "Y": {
         "type": "numeric"
      }
   };
   
   // create a new learner 
   var ht = analytics.newHoeffdingTree(regTestCfg, htParams);

   // train the model
   var examplesN = 0;
   var streamData = fs.openRead("./sandbox/ht/regression-test.dat");
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(",");
      // get discrete attributes
      var example_discrete = line.slice(0, 2);
      // get numeric attributes
      var example_numeric = [];
      // get target
      var target = parseFloat(line[2]);
      // entertain the user in case we have too many learning examples 
      if (++examplesN % 10000 == 0) {
         console.say("Number of examples processed so far: "+examplesN);
         if (examplesN > 200000) { break; }
      }
      // update the model
      ht.process(example_discrete, example_numeric, target);
   }
   
   // use the model 
   var val = ht.predict(["t", "t"], []);
   console.say("f(t,t) = " + val);
   var val = ht.predict(["t", "f"], []);
   console.say("f(t,f) = " + val);
   var val = ht.predict(["f", "t"], []);
   console.say("f(f,t) = " + val);
   var val = ht.predict(["f", "f"], []);
   console.say("f(f,f) = " + val);
   
   console.say("Now exporting the model as regression-test.gv");
   // export the model 
   ht.exportModel({ "file": "./sandbox/ht/regression-test.gv", "type": "DOT" });
}

function testRegressionContAttr() {
   var htParams = {
      "gracePeriod": 200,
      "splitConfidence": 1e-6,
      "tieBreaking": 0.1,
      "conceptDriftP": false,
      "phAlpha": 0.005,
      "phLambda": 50.0,
      "phInit": 100,
      "maxNodes": 50,
      "regLeafModel": "mean",
      "attrDiscretization": "histogram",
      "sdrThreshold": 0.1,
      "sdThreshold": 0.001
   };
   var regTestCfg = {
      "dataFormat": ["A", "B", "Y"],
      "A": {
         "type": "discrete",
         "values": ["0", "1"]
      },
      "B": { "type": "numeric" },
      "Y": { "type": "numeric" }
   };
   
   // create a new learner 
   var ht = analytics.newHoeffdingTree(regTestCfg, htParams);

   // train the model
   var examplesN = 0;
   var streamData = fs.openRead("./sandbox/ht/reg-cont.dat");
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(",");
      // get discrete attributes
      var example_discrete = line.slice(0, 1);
      // get numeric attributes
      var example_numeric = line.slice(1, 2);
      example_numeric[0] = parseFloat(example_numeric[0]);
      // get target
      var target = parseFloat(line[2]);
      // entertain the user in case we have too many learning examples 
      if (++examplesN % 10000 == 0) {
         console.say("Number of examples processed so far: "+examplesN);
         if (examplesN > 200000) { break; }
      }
      // update the model
      ht.process(example_discrete, example_numeric, target);
   }
   
   // use the model 
   var val = ht.predict(["0"], [3.3]);
   console.say("f(0, 3.3) = " + val);
   val = ht.predict(["1"], [3.3]);
   console.say("f(1, 3.3) = " + val);
   val = ht.predict(["0"], [2.3]);
   console.say("f(0, 2.3) = " + val);
   val = ht.predict(["1"], [2.3]);
   console.say("f(1, 2.3) = " + val);
   
   console.say("Exporting the model as reg-cont.gv");
   // export the model 
   ht.exportModel({ "file": "./sandbox/ht/reg-cont.gv", "type": "DOT" });
}

function testRegressionDrift() {
   var htParams = {
      "gracePeriod": 200,
      "splitConfidence": 1e-5,
      "tieBreaking": 0.05,
      "conceptDriftP": false,
      "phAlpha": 0.0001,
      "phLambda": 1.0,
      "phInit": 100,
      "maxNodes": 10,
      "regLeafModel": "mean",
      "attrDiscretization": "bst"
   };
   /*var streamCfg = {
      "dataFormat": ["A", "B", "Y"],
      "A": {"type": "numeric"},
      "B": {"type": "numeric"},
      "Y": {"type": "numeric"}
   };*/
   var streamCfg = {
      "dataFormat": ["A", "B", "Y"],
      "A": {
         "type": "discrete",
         "values": ["0", "1"]
      },
      "B": {
         "type": "discrete",
         "values": ["0", "1"]
      },
      "Y": {"type": "numeric"}
   }
   var ht = analytics.newHoeffdingTree(streamCfg, htParams);
   var examplesN = 0;
   // var streamData = fs.openRead("./sandbox/ht/drift.dat");
   var streamData = fs.openRead("./sandbox/ht/dis_drift.dat");
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(",");
      var dis = line.slice(0, 2);
      // var num = line.slice(0, 2);
      // num[0] = parseFloat(num[0]); num[1] = parseFloat(num[1]);
      var y = parseFloat(line[2]);
      // ht.process([], num, y);
      ht.process(dis, [], y);
      if (++examplesN % 10000 == 0) {
         console.say("Processing example "+examplesN);
      }
   }
   console.say("Exporting the model...");
   ht.exportModel({ "file": "./sandbox/ht/reg-dis-drift.gv", "type": "DOT" });
}

function testRegression() {
   // algorithm parameters 
   var htParams = {
      "gracePeriod": 300,
      "splitConfidence": 1e-6,
      "tieBreaking": 0.005,
      "conceptDriftP": false,
      "phAlpha": 0.005,
      "phLambda": 50.0,
      "phInit": 1000,
      "maxNodes": 50,
      "sdrThreshold": 0.1
   };
   
   // TODO: Extract this info from feature space. Potential problem with
   // discrete attributes --- we don't know the range of the attributes 
   // in case the type is not numeric (i.e. text or date) 
   // describe the data stream 
   var regTestCfg = {
      "dataFormat": [
         "year", "month", "day", "RTP", "VAL", "ROS", "KIL",
         "SHA", "BIR", "DUB", "CLA", "MUL", "CLO", "BEL", "MAL"
      ],
      "year": { "type": "numeric" },
      "month": {
         "type": "discrete",
         "values": [
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"
         ]
      },
      "day": {
         "type": "discrete",
         "values": [
            "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12",
            "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23",
            "24", "25", "26", "27", "28", "29", "30", "31"
         ]
      },
      "RTP": { "type": "numeric" },
      "VAL": { "type": "numeric" },
      "ROS": { "type": "numeric" },
      "KIL": { "type": "numeric" },
      "SHA": { "type": "numeric" },
      "BIR": { "type": "numeric" },
      "DUB": { "type": "numeric" },
      "CLA": { "type": "numeric" },
      "MUL": { "type": "numeric" },
      "CLO": { "type": "numeric" },
      "BEL": { "type": "numeric" },
      "MAL": { "type": "numeric" }
   };
   
   var ht = analytics.newHoeffdingTree(regTestCfg, htParams);
   var examplesN = 0, N = 0;
   var streamData = fs.openRead("./sandbox/ht/wind.dat");
   var err = 0.0;
   console.say("Building a model");
   while (!streamData.eof) {
      var line = streamData.getNextLn().split(",");
      // get discrete attributes
      var example_discrete = line.slice(1, 3);
      // get numeric attributes
      var example_numeric = line.slice(0, 1);
      
      example_numeric = example_numeric.concat(line.slice(3));
      for (var i = 0; i < example_numeric.length; ++i) {
         example_numeric[i] = parseFloat(example_numeric[i]);
      }
      
      var target = parseFloat(line[line.length-1]);
      ht.process(example_discrete, example_numeric, target);
   }
   console.say("Exporting the model as ./sandbox/ht/wind.gv");
   // export the model 
   ht.exportModel({ "file": "./sandbox/ht/wind.gv", "type": "DOT" });
}

function realRegressionTest() {
   // var configFilePath = "./sandbox/ht/airline_14col.names";
   var configFilePath = "./sandbox/ht/winequality.names";
   
   var jsonCfg = names2json(configFilePath);
   
   // console.say(jsonCfg.dataFormat);
   
   var htParams = {
      "gracePeriod": 300,
      "splitConfidence": 1e-6,
      "tieBreaking": 0.005,
      "conceptDriftP": false,
      "phAlpha": 0.005,
      "phLambda": 50.0,
      "phInit": 100,
      "maxNodes": 50,
      "sdrThreshold": 0.06
   };
   
   var ht = analytics.newHoeffdingTree(jsonCfg, htParams);
   // var fin = fs.openRead('./sandbox/ht/airline_14col.dat');
   var fin = fs.openRead('./sandbox/ht/winequality.dat');
   while (!fin.eof) {
      var line = fin.getNextLn();
      var exampleJson = line2array(line, jsonCfg);
      // console.say(exampleJson.numeric.join());
      ht.process(exampleJson.discrete, exampleJson.numeric, exampleJson.target);
   }
   
   console.say("Exporting the model as winequality.gv");
   // The model is huge, because one of the discrete attributes has more than 3000 values 
   ht.exportModel({ "file": "./sandbox/ht/winequality.gv", "type": "DOT" });
}

console.say("In case you get an error of the form \"File 'file_path' does not exist\", it means a dataset is missing.");
console.say("If this the case, run `sh fetch-datasets.sh` to download missing files.");
console.say("Press ENTER to continue");
console.start();

console.say(" --- Example using classification HoeffdingTree --- ");
console.say("- First classification scenario using bootstrapped SEA dataset -");
testClassificationContAttr();
console.say("- Second classification secnario using bootstrapped TITANIC dataset -");
testClassification();


console.say("Breakfast at Tiffany's :)");
testRegressionDrift();
console.say(" --- Example using regression HoeffdingTree --- ");
console.say("- Regression scenario with discrete attributes -");
testRegressionDisAttr();
console.say("- Regression scenario with numeric attributes -");
testRegressionContAttr();
console.say("- Regression scenario using WIND dataset -");
testRegression();
console.say("- Regression scenario using winequality dataset -");
realRegressionTest();

console.say("Interactive mode: empty line to release (press ENTER).");
console.start();

