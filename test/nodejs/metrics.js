/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');
var analytics = qm.analytics;

describe('Classification Metrics Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                // ClassificationScore constructor tests
                new analytics.metrics.ClassificationScore();
                new analytics.metrics.ClassificationScore([1, 0], [0, 1]);
                new analytics.metrics.ClassificationScore(new qm.la.Vector([1, 0]), new qm.la.Vector([0, 1]));
                // PredictionCurve constructor tests
                new analytics.metrics.PredictionCurve();
                new analytics.metrics.PredictionCurve([1, 0], [0, 1]);
                new analytics.metrics.PredictionCurve(new qm.la.Vector([1, 0]), new qm.la.Vector([0, 1]));
            });
        });
    });

    describe('Fuctions Tests', function () {
        var true_lables = [0, 0, 0, 0, 0];
        var pred_lables = [0, 0, 0, 0, 0];

        var true_lables2 = [0, 1, 0, 0 ,1];
        var pred_lables2 = [1, 0, 0, 1, 0];

        var true_lables3 = [0, 1, 0, 0 ,1];
        var pred_lables3 = [0, 1, 0, 0 ,0];

        describe('Acuracy Score', function () {
            it('true_lables = [0, 0, 0, 0, 0], pred_lables = [0, 0, 0, 0, 0], AccuracyScore should be 1', function () {
                assert.equal(1, analytics.metrics.accuracyScore(true_lables, pred_lables));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [1, 0, 0, 1, 0], AccuracyScore should be 0.2', function () {
                assert.equal(0.2, analytics.metrics.accuracyScore(true_lables2, pred_lables2));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [0, 1, 0, 0, 0], AccuracyScore should be 0.8', function () {
                assert.eqtol(0.8, analytics.metrics.accuracyScore(true_lables3, pred_lables3), 1e-2);
            });
        });

        describe('Precision Score', function () {
            it('true_lables = [0, 0, 0, 0, 0], pred_lables = [0, 0, 0, 0, 0], PrecisionScore should be 1', function () {
                assert.equal(1, analytics.metrics.precisionScore(true_lables, pred_lables));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [1, 0, 0, 1, 0], PrecisionScore should be 0', function () {
                assert.equal(0, analytics.metrics.precisionScore(true_lables2, pred_lables2));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [0, 1, 0, 0, 0], PrecisionScore should be 1', function () {
                assert.equal(1, analytics.metrics.precisionScore(true_lables3, pred_lables3));
            });
        });

        describe('Recall Score', function () {
            it('true_lables = [0, 0, 0, 0, 0], pred_lables = [0, 0, 0, 0, 0], RecallScore should be 1', function () {
                assert.equal(1, analytics.metrics.recallScore(true_lables, pred_lables));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [1, 0, 0, 1, 0], RecallScore should be 0', function () {
                assert.equal(0, analytics.metrics.recallScore(true_lables2, pred_lables2));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [0, 1, 0, 0, 0], RecallScore should be 0.5', function () {
                assert.eqtol(0.5, analytics.metrics.recallScore(true_lables3, pred_lables3), 1e-2);
            });
        });

        describe('F1 Score', function () {
            it('true_lables = [0, 0, 0, 0, 0], pred_lables = [0, 0, 0, 0, 0], F1 should be 1', function () {
                assert.equal(1, analytics.metrics.f1Score(true_lables, pred_lables));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [1, 0, 0, 1, 0], F1 should be 0', function () {
                assert.equal(0, analytics.metrics.f1Score(true_lables2, pred_lables2));
            });
            it('true_lables = [0, 1, 0, 0 ,1], pred_lables = [0, 1, 0, 0, 0], F1 should be 0.66', function () {
                assert.eqtol(0.66, analytics.metrics.f1Score(true_lables3, pred_lables3), 1e-2);
            });
        });

    });
});

