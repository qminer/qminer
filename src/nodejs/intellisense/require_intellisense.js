/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
// trick from https://github.com/ishisaka/nodeintellisense/
function require(name) {
    return require.modules[name];
};

// Contains modules recognized by require()
// Add a property to this object to support
// other modules.
require.modules = {};