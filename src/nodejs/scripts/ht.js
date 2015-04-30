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
    exports = require('bindings')(pathPrefix + '/qm.node').ht;

//!STARTJSDOC

   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }

//!ENDJSDOC

    return exports;

}