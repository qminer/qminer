var qm = require('qminer');

var X = qm.la.randn(2,1000);
var kmeans = new qm.analytics.KMeans({iter : 100, k :10});
kmeans.fit(X);
var model = kmeans.getModel();

idxv = kmeans.predict(X);

// test output
//console.log(model.C.toString());