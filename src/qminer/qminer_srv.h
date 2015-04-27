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

#ifndef QMINERSRV_H
#define QMINERSRV_H

#include <qminer.h>
#include <net.h>

namespace TQm {

///////////////////////////////////////////
// QMiner-Server-Function
class TSrvFun : public TSAppSrvFun {
protected:
	TWPt<TBase> Base;
protected:
	TSrvFun(const TWPt<TBase>& _Base, const TStr& FunNm, const TSAppOutType& OutType): 
		 TSAppSrvFun(FunNm, OutType), Base(_Base) { }

	const TWPt<TBase>& GetBase() const { return Base; }
public:
	static void RegDefFun(const TWPt<TBase>& Base, TSAppSrvFunV& SrvFunV);
};

///////////////////////////////////////////
// QMiner-Server-Function-Exit
//  stops qminer server
class TSfExit: public TSrvFun {
private:
	TSfExit(const TWPt<TBase>& Base): TSrvFun(Base, "exit", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfExit(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-Fail
//  for testing failover
class TSfFail: public TSrvFun {
private:
	TSfFail(const TWPt<TBase>& Base): TSrvFun(Base, "fail", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfFail(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) { 
        FailR("FailoverTest"); return ""; }
};

///////////////////////////////////////////
// QMiner-Server-Function-Stores
//  lists all stores in the base and their definiton
class TSfStores: public TSrvFun {
private:
	TSfStores(const TWPt<TBase>& Base):	TSrvFun(Base, "qm_stores", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfStores(Base); }
	static PJsonVal GetStoreJson(const TWPt<TBase>& Base, const TWPt<TStore>& Store);

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-WordVoc
//  lists all complete vocabulray for given key
class TSfWordVoc: public TSrvFun {
private:
	void GetWordVoc(const TStrKdV& FldNmValPrV, TStrIntPrV& WordStrFqV); 

	TSfWordVoc(const TWPt<TBase>& Base): TSrvFun(Base, "qm_wordvoc", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfWordVoc(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-Record
//  lists all the fields and values from a record
class TSfStoreRec: public TSrvFun {
private:
	// helper functions for parsing input parameters
	TWPt<TStore> GetStore(const TStrKdV& FldNmValPrV) const;
	TRec GetRec(const TStrKdV& FldNmValPrV, const TWPt<TStore>& Store) const;

	TSfStoreRec(const TWPt<TBase>& Base): TSrvFun(Base, "qm_record", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfStoreRec(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-Debug
//  dumps statistics to disk
class TSfDebug: public TSrvFun {
private:
	TSfDebug(const TWPt<TBase>& Base): TSrvFun(Base, "qm_debug", saotJSon) { }
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfDebug(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-PartialFlush
//  executes partial flush of data from memory to disk
class TSfPartialFlush : public TSrvFun {
private:
	TSfPartialFlush(const TWPt<TBase>& Base) : TSrvFun(Base, "qm_partial_flush", saotJSon) {}
public:
	static PSAppSrvFun New(const TWPt<TBase>& Base) { return new TSfPartialFlush(Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};
}  // namespace

#endif
