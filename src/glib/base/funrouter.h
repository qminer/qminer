/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef FUNROUTER_H
#define	FUNROUTER_H

///////////////////////////////
/// Router to constructors based on object types.
/// Useful for creating and de-serializing derived objects, such as TAggr and TStreamAggr.
template <typename TFun>
class TFunRouter {
private:
    /// Object descriptions
	THash<TStr, TFun> TypeNmToFunH;
    
public:
    /// Register default stream aggregates
    TFunRouter() { }
    
    /// Register new object
    void Register(const TStr& TypeNm, TFun Fun) {
        if (TypeNmToFunH.IsKey(TypeNm)) {
            throw TExcept::New("[TFunRouter::Register] Already registered object type " + TypeNm);
        }
        TypeNmToFunH.AddDat(TypeNm, Fun); }
    
    /// Get the function for given type
    TFun Fun(const TStr& TypeNm) {
        if (TypeNmToFunH.IsKey(TypeNm)) { return TypeNmToFunH.GetDat(TypeNm); }
        throw TExcept::New("[TFunRouter::Fun] Unknown object type " + TypeNm);
    }

    /// Get the vector of type names
    void GetTypeNmV(TStrV& Vec) {
        TypeNmToFunH.GetKeyV(Vec);
    }
};

#endif
