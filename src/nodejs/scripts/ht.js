/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
 
module.exports = exports = function (pathQmBinary) {    
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    exports = qm.ht;

//!STARTJSDOC

   /**
	* classDesc	
	* @returns {string} desc - Returns class description
	*/
exports.StrStrMap.prototype.classDesc = function () { return 'This class wraps a C++ string-string hash table'; }

//!ENDJSDOC

    return exports;

}