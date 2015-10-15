/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
// typical use case: pathPrefix = 'Release' or pathPrefix = 'Debug'. Empty argument is supported as well (the first binary that the bindings finds will be used)
module.exports = exports = function (pathPrefix) {
    pathPrefix = pathPrefix || '';
    var qm = require('bindings')(pathPrefix + '/qm.node');
    exports = qm.statistics;


    /**
	 * Calculates the z-score for a point sampled from a Gaussian distribution. The z-score indicates
	 * how many standard deviations an element is from the meam and can be calculated using
	 * the following formula: z = (x - mu) / sigma
	 *
	 * @param {Number} x - the sampled point
	 * @param {Number} mu - mean of the distribution
	 * @param {Number} sigma - variance of the distribution
	 */
    exports.getZScore = function (x, mu, sigma) {
    	return (x - mu) / sigma;
    }
    
    return exports;
}