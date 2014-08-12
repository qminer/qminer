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
