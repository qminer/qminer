/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Quintelligence d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Contact: 
 *   Blaz Fortuna <blaz@blazfortuna.com>
 *
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

}  // namespace

#endif
