/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef FUNROUTER_H
#define	FUNROUTER_H

///////////////////////////////
/// Router to constructors based on object types.
/// Useful for creating and de-serializing derived objects, such as TAggr and TStreamAggr.
template <class PObj, typename TFun>
class TFunRouter {
private:
    /// Object descriptions
	THash<TStr, TFun> TypeNmToFunH;
    
public:
    /// Register default stream aggregates
    TFunRouter() { }
    
    /// Register new object
    void Register(const TStr& TypeNm, TFun Fun) { TypeNmToFunH.AddDat(TypeNm, Fun); }
    
    /// Get the function for given type
    TFun Fun(const TStr& TypeNm) {
        if (TypeNmToFunH.IsKey(TypeNm)) { return TypeNmToFunH.GetDat(TypeNm); }
        throw TExcept::New("[TFunRouter::Fun] Unknown object type " + TypeNm);
    }
};

#endif
