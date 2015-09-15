/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics

describe('OnlineRegressionMetrics tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanError();
                var me2 = new metrics.MeanError();
                var mae = new metrics.MeanAbsoluteError();
                var mae2 = new metrics.MeanAbsoluteError();
                var mse = new metrics.MeanSquareError();
                var mse2 = new metrics.MeanSquareError();
                var rmse = new metrics.RootMeanSquareError();
                var rmse2 = new metrics.RootMeanSquareError();
                var mape = new metrics.MeanAbsolutePercentageError();
                var mape2 = new metrics.MeanAbsolutePercentageError();
                var r2 = new metrics.R2Score();
                var r22 = new metrics.R2Score();
            });
        })
        it('should create an objects with initial error value -1', function () {
            var metrics = analytics.metrics.OnlineRegressionMetrics;
            var me = new metrics.MeanError();
            var mae = new metrics.MeanAbsoluteError();
            var mse = new metrics.MeanSquareError();
            var rmse = new metrics.RootMeanSquareError();
            var mape = new metrics.MeanAbsolutePercentageError();
            var r2 = new metrics.R2Score();
            assert.equal(me.getError(), -1);
            assert.equal(mae.getError(), -1);
            assert.equal(mse.getError(), -1);
            assert.equal(rmse.getError(), -1);
            assert.equal(mape.getError(), -1);
            assert.equal(r2.getError(), -1);
        })
        it('should sucessfully create two different ME objects', function () {
            var metrics = analytics.metrics.OnlineRegressionMetrics;
            var me = new metrics.MeanError()
            var me2 = new metrics.MeanError()

            me.update(1, 1);
            assert.equal(me.getError(), 0);
            assert.equal(me2.getError(), -1);
        })
    });

    describe('Fuctions Tests', function () {

        describe('ME (Mean Error) Tests', function () {
            it('true_num=1, pred_num=1, ME should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanError();
                me.update(2, 2);
                assert.equal(me.getError(), 0);
            });
            it('true_num=3, pred_num=5, ME should be -2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanError();
                me.update(3, 5);
                assert.equal(me.getError(), -2);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], ME should be 1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanError();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    me.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(me.getError(), 1);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], ME should be -2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanError();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    me.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(me.getError(), -2);
            });
        });

        describe('MAE (Mean Absolute Error) Tests', function () {
            it('true_num=1, pred_num=1, MAE should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mae = new metrics.MeanAbsoluteError();
                mae.update(2, 2);
                assert.equal(mae.getError(), 0);
            });
            it('true_num=3, pred_num=5, MAE should be 2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mae = new metrics.MeanAbsoluteError();
                mae.update(3, 5);
                assert.equal(mae.getError(), 2);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], MAE should be 1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanAbsoluteError();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    me.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(me.getError(), 1);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], MAE should be 2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var me = new metrics.MeanAbsoluteError();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    me.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(me.getError(), 2);
            });
        });

        describe('MSE (Mean Square Error) Tests', function () {
            it('true_num=1, pred_num=1, MSE should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mse = new metrics.MeanSquareError();
                mse.update(2, 2);
                assert.equal(mse.getError(), 0);
            });
            it('true_num=3, pred_num=5, MSE should be 2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mse = new metrics.MeanSquareError();
                mse.update(3, 5);
                assert.equal(mse.getError(), 4);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], MSE should be 1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mse = new metrics.MeanSquareError();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    mse.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(mse.getError(), 1);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], MSE should be 4', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mse = new metrics.MeanSquareError();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    mse.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(mse.getError(), 4);
            });
        });

        describe('RMSE (Root Mean Square Error) Tests', function () {
            it('true_num=1, pred_num=1, RMSE should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var rmse = new metrics.RootMeanSquareError();
                rmse.update(2, 2);
                assert.equal(rmse.getError(), 0);
            });
            it('true_num=3, pred_num=5, RMSE should be 2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var rmse = new metrics.RootMeanSquareError();
                rmse.update(3, 5);
                assert.equal(rmse.getError(), 2);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], RMSE should be 1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var rmse = new metrics.RootMeanSquareError();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    rmse.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(rmse.getError(), 1);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], RMSE should be 2', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var rmse = new metrics.RootMeanSquareError();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    rmse.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(rmse.getError(), 2);
            });
        });

        describe('MAPE (Mean Absolute Percentage Error) Tests', function () {
            it('true_num=1, pred_num=1, MAPE should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mape = new metrics.MeanAbsolutePercentageError();
                mape.update(2, 2);
                assert.equal(mape.getError(), 0);
            });
            it('true_num=3, pred_num=5, MAPE should be 66.67', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mape = new metrics.MeanAbsolutePercentageError();
                mape.update(3, 5);
                assert.eqtol(mape.getError(), 66.67, 1e-2);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], MAPE should be 29', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mape = new metrics.MeanAbsolutePercentageError();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    mape.update(true_nums[i], pred_nums[i]);
                }
                assert.eqtol(mape.getError(), 29);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], MAPE should be 91.33', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var mape = new metrics.MeanAbsolutePercentageError();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    mape.update(true_nums[i], pred_nums[i]);
                }
                assert.eqtol(mape.getError(), 91.33, 1e-2);
            });
        });

        describe('R2 (R SQUARED) Score', function () {
            it('true_num=1, pred_num=1, R2 should be 1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var r2 = new metrics.R2Score();
                r2.update(2, 2);
                assert.equal(r2.getError(), 1);
            });
            it('true_num=3, pred_num=5, R2 should be 0', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var r2 = new metrics.R2Score();
                r2.update(3, 5);
                assert.equal(r2.getError(), 0);
            });
            it('true_nums = [2, 3, 4, 5, 6], pred_nums = [1, 2, 3, 4, 5], R2 should be 0.5', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var r2 = new metrics.R2Score();
                var true_nums = [2, 3, 4, 5, 6];
                var pred_nums = [1, 2, 3, 4, 5];

                for (var i in true_nums) {
                    r2.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(r2.getError(), 0.5);
            });
            it('true_nums = [1, 2, 3, 4, 5], pred_nums = [3, 4, 5, 6, 7], R2 should be -1', function () {
                var metrics = analytics.metrics.OnlineRegressionMetrics;
                var r2 = new metrics.R2Score();
                var true_nums = [1, 2, 3, 4, 5];
                var pred_nums = [3, 4, 5, 6, 7];

                for (var i in true_nums) {
                    r2.update(true_nums[i], pred_nums[i]);
                }
                assert.equal(r2.getError(), -1);
            });
        });

    });
    
});
