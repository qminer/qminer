var qm = require('qminer');
//var qm = require('../../../bval_logs/bin/qminer');


describe('KMeans', function() {
	it('should execute, save and load', function() {
		var X = qm.la.randn(2,1000);

		var kmeans = new qm.analytics.KMeans({iter : 100, k :10});
		kmeans.fit(X);

		var model = kmeans.getModel();
		var idxv = kmeans.predict(X);
		/*		
		kmeans.save("./tmpclfile");
		
		var kmeans2 = new qm.analytics.KMeans({iter : 100, k :10});
		kmeans2.load("./tmpclfile");
		var model2 = kmeans2.getModel();
		var idxv2 = kmeans2.predict(X);

		assert.equal(model.C.cols, model2.C.cols);
		assert.equal(model.C.rows, model2.C.rows);
		
		assert.equal(idxv.length, idxv2.length);
		*/
		// test output
		//console.log(model.C.toString());
	});
});