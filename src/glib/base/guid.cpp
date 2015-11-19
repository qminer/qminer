/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "sole.hpp"

//////////////////////////////////////////////
// GUID

TStr TGuid::GenGuid() {
	return sole::uuid1().str().c_str();
}

TStr TGuid::GenSafeGuid() { 
    TChA GuidChA = "GUID_";
    GuidChA += GenGuid();
    GuidChA.ChangeCh('-', '_');
    return GuidChA;
}