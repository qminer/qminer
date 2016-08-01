/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

module.exports = exports = function (pathQmBinary) {    
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    exports = qm.statistics;

    //!STARTJSDOC

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
    
    //!ENDJSDOC

    return exports;
}