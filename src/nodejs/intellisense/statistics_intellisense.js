/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
//intellisense start
exports = {}; require.modules.qminer_stat = exports;
//intellisense end
/**
* Statistics module.
* @module statistics
* @example
* // TODO
*/
/**
	* returns mean of vector.
	* @param {(module:la.Vector | module:la.Matrix)} input - input can be vector or a matrix.
	* @returns {(number | module:la.Vector)}
	* <br>1. Number, if input parameters is {@link module:la.Vector}.
    * <br>2. {@link module:la.Vector}, if parameter is {@link module:la.Matrix}.
	*/
 exports.mean = function (input) { return input instanceof Object.create(require('qminer').la.Vector) ? 0.0 : Object.create(require('qminer').la.Vector); }
/**
	* returns vector containing the standard deviation of each column from input matrix.
	* @param {(module:la.Vector | module:la.Matrix)} input - input can be vector or a matrix.
	* @param {number} [flag=0] - Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
	* @param {number} [dim=1] - Computes the standard deviations along the dimension of `mat` specified by parameter `dim`. 1 is col std, 2 is row std.
	* @returns {(number | module:la.Vector)}
	* <br>1. Number, if input parameters is {@link module:la.Vector}.
    * <br>2. {@link module:la.Vector}, if parameter is {@link module:la.Matrix}.
	*/
 exports.std = function (input, flag, dim) { return input instanceof Object.create(require('qminer').la.Vector) ? 0.0 : Object.create(require('qminer').la.Vector); }
/**
	* returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`.
	* @param {module:la.Matrix} input - Matrix
	* @param {number} [flag=0] - Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
	* @param {number} [dim=1] - Computes the standard deviations along the dimension of `mat` specified by parameter `dim`. 1 is col std, 2 is row std.
	* @returns {zscoreResult}
	* <br>zscoreResult.sigma - Standard deviation of input used to compute the z-scores.
	* <br>zscoreResult.mu - Mean of input used to compute the z-scores.
	* <br>zscoreResult.Z - A vector of z-scores has mean 0 and variance 1.
	*/
 exports.zscoreResult = function (mat, flag, dim) { return {sigma:'', mu:'', Z:''}; }
