// QMiner - Open Source Analytics Platform
// 
// Copyright (C) 2014 Jozef Stefan Institute
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License, version 3,
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/////// Store prototype
//#- `qm.storeProto` -- the prototype object for qminer store. Implemented in store.js, every store inherits from it.
qm.storeProto = function() {};
//#- `sa = store.addStreamAggr(param)` -- creates a new stream aggregate `sa` and registers it to the store
qm.storeProto.addStreamAggr = function (param) { return qm.newStreamAggr(param, this.name);}
//#- `str = store.toString()` -- returns a string `str` - a description of `store`
qm.storeProto.toString = function () {
    return JSON.stringify(this.toJSON());
}
//#- `store.each(callback)` -- call `callback` on each element of the store
qm.storeProto.each = function (callback) {
    var iter = this.forwardIter, i = 0;
    while (iter.next()) {
        callback(iter.rec, i++);
    }
}
//#- `arr = store.map(callback)` -- call `callback` on each element of the store and store result to `arr`
qm.storeProto.map = function (callback) {
    var iter = this.forwardIter, i = 0, result = [];
    while (iter.next()) {
        result.push(callback(iter.rec, i++));
    }
    return result;
}
//#- `rs = store.head(num)` -- return record set with first `num` records
qm.storeProto.head = function (recs) {
    var iter = this.forwardIter;
    var recIdV = la.newIntVec();
    while (recIdV.length < recs && iter.next()) {
        recIdV.push(iter.rec.$id);
    }
    return this.newRecSet(recIdV);
}
//#- `rs = store.tail(num)` -- return record set with last `num` records
qm.storeProto.tail = function (recs) {
    var iter = this.backwardIter;
    var recIdV = la.newIntVec();
    while (recIdV.length < recs && iter.next()) {
        recIdV.push(iter.rec.$id);
    }
    return this.newRecSet(recIdV);
}

