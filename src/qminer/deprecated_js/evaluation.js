/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

///////////////////////////////
//#
//# ### evaluation.js (use require)
//#
//# Evaluation metrics functions.
//# The library must be loaded using `var evaluation = require('evaluation.js');`.
//# 
//# **Functions and properties:**
//#

// object for online metrics model
function createOnlineMetric(updateCallback) {
    var error = -1;
    var calcError = new updateCallback();
    // update function defined with callback function
    this.update = function (true_num, pred_num, ref_num) {
        var pred_num = pred_num == null ? 0 : pred_num;
        var ref_num = ref_num == null ? 0 : ref_num;

        error = calcError.update(true_num, pred_num);
    }
    // getter for error
    this.getError = function () {
        return error;
    }
    return this;
}

//////////// MEAN ERROR (ME)
//#- `me = evaluation.newMeanError()` -- create new (online) mean error instance.
//#   - `me.update(true_num, pred_num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = me.getError()` -- returns current error `num`
exports.newMeanError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (true_num, pred_num) {
            var err = true_num - pred_num;
            this.sumErr += err;
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// MEAN ABSOLUTE ERROR (MAE)
//#- `mae = evaluation.newMeanAbsoluteError()` -- create new (online) mean absolute error instance.
//#   - `mae.update(true_num, pred_num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = mae.getError()` -- returns current error `num`
exports.newMeanAbsoluteError = function () {
    function calcError () {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (true_num, pred_num) {
            var err = true_num - pred_num;
            this.sumErr += Math.abs(err);
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// MEAN SQUARE ERROR (MSE)
//#- `mse = evaluation.newMeanSquareError()` -- create new (online) mean square error instance.
//#   - `mse.update(true_num, pred_num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = mse.getError()` -- returns current error `num`
exports.newMeanSquareError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (true_num, pred_num) {
            var err = true_num - pred_num;
            this.sumErr += (err*err);
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// ROOT MEAN SQUARE ERROR (RMSE)
//#- `rmse = evaluation.newRootMeanSquareError()` -- create new (online) root mean square error instance.
//#   - `rmse.update(true_num, pred_num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = rmse.getError()` -- returns current error `num`
exports.newRootMeanSquareError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (true_num, pred_num) {
            var err = true_num - pred_num;
            this.sumErr += (err * err);
            this.count++;
            var error = this.sumErr / this.count;
            return Math.sqrt(error);
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// MEAN ABSOLUTE PERCENTAGE ERROR (MAPE)
//#- `mape = evaluation.newMeanAbsolutePercentageError()` -- create new (online) mean absolute percentage error instance.
//#   - `mape.update(true_num, pred_num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = mape.getError()` -- returns current error `num`
exports.newMeanAbsolutePercentageError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (true_num, pred_num) {           
            if (true_num != 0) { // skip if true_num is 0, otherwise we have devision by zero in the next step.
                var err = true_num - pred_num;
                this.sumErr += Math.abs(err / true_num) * 100;
            }
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// R SQUARED SCORE (R2)
//#- `r2 = evaluation.newRSquareScore()` -- create new (online) R Square instance. This statistic measures how successful the fit is in explaining the variation of the data. Best possible score is 1.0, lower values are worse.
//#   - `r2.update(num)` -- updates metric with ground truth target value `true_num` and estimated target value `pred_num`.
//#   - `num = rmse.getError()` -- returns current score `num`
exports.newRSquareScore = function () {
    function rSquare() {
        this.sst = 0;
        this.sse = 0;
        this.mean = 0;
        this.count = 0;
        this.sumTrue = 0;
        this.sumTrue2 = 0;
        // update function
        this.update = function (true_num, pred_num) {
            this.count++;
            this.sumTrue += true_num;
            this.sumTrue2 += true_num * true_num;
            this.mean = this.sumTrue / this.count;
            //calculate R squared score 
            this.sse += (true_num - pred_num) * (true_num - pred_num);
            this.sst = this.sumTrue2 - this.count * this.mean * this.mean;
            if (this.sst == 0.0) {
                return (this.sse == 0.0) ? 1.0 : 0.0;
            }
            return 1 - this.sse / this.sst;           
        }
    }
    return new createOnlineMetric(rSquare);
}

// About this module
exports.about = function () {
    var description = "Module with evalutaion metrics.";
    return description;
};