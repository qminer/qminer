var qm = require('qminer');
var base = new qm.Base({ mode: 'createClean' });

base.createStore({
    "name": "People",
    "fields": [
        { "name": "Name", "type": "string", "primary": true },
        { "name": "Gender", "type": "string", "shortstring": true }
    ]
});
var store = base.store("People");