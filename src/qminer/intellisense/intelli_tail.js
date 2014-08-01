
// Manual override

_vec[0] = 1; // vec is indexed and returns numbers
_intVec[0] = 1; // vec is indexed and returns numbers
_spMat[0] = _spVec; // spMat is indexed and returns sparse column vectors
_rs[0] = _rec; // record set at index returns a record
_store[0] = _rec; // store index operator returns a record
_addIntellisenseVar("_rsArr", "[_rs]");
