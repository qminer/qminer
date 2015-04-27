/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
exports.addToProcess = function (proc) {
    proc.isArg = function (arg) {
        return process.argv.indexOf(arg) >= 0;
    }
};