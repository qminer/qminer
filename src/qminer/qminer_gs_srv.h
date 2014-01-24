/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute d.o.o.
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
 */

#ifndef QMINER_GS_SRV_H
#define QMINER_GS_SRV_H

#include "qminer_gs.h"
#include <qminer_srv.h>

namespace TQm {

///////////////////////////////////////////
// QMiner-Server-Function - base class
class TGenericStoreSrvFun : public TSAppSrvFun {

protected:
	PGenericBase GenericBase;

protected:
	TGenericStoreSrvFun(const PGenericBase& _Base, const TStr& FunNm, const TSAppOutType& OutType): 
		TSAppSrvFun(FunNm, OutType), GenericBase(_Base) { }

	const TWPt<TGenericStore> GetStore(const TStr& Name) const { return GenericBase->GetStoreByStoreNm(Name); };

public:
	static void RegDefFun(const PGenericBase& Base, TSAppSrvFunV& SrvFunV);
};

///////////////////////////////////////////

class TGsSrvFunQuickInfo : public TGenericStoreSrvFun {
private:

	TGsSrvFunQuickInfo(const PGenericBase& _Base) :	
		TGenericStoreSrvFun(_Base, "gs_quick_info", saotJSon) { }
public:
	static PSAppSrvFun New(const PGenericBase& _Base) { return new TGsSrvFunQuickInfo(_Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////

class TGsSrvFunStoreDef : public TGenericStoreSrvFun {
private:

	TGsSrvFunStoreDef(const PGenericBase& _Base) :	
		TGenericStoreSrvFun(_Base, "gs_store_def", saotJSon) { }
public:
	static PSAppSrvFun New(const PGenericBase& _Base) { return new TGsSrvFunStoreDef(_Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////

class TGsSrvFunCreateSchema : public TGenericStoreSrvFun {
private:

	TGsSrvFunCreateSchema(const PGenericBase& _Base) :	
		TGenericStoreSrvFun(_Base, "gs_create_schema", saotJSon) { }
public:
	static PSAppSrvFun New(const PGenericBase& _Base) { return new TGsSrvFunCreateSchema(_Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////

class TGsSrvFunValidateSchema : public TGenericStoreSrvFun {
private:

	TGsSrvFunValidateSchema(const PGenericBase& _Base) :	
		TGenericStoreSrvFun(_Base, "gs_validate_schema", saotJSon) { }
public:
	static PSAppSrvFun New(const PGenericBase& _Base) { return new TGsSrvFunValidateSchema(_Base); }

	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

}

#endif