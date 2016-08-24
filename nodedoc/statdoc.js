/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
* Statistics module.
* @module statistics
* @example
* // import the modules
* var qm = require('qminer');
* var statistics = qm.statistics;
* // create a vector
* var vec = new qm.la.Vector([0, 1, 2, -1, -2]);
* // calculate the mean value of the vector
* var mean = statistics.mean(vec); // returns 0
*/
/**
	* Calculates the mean value(s).
	* @param {(module:la.Vector | module:la.Matrix)} input - The input the method is used on.
	* @returns {(number | module:la.Vector)}
	* <br>1. If input is {@link module:la.Vector}, returns the mean of the vector.
    * <br>2. If input is {@link module:la.Matrix}, returns a vector of where the i-th value is the mean of i-th column.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var la = qm.la;
    * var statistics = qm.statistics;
    * // create a matrix
    * var mat = new la.Matrix([[1, 2, 1], [-1, 2, -1], [3, 2, 3]]);
    * // calculate the mean of the matrix columns
    * // vector contains the elements [1, 2, 1]
    * var mean = statistics.mean(mat);
	*/
 exports.mean = function (input) { return input instanceof Object.create(require('qminer').la.Vector) ? 0.0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* Calculates the standard deviation(s).
	* @param {(module:la.Vector | module:la.Matrix)} X - The input the method is used on.
	* @param {number} [flag=0] - If set to to 0, it normalizes X by n-1; If set to 1 to, it normalizes by n.
	* @param {number} [dim=1] - Computes the standard deviations along the dimension of X specified by parameter `dim`. 
    * If set to 1, calculates the column standard deviation. If set to 2, calculates the row standard deviation.
	* @returns {(number | module:la.Vector)}
	* <br>1. If X is {@link module:la.Vector}, returns standard deviation of the vector.
    * <br>2. If X is {@link module:la.Matrix}, returns a vector where the i-th value is the standard deviation of the i-th column(row).
    * @example
    * // import modules
    * var qm = require('qminer');
    * var la = qm.la;
    * var statistics = qm.statistics;
    * // create a matrix
    * var mat = new la.Matrix([[1, 2, 1], [-1, 2, -1], [3, 2, 3]]);
    * // calculate the standard deviation of the matrix columns
    * var mean = statistics.std(mat); 
	*/
 exports.std = function (X, flag, dim) { return input instanceof Object.create(require('qminer').la.Vector) ? 0.0 : Object.create(require('qminer').la.Vector.prototype); }
/**
	* Returns an object containing the standard deviation of each column of matrix, mean vector and z-score matrix.
	* @param {module:la.Matrix} mat - The matrix.
	* @param {number} [flag=0] - If set to 0, it normalizes mat by n-1; if set to 1, it normalizes by n.
	* @param {number} [dim=1] - Computes the standard deviations along the dimension of mat specified by parameter `dim`. 
    * If set to 1, calculates the column standard deviation. If set to 2, calculates the row standard deviation.
	* @returns {Object} The object `zscoreResult` containing:
	* <br>`zscoreResult.sigma` - {@link module:la.Vector} of standard deviations of mat used to compute the z-scores.
	* <br>`zscoreResult.mu` - {@link module:la.Vector} of mean values of mat used to compute the z-scores.
	* <br>`zscoreResult.Z` - {@link module:la.Matrix} of z-scores that has mean 0 and variance 1.
    * @example
    * // import modules
    * var qm = require('qminer');
    * var la = qm.la;
    * var statistics = qm.statistics;
    * // create a matrix
    * var mat = new la.Matrix([[1, 2, 1], [-1, 2, -1], [3, 2, 3]]);
    * // calculate the standard deviation of the matrix columns
    * var mean = statistics.zscore(mat);
	*/
 exports.zscore = function (mat, flag, dim) { return {sigma: Object.create(require('qminer').la.Vector.prototype), mu: Object.create(require('qminer').la.Vector.prototype), Z: Object.create(require('qminer').la.Matrix.prototype)}; }
/**
	* function studentCdf calculates Student's t cumulative distribution function (PDF integral from -inf to t)
	* function studentCdf returns 'Alpha' as in the p-value of a Student t-test
	* If you already have a t-value than the studentCdf function has 2 inputs: t-value and degrees of freedom
	* @param {number} val - The t-value value of the sample you want to calculate the p-value for
	* @param {number} df - Degrees of freedom for the sample (if your sample is big n than degrees of freedom in n-1)
	* If you don't have the t-value than studentCdf function has 4 inputs: Value, Mean, Standard deviation and degrees of freedom
	* @param {number} val - The average value of the sample you want to calculate the p-value for
	* @param {number} mean - The mean value of the sample you want to calculate the p-value for
	* @param {number} std - The sample standard deviation of the sample you want to calculate the p-value for
	* @param {number} df - Degrees of freedom for the sample (if your sample is n big then degrees of freedom is n-1)
	* @returns {Alpha}
	*/


    /**
	 * Calculates the z-score for a point sampled from a Gaussian distribution. The z-score indicates
	 * how many standard deviations an element is from the meam and can be calculated using
	 * the following formula: `z = (x - mu) / sigma`.
	 * @param {Number} x - The sampled point.
	 * @param {Number} mu - Mean of the distribution.
	 * @param {Number} sigma - Variance of the distribution.
     * @returns {number} The z-score of the sampled point.
     * @example
     * // import modules
     * var stat = require('qminer').statistics;
     * // calculate the z-score of the sampled point
     * var point = 10;
     * var mu    = 5;
     * var sigma = 5;
     * var zScore = stat.getZScore(point, mu, sigma); // returns 1
	 */
    exports.getZScore = function (x, mu, sigma) {
    	return (x - mu) / sigma;
    }
    
    
