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

#include "qminer_gs_srv.h"

namespace TQm {

/////////////////////////////////////////////////////////////////////////////////////////////

void TGenericStoreSrvFun::RegDefFun(const PGenericBase& Base, TSAppSrvFunV& SrvFunV) {
	TSrvFun::RegDefFun(Base->Base, SrvFunV);
	// register additional functions	
	SrvFunV.Add(TGsSrvFunStoreDef::New(Base));
	SrvFunV.Add(TGsSrvFunQuickInfo::New(Base));
	SrvFunV.Add(TGsSrvFunCreateSchema::New(Base));	
	SrvFunV.Add(TGsSrvFunValidateSchema::New(Base));	
}

TStr TGsSrvFunQuickInfo::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {

	const int store_cnt = GenericBase->Base->GetStores();
	PJsonVal json = TJsonVal::NewObj();
	json->AddToObj("stores", store_cnt);

	uint64 rec_cnt = 0;
	uint64 rec_cnt_max = 0;
	TStr store_with_max;
	for (int i = 0; i < store_cnt; i++) {
		TWPt<TGenericStore> store = GenericBase->GetStoreByStoreIndex(i);

		uint64 rec_c = store->GetRecs();
		rec_cnt += rec_c;
		if (rec_c > rec_cnt_max) {
			rec_cnt_max = rec_c;
			store_with_max = store->GetStoreNm();
		}
	}
	json->AddToObj("records", (double)rec_cnt);

	PJsonVal json2 = TJsonVal::NewObj();
	json2->AddToObj("name", store_with_max);
	json2->AddToObj("records", (double)rec_cnt_max);
	json->AddToObj("store_with_max_records", json2);

	return TJsonVal::GetStrFromVal(json);
}

PJsonVal FindFieldJson(const TStr& Name, const PJsonVal& store_json) {
	PJsonVal fields = store_json->GetObjKey("fields");
	for (int i = 0; i < fields->GetArrVals(); i++) {
		PJsonVal field = fields->GetArrVal(i);
		if (field->GetObjKey("fieldName")->GetStr() == Name)
			return field;
	}
	return TJsonVal::NewNull();
}

TStr TGsSrvFunStoreDef::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	const int Stores = GenericBase->Base->GetStores();
	TJsonValV StoreValV;
	for (int StoreN = 0; StoreN < Stores; StoreN++) {
		TWPt<TStore> Store = GenericBase->Base->GetStoreByStoreN(StoreN);
		TWPt<TGenericStore> GStore = GetStore(Store->GetStoreNm());

		PJsonVal StoreJson = Store->GetStoreJson(GenericBase->Base);

		const TGenericStoreSettings GStoreSettings = GStore->GetStoreSettings();

		for (int i = 0; i < Store->GetFields(); i++) {
			TFieldDesc fd = Store->GetFieldDesc(i);
			PJsonVal FieldJson = FindFieldJson(fd.GetFieldNm(), StoreJson);
			// inject additional data for fields
			TGenericStoreFieldDesc ex_data = GStore->GetGenericStoreFieldDesc(i);
			if (!ex_data.DefValue.Empty() && ex_data.DefValue->GetJsonValType() != jvtUndef)
				FieldJson->AddToObj("default_value", ex_data.DefValue);
			FieldJson->AddToObj("is_small_string", ex_data.IsSmallString);
			FieldJson->AddToObj("location", TCh::GetStr(ex_data.FieldStoreLoc));
			FieldJson->AddToObj("use_as_name", ex_data.UseAsName);
			FieldJson->AddToObj("use_codebook", ex_data.UseCodebook);
			FieldJson->AddToObj("nullable", fd.IsNullable());
		}
		// inject store data 		
		StoreJson->AddToObj("time_window_source_field", GStoreSettings.FieldAsSource);
		StoreJson->AddToObj("time_window_source_field_id", GStoreSettings.FieldId);
		StoreJson->AddToObj("sys_inserted_at_field", GStoreSettings.SysInsertedAtFieldName);
		StoreJson->AddToObj("window_size", (double)GStoreSettings.WindowSize);
		StoreJson->AddToObj("window_type", GStoreSettings.WindowType);		

		StoreValV.Add(StoreJson);
	}

	PJsonVal json = TJsonVal::NewObj();
	json->AddToObj("stores", TJsonVal::NewArr(StoreValV));
	json->AddToObj("index_path", GenericBase->Base->GetFPath());
	return TJsonVal::GetStrFromVal(json);
}

TStr TGsSrvFunCreateSchema::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	TStr req_body = RqEnv->GetHttpRq()->GetBodyAsStr();
	TEnv::Logger->OnStatus("creating new schema...");

	PJsonVal json = TJsonVal::GetValFromStr(req_body);
	GenericBase->CreateSchema(json);

	TEnv::Logger->OnStatus("done.");
	return "{ ok: true }";
}

TStr TGsSrvFunValidateSchema::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	TStr req_body = RqEnv->GetHttpRq()->GetBodyAsStr();
	TEnv::Logger->OnStatus("validating schema...");

	PJsonVal json = TJsonVal::GetValFromStr(req_body);
	TGenericSchema schema = TGenericStoreUtils::ParseSchema(json);
	TGenericStoreUtils::ValidateSchema(GenericBase->Base, schema);
	TEnv::Logger->OnStatus("done.");
	return "{ ok: true }";
}

}