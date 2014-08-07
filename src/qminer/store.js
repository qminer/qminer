/////// Store prototype
//#- `qm.storeProto` -- the prototype object for qminer store. Implemented in store.js, every store inherits from it.
qm.storeProto = function() {};
//#- `sa = store.addStreamAggr(param)` -- creates a new stream aggregate `sa` and registers it to the store
qm.storeProto.addStreamAggr = function (param) { return qm.newStreamAggr(param, this.name);}