describe('Regression Metrics Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var me = new analytics.metrics.MeanError();
                var me2 = new analytics.metrics.MeanError();
                var mae = new analytics.metrics.MeanAbsoluteError();
                var mae2 = new analytics.metrics.MeanAbsoluteError();
                var mse = new analytics.metrics.MeanSquareError();
                var mse2 = new analytics.metrics.MeanSquareError();
                var rmse = new analytics.metrics.RootMeanSquareError();
                var rmse2 = new analytics.metrics.RootMeanSquareError();
                var mape = new analytics.metrics.MeanAbsolutePercentageError();
                var mape2 = new analytics.metrics.MeanAbsolutePercentageError();
                var r2 = new analytics.metrics.R2Score();
                var r22 = new analytics.metrics.R2Score();
            });
        });
        it('should create an objects with initial error value -1', function () {
            var me = new analytics.metrics.MeanError();
            var mae = new analytics.metrics.MeanAbsoluteError();
            var mse = new analytics.metrics.MeanSquareError();
            var rmse = new analytics.metrics.RootMeanSquareError();
            var mape = new analytics.metrics.MeanAbsolutePercentageError();
            var r2 = new analytics.metrics.R2Score();

            assert.equal(me.getError(), -1);
            assert.equal(mae.getError(), -1);
            assert.equal(mse.getError(), -1);
            assert.equal(rmse.getError(), -1);
            assert.equal(mape.getError(), -1);
            assert.equal(r2.getError(), -1);
        });
        it('should sucessfully create two different ME objects', function () {
            var me = new analytics.metrics.MeanError();
            var me2 = new analytics.metrics.MeanError();

            me.push(1, 1);
            assert.equal(me.getError(), 0);
            assert.equal(me2.getError(), -1);
        });
        it('should sucessfully load identical state as it was saved', function () {
            var me = new analytics.metrics.MeanError();
            var me2 = new analytics.metrics.MeanError();

            me.push(1, 1);

            var fout = new qm.fs.FOut('me.bin');
            me.save(fout);
            fout.flush();
            fout.close();

            var fin = new qm.fs.FIn('me.bin');
            me2.load(fin);
            fin.close();

            var fin = new qm.fs.FIn('me.bin');
            var me3 = new analytics.metrics.MeanError(fin);
            fin.close();

            assert.equal(me.getError(), me2.getError());
            assert.equal(me.getError(), me3.getError())
        });
        it('should throw an exception if input parameters are not of type number', function () {
            var me = new analytics.metrics.MeanError()

            // test online version
            assert.throws(function () { me.push("a") });
            assert.throws(function () { me.push(2, "a") });
            assert.throws(function () { me.push(2, 2, "a") });

            // test offline version
            assert.throws(function () { analytics.metrics.meanError("a") });
            assert.throws(function () { analytics.metrics.meanError(2, "a") });
            assert.throws(function () { analytics.metrics.meanError(2, 2, "a") });
            assert.throws(function () { analytics.metrics.meanError(2) });
            assert.throws(function () { analytics.metrics.meanError([2, 2]) });
        })
    });

    describe('Fuctions Tests', function () {
        var error = undefined;

        var true_vals = [2, 3, 4, 5, 6];
        var pred_vals = [1, 2, 3, 4, 5];

        var true_vals2 = [1, 2, 3, 4, 5];
        var pred_vals2 = [3, 4, 5, 6, 7];


        describe('ME (Mean Error) Tests', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.MeanError();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, ME should be 0', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 0);
            });
            it('true_val=3, pred_val=5, ME should be -2', function () {
                error.push(3, 5);
                assert.equal(error.getError(), -2);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], ME should be 1', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.equal(error.getError(), 1);
                // test batch version
                assert.equal(analytics.metrics.meanError(true_vals, pred_vals), 1);
                assert.equal(analytics.metrics.meanError(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 1);
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], ME should be -2', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.equal(error.getError(), -2);
                // test batch version
                assert.equal(analytics.metrics.meanError(true_vals2, pred_vals2), -2);
                assert.equal(analytics.metrics.meanError(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), -2);
            });
        });

        describe('MAE (Mean Absolute Error) Tests', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.MeanAbsoluteError();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, MAE should be 0', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 0);
            });
            it('true_val=3, pred_val=5, MAE should be 2', function () {
                error.push(3, 5);
                assert.equal(error.getError(), 2);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], MAE should be 1', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.equal(error.getError(), 1);
                // test batch version
                assert.equal(analytics.metrics.meanAbsoluteError(true_vals, pred_vals), 1);
                assert.equal(analytics.metrics.meanAbsoluteError(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 1);
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], MAE should be 2', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.equal(error.getError(), 2);
                // test batch version
                assert.equal(analytics.metrics.meanAbsoluteError(true_vals2, pred_vals2), 2);
                assert.equal(analytics.metrics.meanAbsoluteError(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), 2);
            });
        });

        describe('MSE (Mean Square Error) Tests', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.MeanSquareError();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, MSE should be 0', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 0);
            });
            it('true_val=3, pred_val=5, MSE should be 2', function () {
                error.push(3, 5);
                assert.equal(error.getError(), 4);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], MSE should be 1', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.equal(error.getError(), 1);
                // test batch version
                assert.equal(analytics.metrics.meanSquareError(true_vals, pred_vals), 1);
                assert.equal(analytics.metrics.meanSquareError(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 1);
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], MSE should be 4', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.equal(error.getError(), 4);
                // test batch version
                assert.equal(analytics.metrics.meanSquareError(true_vals2, pred_vals2), 4);
                assert.equal(analytics.metrics.meanSquareError(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), 4);
            });
        });

        describe('RMSE (Root Mean Square Error) Tests', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.RootMeanSquareError();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, RMSE should be 0', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 0);
            });
            it('true_val=3, pred_val=5, RMSE should be 2', function () {
                error.push(3, 5);
                assert.equal(error.getError(), 2);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], RMSE should be 1', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.equal(error.getError(), 1);
                // test batch version
                assert.equal(analytics.metrics.rootMeanSquareError(true_vals, pred_vals), 1);
                assert.equal(analytics.metrics.rootMeanSquareError(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 1);
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], RMSE should be 2', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.equal(error.getError(), 2);
                // test batch version
                assert.equal(analytics.metrics.rootMeanSquareError(true_vals2, pred_vals2), 2);
                assert.equal(analytics.metrics.rootMeanSquareError(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), 2);
            });
        });

        describe('MAPE (Mean Absolute Percentage Error) Tests', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.MeanAbsolutePercentageError();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, MAPE should be 0', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 0);
            });
            it('true_val=3, pred_val=5, MAPE should be 66.67', function () {
                error.push(3, 5);
                assert.eqtol(error.getError(), 66.67, 1e-2);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], MAPE should be 29', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.eqtol(error.getError(), 29);
                // test batch version
                assert.eqtol(analytics.metrics.meanAbsolutePercentageError(true_vals, pred_vals), 29);
                assert.eqtol(analytics.metrics.meanAbsolutePercentageError(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 29)
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], MAPE should be 91.33', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.eqtol(error.getError(), 91.33, 1e-2);
                // test batch version
                assert.eqtol(analytics.metrics.meanAbsolutePercentageError(true_vals2, pred_vals2), 91.33, 1e-2);
                assert.eqtol(analytics.metrics.meanAbsolutePercentageError(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), 91.33, 1e-2)
            });
        });

        describe('R2 (R Squared) Score', function () {
            // define the error metrics instance before each test
            beforeEach(function () {
                error = new analytics.metrics.R2Score();
            });
            afterEach(function () {
                error = undefined;
            });

            // tests
            it('true_val=1, pred_val=1, R2 should be 1', function () {
                error.push(2, 2);
                assert.equal(error.getError(), 1);
            });
            it('true_val=3, pred_val=5, R2 should be 0', function () {
                error.push(3, 5);
                assert.equal(error.getError(), 0);
            });
            it('true_vals = [2, 3, 4, 5, 6], pred_vals = [1, 2, 3, 4, 5], R2 should be 0.5', function () {
                for (var i in true_vals) {
                    error.push(true_vals[i], pred_vals[i]);
                }
                assert.equal(error.getError(), 0.5);
                // test batch version
                assert.equal(analytics.metrics.r2Score(true_vals, pred_vals), 0.5);
                assert.equal(analytics.metrics.r2Score(new qm.la.Vector(true_vals), new qm.la.Vector(pred_vals)), 0.5)
            });
            it('true_vals = [1, 2, 3, 4, 5], pred_vals = [3, 4, 5, 6, 7], R2 should be -1', function () {
                for (var i in true_vals2) {
                    error.push(true_vals2[i], pred_vals2[i]);
                }
                assert.equal(error.getError(), -1);
                // test batch version
                assert.equal(analytics.metrics.r2Score(true_vals2, pred_vals2), -1);
                assert.equal(analytics.metrics.r2Score(new qm.la.Vector(true_vals2), new qm.la.Vector(pred_vals2)), -1);
            });
        });

    });

});