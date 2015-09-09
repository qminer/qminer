/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var qm = require('qminer');
var assert = require('assert');

var base = new qm.Base({
    mode: 'createClean',
    schema: [
      {
          name: 'People',
          fields: [{ name: 'name', type: 'string' }],
      }
    ]
});

var store = base.store('People');

var tokenizer = new qm.analytics.Tokenizer({
    type: "unicode"
});

base.close();
