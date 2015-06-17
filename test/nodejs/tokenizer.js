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
