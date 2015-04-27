/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
