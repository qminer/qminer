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
    this.update = function (err) {
        error = calcError.update(err);
    }
    // getter for error
    this.getError = function () {
        return error;
    }
    return this;
}

//////////// MEAN ERROR
//#- `me = evaluation.newMeanError()` -- create new (online) mean error instance.
//#   - `me.update(num)` -- updates error with new sample `num`
//#   - `num = me.getError()` -- returns current error `num`
exports.newMeanError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (err) {
            this.sumErr += err;
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// MEAN ABSOLUTE ERROR
//#- `mae = evaluation.newMeanAbsoluteError()` -- create new (online) mean absolute error instance.
//#   - `mae.update(num)` -- updates error with new sample `num`
//#   - `num = mae.getError()` -- returns current error `num`
exports.newMeanAbsoluteError = function () {
    function calcError () {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (err) {
            this.sumErr += Math.abs(err);
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// MEAN SQUARE ERROR
//#- `mse = evaluation.newMeanSquareError()` -- create new (online) mean square error instance.
//#   - `mse.update(num)` -- updates error with new sample `num`
//#   - `num = mse.getError()` -- returns current error `num`
exports.newMeanSquareError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (err) {
            this.sumErr += (err*err);
            this.count++;
            var error = this.sumErr / this.count;
            return error;
        }
    }
    return new createOnlineMetric(calcError);
}

//////////// ROOT MEAN ABSOLUTE ERROR
//#- `rmse = evaluation.newRootMeanSquareError()` -- create new (online) root mean square error instance.
//#   - `rmse.update(num)` -- updates error with new sample `num`
//#   - `num = rmse.getError()` -- returns current error `num`
exports.newRootMeanSquareError = function () {
    function calcError() {
        this.sumErr = 0;
        this.count = 0;
        // update function
        this.update = function (err) {
            this.sumErr += (err * err);
            this.count++;
            var error = this.sumErr / this.count;
            return Math.sqrt(error);
        }
    }
    return new createOnlineMetric(calcError);
}

// About this module
exports.about = function () {
    var description = "Module with evalutaion metrics.";
    return description;
};