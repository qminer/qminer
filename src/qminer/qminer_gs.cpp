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

#include "qminer_gs.h"

namespace TQm {

///////////////////////////////
// Generic in-memory storage
TGenericMemoryStorage::TGenericMemoryStorage(const TStr& _FNmPrefix): 
	FNmPrefix(_FNmPrefix), Access(faCreate) { }

TGenericMemoryStorage::TGenericMemoryStorage(const TStr& _FNmPrefix, 
		const TFAccess& _Access): FNmPrefix(_FNmPrefix), Access(_Access) { 

	// load vector
	TFIn FIn(FNmPrefix); 
	Load(FIn);
	// load rest
	FirstRecordOffset.Load(FIn);
}
	
TGenericMemoryStorage::~TGenericMemoryStorage() { 
	if (Access != faRdOnly) { 
		// save vector
		TFOut FOut(FNmPrefix); 
		Save(FOut); 
		// save rest
		FirstRecordOffset.Save(FOut);
	}
}

void TGenericMemoryStorage::AssertReadOnly() const {
	QmAssertR(((Access==faCreate)||(Access==faUpdate)), FNmPrefix + " opened in Read-Only mode!"); 
}

bool TGenericMemoryStorage::IsValId(const uint64& ValId) const { 
	return (ValId >= (uint64)FirstRecordOffset.Val && ValId < (uint64)FirstRecordOffset.Val + Len()); 
}

void TGenericMemoryStorage::GetVal(const uint64& ValId, TMem& Val) const { 
	Val = operator[]((int)(ValId - FirstRecordOffset)); 
}	

uint64 TGenericMemoryStorage::AddVal(const TMem& Val) { 
	return Add(Val) + FirstRecordOffset; 
}

void TGenericMemoryStorage::SetVal(const uint64& ValId, const TMem& Val) { 
	AssertReadOnly(); 
	operator[]((int)(ValId - FirstRecordOffset)) = Val; 
}
	
void TGenericMemoryStorage::DelVals(int to_delete) {
	if (to_delete > 0) {
		Del(0, to_delete - 1);
		FirstRecordOffset += to_delete;
	}
}

uint64 TGenericMemoryStorage::Len() const { 
	return (uint64 )TVec<TMem, int64>::Len(); 
}

uint64 TGenericMemoryStorage::GetMinId() const { 
	return (uint64)FirstRecordOffset; 
}

uint64 TGenericMemoryStorage::GetMaxId() const { 
	return GetMinId() + Len(); 
}

///////////////////////////////////////////////////////////////////////////////////
// Some constants

const char* const TGenericStoreSettings::SysInsertedAtFieldName = "_sys_inserted_at";


///////////////////////////////////////////////////////////////////////////////////
// TBsonObj methods

int64 TBsonObj::GetMemUsedRecursive(const TJsonVal& JsonVal, bool UseVoc) {
	int64 res = 1; // 1 byte for the type
	TJsonValType t = JsonVal.GetJsonValType();
	if (t == jvtUndef) {  }
	else if (t == jvtNull) {  }
	else if (t == jvtBool) { res += 1; }
	else if (t == jvtNum) { res += 8; }
	else if (t == jvtStr) { 
		res += (UseVoc ? 4 : 4 + JsonVal.GetStr().Len() + 1);
	} else if (t == jvtArr) {
		res += 4; // array length
		for (int i = 0; i < JsonVal.GetArrVals(); i++) {
			res+= GetMemUsedRecursive(*JsonVal.GetArrVal(i), UseVoc);
		}
	} else if (t == jvtObj) {
		TStr curr_name;
		PJsonVal curr_val_pt;
		res += 4; // property-list length
		for (int i = 0; i < JsonVal.GetObjKeys(); i++) {
			JsonVal.GetObjKeyVal(i, curr_name, curr_val_pt);
			res += (UseVoc ? 4 :  4 + curr_name.Len() + 1);
			res += GetMemUsedRecursive(*curr_val_pt, UseVoc);
		}
	}
	return res;
}

void TBsonObj::CreateBsonRecursive(const TJsonVal& JsonVal, TStrHash<TInt, TBigStrPool> *Voc, TSOut& SOut) {
	TJsonValType t = JsonVal.GetJsonValType();
	if (t == jvtUndef) { SOut.Save((char)0); }
	else if (t == jvtNull) { SOut.Save((char)1); }
	else if (t == jvtBool) { SOut.Save((char)2); SOut.Save(JsonVal.GetBool()); }
	else if (t == jvtNum) { SOut.Save((char)3); SOut.Save(JsonVal.GetNum()); }
	else if (t == jvtStr) { 
		SOut.Save((char)4); 
		TStr s = JsonVal.GetStr();
		if (Voc == NULL) {
			s.Save(SOut, false);
		} else {            
			if (Voc->IsKey(s)) {
				TInt id = Voc->GetKeyId(s);
				id.Save(SOut);
			} else {
				TInt id = Voc->AddKey(s);
				id.Save(SOut);
			}
		}
	} else if (t == jvtArr) {
		int arr_len = JsonVal.GetArrVals();
		SOut.Save((char)5); 
		SOut.Save(arr_len); 
		for (int i=0; i<arr_len; i++) {
			CreateBsonRecursive(*JsonVal.GetArrVal(i), Voc, SOut);
		}
	} else if (t == jvtObj) {
		int obj_len = JsonVal.GetObjKeys();
		TStr curr_name;
		PJsonVal curr_val_pt;

		SOut.Save((char)6); 
		SOut.Save(obj_len);  
		for (int i=0; i<obj_len; i++) {
			JsonVal.GetObjKeyVal(i, curr_name, curr_val_pt);
			if (Voc == NULL) {
				curr_name.Save(SOut, false);
			} else {
				if (Voc->IsKey(curr_name)) {
					TInt id = Voc->GetKeyId(curr_name);
					id.Save(SOut);
				} else {
					TInt id = Voc->AddKey(curr_name);
					id.Save(SOut);
				}
			}
			CreateBsonRecursive(*curr_val_pt, Voc, SOut);
		}
	}
}

PJsonVal TBsonObj::GetJsonRecursive(TSIn& SIn, TStrHash<TInt, TBigStrPool>* Voc) {
	PJsonVal res = TJsonVal::New();
	char c = SIn.GetCh();    

	if (c == 0) {
		// do nothing, undefined is default
	} else if (c == 1) {
		res->PutNull(); 
	} else if (c == 2) {
		TBool b; 
		b.Load(SIn); 
		res->PutBool(b); 
	} else if (c == 3) {
		TFlt f; 
		f.Load(SIn); 
		res->PutNum(f);
	} else if (c == 4) {
		if (Voc == NULL){
			TStr s; 
			s.Load(SIn, false); 
			res->PutStr(s);
		} else {
			TInt i;
			i.Load(SIn); 
			res->PutStr(Voc->GetKey(i));
		}
	} else if (c == 5) {
		res->PutArr();
		TInt ti; 
		ti.Load(SIn);
		for (int i=0; i<ti; i++) {
			res->AddToArr(GetJsonRecursive(SIn, Voc));
		}
	} else if (c == 6) {
		res->PutObj();
		TInt ti; 
		ti.Load(SIn);
		if (Voc == NULL){
			TStr s;
			for (int i=0; i<ti; i++) {
				s.Load(SIn, false);
				res->AddToObj(s, GetJsonRecursive(SIn, Voc));
			}
		} else {
			TInt ti2;
			for (int i=0; i<ti; i++) {
				ti2.Load(SIn);
				res->AddToObj(Voc->GetKey(ti2), GetJsonRecursive(SIn, Voc));
			}
		}
	}
	return res;
}

///////////////////////////////////////////////////////////////////////////////////
// TGenericStoreSerializator methods

// initialize object from TFieldDesc - field descriptions
TGenericStoreSerializator::TGenericStoreSerializator(const IFieldDescProvider* FieldDescProvider, const char _TargetStorage){
	TargetStorage = _TargetStorage;
	int field_count = FieldDescProvider->GetFieldCount();

	FixedPartOffset = (int)ceil((float)field_count / 8);
	VarIndexPartOffset = FixedPartOffset;

	int fixed_index_offset = 0;
	int var_index_offset = 0;
	int var_field_count = 0;

	for (int i = 0; i < field_count; i++) {
		const TFieldDesc& field_desc = FieldDescProvider->GetFieldDescription(i);
		TGenericStoreFieldDesc field_desc_ex = FieldDescProvider->GetGenericStoreFieldDesc(i);
		if (field_desc_ex.FieldStoreLoc != TargetStorage)
			continue;
		int fixed_size = 0;
		bool is_fixed = true;
		TFieldType field_type = field_desc.GetFieldType();
		switch (field_type) {
		case oftInt: fixed_size = sizeof(int); break;
		case oftIntV: is_fixed = false; break;
		case oftUInt64: fixed_size = sizeof(uint64); break;
		case oftStr:
			{
				is_fixed = field_desc_ex.UseCodebook; 
				if (is_fixed) 
					fixed_size = sizeof(int);
			};
			break;
		case oftStrV: is_fixed = false; break;
		case oftBool: fixed_size = sizeof(bool); break;
		case oftFlt: fixed_size = sizeof(double); break;
		case oftFltPr: fixed_size = sizeof(double) * 2; break;
		case oftFltV: is_fixed = false; break;
		case oftTm: fixed_size = sizeof(uint64); break;
		case oftNumSpV: 
			throw TQmExcept::New("Unsupported JSon data type for DB storage - oftNumSpV.");
			break;
		case oftBowSpV: 
			throw TQmExcept::New("Unsupported JSon data type for DB storage - oftBowSpV.");
			break;
		}
		VarIndexPartOffset += fixed_size;

		TGenericStoreSerialF rec;
		rec.FieldId = field_desc.GetFieldId();
		rec.InFixedPart = is_fixed;
		rec.IsCodebookString = field_desc_ex.UseCodebook;
		rec.IsShortString = field_desc_ex.IsSmallString;
		rec.NullMapByte = i / 8;
		rec.NullMapMask = 1u << (i % 8);
		rec.Offset = (is_fixed ? fixed_index_offset : var_index_offset);		
		int id = FieldsF.Add(rec);
		FieldsH.AddDat(rec.FieldId, id);

		if (is_fixed) {
			fixed_index_offset += fixed_size;
		} else {
			var_index_offset += sizeof(int);
			var_field_count++;
		}
	}

	// var-index part consists of integers that are offsets for specific field
	VarContentPartOffset = VarIndexPartOffset + var_field_count * sizeof(int);
}

void TGenericStoreSerializator::SerializeUpdate(const PJsonVal& JsonVal, TMem& Source, TMem& Destination, const IFieldDescProvider* FieldDescProvider, const THash<TStr, uint64>& FieldJoinIds, TVec<int>& ChangedFields) {

	TMem internal_mem(VarContentPartOffset);
	TMOut internal_sout; 	

	// copy fixed part
	internal_mem.GenZeros(VarContentPartOffset);
	Assert(internal_mem.Len() <= Source.Len());
	memcpy(internal_mem.GetBf(), Source.GetBf(), VarContentPartOffset);

	for (int i = 0; i < FieldsF.Len(); i++) {
		TGenericStoreSerialF rec = FieldsF[i];		
		const TFieldDesc& fd = FieldDescProvider->GetFieldDescription(rec.FieldId);
		TStr field_name = fd.GetFieldNm();
		TFieldType field_type = fd.GetFieldType();

		if (!JsonVal->IsObjKey(field_name)){			
			// copy the field when no update to it is provided
			TGenericStoreFieldDesc fdx = FieldDescProvider->GetGenericStoreFieldDesc(rec.FieldId);
			if (FieldJoinIds.IsKey(field_name)) {
				if (field_type != oftUInt64)
					throw TQmExcept::New(TStr::Fmt("Received id for child object for field %s, but field is not uint64.", field_name.CStr()));
				uint64 child_id = FieldJoinIds.GetDat(field_name);

				uchar* bf = GetLocationFixed(internal_mem, rec);
				*((uint64*)bf) = child_id;
				continue; 
			}
			if (!rec.InFixedPart) {
				// copy old field value
				int var_content_offset = internal_sout.Len();
				SetLocationVar(internal_mem, rec, var_content_offset);

				int old_bf_len = GetVarPartBfLen(Source, rec);
				if (old_bf_len > 0) {
					uchar* old_bf = GetLocationVar(Source, rec);
					internal_sout.AppendBf(old_bf, old_bf_len);
				}
			} else {
				// do nothing, fixed length variables are already copied
			}
		} else {
			// new value
			PJsonVal json_child = JsonVal->GetObjKey(field_name);
			SetNullFlag(internal_mem, rec, false);

			ChangedFields.Add(fd.GetFieldId());

			if (rec.InFixedPart) {
				SetFixedPart(internal_mem, rec, fd, json_child);
			} else {
				SetVarPart(internal_mem, internal_sout, rec, fd, json_child);
			} 
		}
	} 

	Destination.AddBf(internal_mem.GetBf(), VarContentPartOffset);
	Destination.AddBf(internal_sout.GetBfAddr(), internal_sout.Len());
}

void TGenericStoreSerializator::SetFixedPart(TMem& internal_mem, const TGenericStoreSerialF& rec, const TFieldDesc& fd, const PJsonVal& json_child) {
	uchar* bf = GetLocationFixed(internal_mem, rec);
	TStr field_name = fd.GetFieldNm();
	TFieldType field_type = fd.GetFieldType();

	switch (field_type) {
	case oftBool: 
		{
			if (!json_child->IsBool())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not boolean.", field_name.CStr()));
			bool v = (bool)json_child->GetBool();
			*((bool*)bf) = v;
		}; 
		break;
		//case oftBowSpV:break;
	case oftFlt:
		{
			if (!json_child->IsNum())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not numeric.", field_name.CStr()));
			double v = (double)json_child->GetNum();
			*((double*)bf) = v;
		}; 
		break;
	case oftFltPr:
		{
			if (!json_child->IsArr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not array.", field_name.CStr()));
			if (json_child->GetArrVals() != 2)
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not array - expected 2 fields.", field_name.CStr()));

			double v_flt1;
			double v_flt2;
			PJsonVal json_child2 = json_child->GetArrVal(0);
			if (!json_child2->IsNum())
				throw TQmExcept::New(TStr::Fmt("The first element in the JSon array in data field %s is not numeric.", field_name.CStr()));
			v_flt1 = json_child2->GetNum();
			json_child2 = json_child->GetArrVal(1);
			if (!json_child2->IsNum())
				throw TQmExcept::New(TStr::Fmt("The second element in the JSon array in data field %s is not numeric.", field_name.CStr()));
			v_flt2 = json_child2->GetNum();

			*((double*)bf) = v_flt1;
			*(((double*)bf) + 1) = v_flt2;
		}; 
		break;
		//case oftFltV:break;
	case oftInt:
		{
			if (!json_child->IsNum())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not numeric.", field_name.CStr()));
			int v = (int)json_child->GetNum();
			*((int*)bf) = v;
		}; 
		break;
		//case oftIntV:break;
		//case oftNumSpV:break;
	case oftStr:
		{ // this string should be encoded using a codebook
			if (!json_child->IsStr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not string.", field_name.CStr()));
			TStr s = json_child->GetStr();
			TInt v;
			if (Codebook.IsKey(s))
				v = Codebook.GetKeyId(s);
			else
				v = Codebook.AddKey(s);
			*((int*)bf) = v.Val;
		}; 
		break;
		//case oftStrV:break;
	case oftTm:
		{
			if (!json_child->IsStr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not string that represents DateTime.", field_name.CStr()));
			TStr s = json_child->GetStr();
			uint64 v = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(s, '-', ':', '.', 'T'));
			*((uint64*)bf) = v;
		};
		break;
	case oftUInt64:
		{
			if (!json_child->IsNum())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not numeric.", field_name.CStr()));
			uint64 v = (uint64)json_child->GetNum();
			*((uint64*)bf) = v;
		}; 
		break;
	default:
		throw TQmExcept::New(TStr::Fmt("Unsupported JSon data type for DB storage (fixed part) - %d", field_type));

	}
}

void TGenericStoreSerializator::SetVarPart(TMem& internal_mem, TMOut& internal_sout, const TGenericStoreSerialF& rec, const TFieldDesc& fd, const PJsonVal& json_child) {
	TStr field_name = fd.GetFieldNm();
	TFieldType field_type = fd.GetFieldType();

	int var_content_offset = internal_sout.Len();
	SetLocationVar(internal_mem, rec, var_content_offset);

	switch (field_type) {
		//case oftBool: break;
		//case oftBowSpV:break;
		//case oftFlt:break;
		//case oftFltPr:break;
	case oftFltV:  
		{	
			if (!json_child->IsArr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not array.", field_name.CStr()));
			TFltV v_fltv;
			for (int j = 0; j < json_child->GetArrVals(); j++) {
				PJsonVal json_child2 = json_child->GetArrVal(j);
				if (!json_child2->IsNum())
					throw TQmExcept::New(TStr::Fmt("One of the elements in the JSon array in data field %s is not numeric.", field_name.CStr()));
				v_fltv.Add(json_child2->GetNum());
			}
			v_fltv.Save(internal_sout);
		}; 
		break;
		//case oftInt:break;
	case oftIntV:  
		{
			if (!json_child->IsArr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not array.", field_name.CStr()));
			TIntV v_intv;
			for (int j = 0; j < json_child->GetArrVals(); j++) {
				PJsonVal json_child2 = json_child->GetArrVal(j);
				if (!json_child2->IsNum())
					throw TQmExcept::New(TStr::Fmt("One of the elements in the JSon array in data field %s is not numeric.", field_name.CStr()));
				v_intv.Add((int)json_child2->GetNum());
			}						
			v_intv.Save(internal_sout);
		}; 
		break;
		//case oftNumSpV:break;
	case oftStr:  
		{
			if (!json_child->IsStr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not string.", field_name.CStr()));
			json_child->GetStr().Save(internal_sout, rec.IsShortString);
		}; 
		break;
	case oftStrV:  
		{
			if (!json_child->IsArr())
				throw TQmExcept::New(TStr::Fmt("Provided JSon data field %s is not array.", field_name.CStr()));
			TStrV v_strv;
			for (int j = 0; j < json_child->GetArrVals(); j++) {
				PJsonVal json_child2 = json_child->GetArrVal(j);
				if (!json_child2->IsStr())
					throw TQmExcept::New(TStr::Fmt("One of the elements in the JSon array in data field %s is not string.", field_name.CStr()));
				v_strv.Add(json_child2->GetStr());
			}
			v_strv.Save(internal_sout);
		}; 
		break; 
		//case oftTm:break;
		//case oftUInt64:break;
	default:
		throw TQmExcept::New(TStr::Fmt("Unsupported JSon data type for DB storage (variable part) - %d", field_type));
	}
}

void TGenericStoreSerializator::Serialize(const PJsonVal& JsonVal, TMem& Destination, const IFieldDescProvider* FieldDescProvider, const THash<TStr, uint64>& FieldJoinIds) {
	// reserve fixed space - null map, fixed fields, var-field indexes
	TMem internal_mem(VarContentPartOffset); // this buffer is of fixed length and is accessed randomly, like an array
	TMOut internal_sout; // this buffer is of variable width and is written to in sequential manner, like an output stream

	internal_mem.GenZeros(VarContentPartOffset);

	for (int i = 0; i < FieldsF.Len(); i++) {
		TGenericStoreSerialF rec = FieldsF[i];		

		const TFieldDesc& fd = FieldDescProvider->GetFieldDescription(rec.FieldId);
		TStr field_name = fd.GetFieldNm();
		TFieldType field_type = fd.GetFieldType();
		PJsonVal json_child;
		if (!JsonVal->IsObjKey(field_name)){			
			TGenericStoreFieldDesc fdx = FieldDescProvider->GetGenericStoreFieldDesc(rec.FieldId);
			if (FieldJoinIds.IsKey(field_name)) {
				if (field_type != oftUInt64)
					throw TQmExcept::New(TStr::Fmt("Received id for child object for field %s, but field is not uint64.", field_name.CStr()));
				uint64 child_id = FieldJoinIds.GetDat(field_name);

				uchar* bf = GetLocationFixed(internal_mem, rec);
				*((uint64*)bf) = child_id;
				continue;
			} else if (!fdx.DefValue.Empty()) {
				json_child = fdx.DefValue;
			} else if (fd.IsNullable()) {
				SetNullFlag(internal_mem, rec, true);
				if (!rec.InFixedPart()) { SetLocationVar(internal_mem, rec, internal_sout.Len()); }
				continue;
			} else {
				TStr Str = TJsonVal::GetStrFromVal(JsonVal);
				throw TQmExcept::New(TStr::Fmt("JSon data is missing field - expecting %s.", field_name.CStr()));
			}
		}
		if (json_child.Empty())
			json_child = JsonVal->GetObjKey(field_name);

		if (rec.InFixedPart) {
			SetFixedPart(internal_mem, rec, fd, json_child);
		} else {
			SetVarPart(internal_mem, internal_sout, rec, fd, json_child);
		}
	}

	// append generated data to input parameter
	Destination.AddBf(internal_mem.GetBf(), VarContentPartOffset);
	Destination.AddBf(internal_sout.GetBfAddr(), internal_sout.Len());
}

// go from binary to JSON
void TGenericStoreSerializator::Deserialize(PJsonVal& JsonVal, const TMem& Source, const IFieldDescProvider* FieldDescProvider) const {

	if (!JsonVal->IsObj())
		JsonVal->PutObj();
	for (int i = 0; i < FieldsF.Len(); i++) {
		//TGenericStoreSerialF rec = FieldsF[i];		

		const TFieldDesc& fd = FieldDescProvider->GetFieldDescription(i);
		int og_field_id = fd.GetFieldId();
		if (IsFieldNull(Source, og_field_id))
			continue; // field is NULL, just skip

		TStr field_name = fd.GetFieldNm();
		TFieldType field_type = fd.GetFieldType();

		TThinMIn tmin(Source.GetBf() + VarContentPartOffset, Source.Len() - VarContentPartOffset);

		switch (field_type){
		case oftInt: JsonVal->AddToObj(field_name, GetFieldInt(Source, og_field_id)); break;
		case oftIntV: 
			{
				TIntV v_intv;
				GetFieldIntV(Source, og_field_id, v_intv);
				PJsonVal json_child = TJsonVal::NewArr();
				for (int j = 0; j< v_intv.Len(); j++) {
					json_child->AddToArr(TJsonVal::NewNum(v_intv[j]));
				}
				JsonVal->AddToObj(field_name, json_child);
			}; break;
		case oftUInt64: JsonVal->AddToObj(field_name, (double)GetFieldUInt64(Source, og_field_id)); break;
		case oftStr: JsonVal->AddToObj(field_name, GetFieldStr(Source, og_field_id)); break;
		case oftStrV: 
			{
				TStrV v_strv;
				GetFieldStrV(Source, og_field_id, v_strv);
				PJsonVal json_child = TJsonVal::NewArr();
				for (int j = 0; j< v_strv.Len(); j++) {
					json_child->AddToArr(TJsonVal::NewStr(v_strv[j]));
				}
				JsonVal->AddToObj(field_name, json_child);
			}; break;
		case oftBool: JsonVal->AddToObj(field_name, GetFieldBool(Source, og_field_id)); break;
		case oftFlt: JsonVal->AddToObj(field_name, GetFieldFlt(Source, og_field_id)); break;
		case oftFltPr:  
			{
				TFltV v_fltv;
				GetFieldFltV(Source, og_field_id, v_fltv);
				TFlt v_flt1 = v_fltv[0];
				TFlt v_flt2 = v_fltv[1];
				PJsonVal json_child = TJsonVal::NewArr();
				json_child->AddToArr(TJsonVal::NewNum(v_flt1));
				json_child->AddToArr(TJsonVal::NewNum(v_flt2));
				JsonVal->AddToObj(field_name, json_child);
			}; break;
		case oftFltV:  
			{
				TFltV v_fltv;
				GetFieldFltV(Source, og_field_id, v_fltv);
				PJsonVal json_child = TJsonVal::NewArr();
				for (int j = 0; j< v_fltv.Len(); j++) {
					json_child->AddToArr(TJsonVal::NewNum(v_fltv[j]));
				}
				JsonVal->AddToObj(field_name, json_child);
			}; break;
		case oftTm: 
			{
				TTm Tm;
				GetFieldTm(Source, og_field_id, Tm);
				JsonVal->AddToObj(field_name, Tm.GetStr());
			}; break;
		case oftNumSpV: 
			throw TQmExcept::New("Unsupported JSon data type for DB storage - oftNumSpV.");
			break;
		case oftBowSpV: 
			throw TQmExcept::New("Unsupported JSon data type for DB storage - oftBowSpV.");
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
// TGenericStoreUtils members and methods

TGenericStoreUtils::TMaps TGenericStoreUtils::Maps;

TGenericStoreUtils::TMaps::TMaps() {            
	// field-type map
	FieldTypeMap.AddDat("int") = oftInt;
	FieldTypeMap.AddDat("int_v") = oftIntV;
	FieldTypeMap.AddDat("uint64") = oftUInt64;
	FieldTypeMap.AddDat("string") = oftStr;
	FieldTypeMap.AddDat("string_v") = oftStrV;
	FieldTypeMap.AddDat("bool") = oftBool;
	FieldTypeMap.AddDat("float") = oftFlt;
	FieldTypeMap.AddDat("float_pair") = oftFltPr;
	FieldTypeMap.AddDat("float_v") = oftFltV;
	FieldTypeMap.AddDat("datetime") = oftTm;
	FieldTypeMap.AddDat("num_sp_v") = oftNumSpV;
	FieldTypeMap.AddDat("bow_sp_v") = oftBowSpV;

	// time-window units
	TimeWindowUnitMap.AddDat("hour", "h");
	TimeWindowUnitMap.AddDat("minute", "i");
	TimeWindowUnitMap.AddDat("second", "s");
	TimeWindowUnitMap.AddDat("day", "d");
	TimeWindowUnitMap.AddDat("month", "m");
	TimeWindowUnitMap.AddDat("week", "w");
}

TGenericKeyIndexDesc TGenericStoreUtils::ParseKeyIndexDescInner(const PJsonVal& json, const TGenericStoreSchema& store) {
	// check for mandatory fields
	if (!json->IsObjKey("field"))
		throw TQmExcept::New("Missing key-index field");
	if (!json->IsObjKey("type"))
		throw TQmExcept::New("Missing key-index type");
	// parse out indexed field
	TGenericKeyIndexDesc res;
	res.FieldName = json->GetObjStr("field");
	// check if it is a valid field name
	QmAssertR(store.Fields.IsKey(res.FieldName), "Target field for key-index unknown");
	// get field type to avoid further lookups when indexing
	res.FieldType = store.Fields.GetDat(res.FieldName).GetFieldType();
	// parse out key name, use field name as default
	res.KeyIndexName = json->GetObjStr("name", res.FieldName);
	// get and parse key type
	TStr KeyTypeStr = json->GetObjStr("type");  // mandatory
	if (KeyTypeStr == "value") {
		res.KeyType = oiktValue;
	} else if (KeyTypeStr == "text") {
		res.KeyType = oiktText;
	} else if (KeyTypeStr == "location") {
		res.KeyType = oiktLocation;
	} else {
		throw TQmExcept::New("Unknown key type " +  KeyTypeStr);
	}
	// check field type and index type match
	if (res.FieldType == oftStr && res.IsValue()) {
	} else if (res.FieldType == oftStr && res.IsText()) {
	} else if (res.FieldType == oftStrV && res.IsValue()) {
	} else if (res.FieldType == oftTm && res.IsValue()) {
	} else if (res.FieldType == oftFltPr && res.IsLocation()) {
	} else {
		// not supported, lets complain about it...
		throw TQmExcept::New("Indexing '" + KeyTypeStr + "' not supported for field " + res.FieldName);
	}
	// get and parse sort type
	if (json->IsObjKey("sort")) {
		TStr SortTypeStr = json->GetObjStr("sort");
		if (SortTypeStr == "string") {
			res.SortType = oikstByStr;
		} else if (SortTypeStr == "id") {
			res.SortType = oikstById;
		} else if (SortTypeStr == "number") {
			res.SortType = oikstByFlt;
		} else {
			throw TQmExcept::New("Unsupported sort type " + SortTypeStr);
		}
	} else {
		res.SortType = oikstUndef;
	}
	// pasre out word vocabulary
	res.WordVocName = json->GetObjStr("vocabulary", "");
	return res;
}

TGenericStoreJoinSchema TGenericStoreUtils::ParseJoinDescInner(const PJsonVal& json) {
	if (!json->IsObjKey("name"))
		throw TQmExcept::New("Missing join name");
	if (!json->IsObjKey("type"))
		throw TQmExcept::New("Missing join type");
	if (!json->IsObjKey("store"))
		throw TQmExcept::New("Missing join store");

	TStr JoinName = json->GetObjKey("name")->GetStr();
	TStr JoinType = json->GetObjKey("type")->GetStr();
	TStr JoinStore = json->GetObjKey("store")->GetStr();

	TGenericStoreJoinSchema res;
	res.JoinName = JoinName;
	res.OtherStoreName = JoinStore;
	if (JoinType == "index") {    
		res.IsFieldJoin = false;
	} else if (JoinType == "field") {
		res.IsFieldJoin = true;
	} else {
		throw TQmExcept::New("Unsupported join type");
	}

	if (json->IsObjKey("inverse")){
		res.InverseJoinName = json->GetObjKey("inverse")->GetStr();
	}
	return res;
}

TJoinDesc TGenericStoreUtils::ParseJoinDesc(const PJsonVal& json, const TWPt<TGenericStore>& store, TGenericBase* base) {
	if (!json->IsObjKey("name"))
		throw TQmExcept::New("Missing join name");
	if (!json->IsObjKey("type"))
		throw TQmExcept::New("Missing join type");
	if (!json->IsObjKey("store"))
		throw TQmExcept::New("Missing join store");

	TStr JoinName = json->GetObjKey("name")->GetStr();
	TStr JoinType = json->GetObjKey("type")->GetStr();
	TStr JoinStore = json->GetObjKey("store")->GetStr();

	TJoinDesc res;
	if (JoinType == "index") {
		uchar other_store_id = base->GetStoreId(JoinStore);
		res = TJoinDesc(JoinName, other_store_id, store->GetStoreId(), base->Base->GetIndexVoc());
	} else if (JoinType == "field") {
		uchar other_store_id = base->GetStoreId(JoinStore);
		res = TJoinDesc(JoinName, other_store_id, -1);
	} else {
		throw TQmExcept::New("Unsupported join type");
	}
	return res;
}

TFieldDesc TGenericStoreUtils::ParseFieldDesc(const PJsonVal& json) {
	// load stuff from JSON
	if (!json->IsObjKey("name"))
		throw TQmExcept::New("Missing field name");
	if (!json->IsObjKey("type"))
		throw TQmExcept::New("Missing field type");

	TStr FieldName = json->GetObjKey("name")->GetStr();
	TStr FieldType = json->GetObjKey("type")->GetStr();

	TBool Nullable = false; 
	if (json->IsObjKey("null"))
		Nullable = json->GetObjKey("null")->GetBool();

	// validate
	TValidNm::AssertValidNm(FieldName);

	if (!Maps.FieldTypeMap.IsKey(FieldType))
		throw TQmExcept::New(TStr("Unsupported field type ") + FieldType);

	TFieldType ft = (TFieldType)Maps.FieldTypeMap.GetDat(FieldType).Val;

	return TFieldDesc(FieldName, ft, Nullable);        
}

TGenericStoreFieldDesc TGenericStoreUtils::ParseFieldDescEx(const PJsonVal& json) {
	TGenericStoreFieldDesc res;

	res.IsSmallString = false;
	res.UseAsName = false;
	res.UseCodebook = false;
	//res.DefValue ; //  leave empty
	res.FieldStoreLoc = 'm'; // store in memory

	// load stuff from JSON
	if (json->IsObjKey("shortstring"))
		res.IsSmallString = json->GetObjKey("shortstring")->GetBool();
	if (json->IsObjKey("primary"))
		res.UseAsName = json->GetObjKey("primary")->GetBool();
	if (json->IsObjKey("codebook"))
		res.UseCodebook = json->GetObjKey("codebook")->GetBool();
	if (json->IsObjKey("default"))
		res.DefValue = json->GetObjKey("default");
	if (json->IsObjKey("store")) {
		TStr s = json->GetObjKey("store")->GetStr().ToLc();
		if (s != "cache" && s != "memory")
			throw TQmExcept::New(TStr::Fmt("Unsupported 'store' flag for field: %s", s.CStr()));
		res.FieldStoreLoc = s.GetCh(0); // use 'c' or 'm'
	}
	return res;
}

TStr TGenericStoreUtils::GetRecNameFromJson(const PJsonVal& json, const IFieldDescProvider* field_defs) {
	TStr res;
	TStr name_field = field_defs->GetNameField();
	if (name_field.Empty()) 
		return res;
	if (!json->IsObjKey(name_field)) 
		throw TQmExcept::New(TStr::Fmt("Missing name field in incomming record: %s", name_field.CStr()));

	PJsonVal json_child = json->GetObjKey(name_field);
	if (!json_child->IsStr())
		throw TQmExcept::New(TStr::Fmt("Name field in incomming record is not of type string: %s", name_field.CStr()));

	res = json_child->GetStr();
	return res;
}


TGenericStoreSchema TGenericStoreUtils::ParseStoreSchema(const PJsonVal& json) {
	TGenericStoreSchema store;

	if (!json->IsObj()) { 
		throw TQmExcept::New("Invalid JSON for store definition."); }

	// name
	if (!json->IsObjKey("name")) {
		throw TQmExcept::New("Missing store name."); }
	store.StoreName = json->GetObjKey("name")->GetStr();

	// id (optional)
	store.Id = 0;
	store.IdSpecified = false;
	if (json->IsObjKey("id")) {
		double dtc = json->GetObjKey("id")->GetNum();
		if (dtc >= 0 && dtc < 255) {
			store.Id = (uchar) dtc;
			store.IdSpecified = true;
		}
	}

	// fields
	if (!json->IsObjKey("fields")) {
		throw TQmExcept::New("Missing field list."); }
	PJsonVal FieldDefs = json->GetObjKey("fields");
	if (!FieldDefs->IsArr()) {
		throw TQmExcept::New("Bad field list."); }

	TStr NameField; // which field is used as name - empty by default
	for (int i = 0; i<FieldDefs->GetArrVals(); i++) {
		PJsonVal FieldDef = FieldDefs->GetArrVal(i);

		TFieldDesc FieldDesc = TGenericStoreUtils::ParseFieldDesc(FieldDef);
		store.Fields.AddDat(FieldDesc.GetFieldNm(), FieldDesc);

		TGenericStoreFieldDesc FieldDescEx = TGenericStoreUtils::ParseFieldDescEx(FieldDef);
		store.FieldsEx.AddDat(FieldDesc.GetFieldNm(), FieldDescEx);

		// determine name field
		if (FieldDescEx.UseAsName) {
			if (!NameField.Empty()) {
				// more than one field is marked as "primary"
				throw TQmExcept::New(TStr::Fmt("More than one field is marked as primary: %s and %s", NameField.CStr(), FieldDesc.GetFieldNm().CStr()));
			}
			if (!FieldDesc.IsStr()) {
				// fields marked as "primary" must be strings
				throw TQmExcept::New(TStr::Fmt("Filed marked as primary must be string: %s", FieldDesc.GetFieldNm().CStr()));
			}
			if (FieldDesc.IsNullable()) {
				// fields marked as "primary" must be strings
				throw TQmExcept::New(TStr::Fmt("Filed marked as primary cannot be nullable: %s", FieldDesc.GetFieldNm().CStr()));
			}
			NameField = FieldDesc.GetFieldNm();
		}
	}

	// indexes
	if (json->IsObjKey("keys")) {
		PJsonVal KeyDefs = json->GetObjKey("keys");
		if (!KeyDefs->IsArr())
			throw TQmExcept::New("Bad key list.");

		for (int i = 0; i<KeyDefs->GetArrVals(); i++) {
			PJsonVal KeyDef = KeyDefs->GetArrVal(i);
			TGenericKeyIndexDesc key_index_desc = TGenericStoreUtils::ParseKeyIndexDescInner(KeyDef, store);
			store.Indexes.Add(key_index_desc);
		}
	}

	// joins
	if (json->IsObjKey("joins")) {
		PJsonVal JoinDefs = json->GetObjKey("joins");
		if (!JoinDefs->IsArr())
			throw TQmExcept::New("Bad join list.");

		for (int i = 0; i<JoinDefs->GetArrVals(); i++) {
			PJsonVal JoinDef = JoinDefs->GetArrVal(i);

			TGenericStoreJoinSchema JoinDesc = TGenericStoreUtils::ParseJoinDescInner(JoinDef);
			store.JoinsStr.Add(JoinDesc);
			bool is_field_join = JoinDesc.IsFieldJoin;
			TStr join_name = JoinDesc.JoinName;

			if (is_field_join) {
				// add new field to store
				TFieldDesc FieldDesc(join_name + "Id", oftUInt64, false, true);
				store.Fields.AddDat(FieldDesc.GetFieldNm(), FieldDesc);

				TGenericStoreFieldDesc FieldDescEx;
				//FieldDescEx.DefValue
				FieldDescEx.FieldStoreLoc = 'm';
				FieldDescEx.IsSmallString = false;
				FieldDescEx.UseAsName = false;
				FieldDescEx.UseCodebook = false;
				store.FieldsEx.AddDat(FieldDesc.GetFieldNm(), FieldDescEx);        
			}
		}
	}

	// fixed window size
	if (json->IsObjKey("window")) {

		PJsonVal WindowSize = json->GetObjKey("window");
		if (!WindowSize->IsNum())
			throw TQmExcept::New("Bad window size parameter.");
		store.StoreSettings.WindowSize = (int)WindowSize->GetNum();
		store.StoreSettings.WindowType = gbcwtLength;

	} else if (json->IsObjKey("timeWindow")) {

		PJsonVal TimeWindow = json->GetObjKey("timeWindow");
		if (!TimeWindow->IsObj())
			throw TQmExcept::New("Bad timeWindow parameter.");

		if (!TimeWindow->IsObjKey("duration"))
			throw TQmExcept::New("Missing duration parameter.");
		PJsonVal Duration = TimeWindow->GetObjKey("duration");
		if (!Duration->IsNum())
			throw TQmExcept::New("Bad duration parameter.");

		if (!TimeWindow->IsObjKey("unit"))
			throw TQmExcept::New("Missing unit parameter.");
		PJsonVal Unit = TimeWindow->GetObjKey("unit");
		if (!Unit->IsStr())
			throw TQmExcept::New("Bad unit parameter.");

		if (!Maps.TimeWindowUnitMap.IsKey(Unit->GetStr()))
			throw TQmExcept::New("Unsupported timeWindow length unit type");

		TStr FieldName = TGenericStoreSettings::SysInsertedAtFieldName;
		if (TimeWindow->IsObjKey("field")) {
			FieldName = TimeWindow->GetObjStr("field");

			store.StoreSettings.InsertP = false;
		} else {
			TFieldDesc FieldDesc(FieldName, oftTm, false, true);
			store.Fields.AddDat(FieldDesc.GetFieldNm(), FieldDesc);

			TGenericStoreFieldDesc FieldDescEx;
			//FieldDescEx.DefValue
			FieldDescEx.FieldStoreLoc = 'c';
			FieldDescEx.IsSmallString = false;
			FieldDescEx.UseAsName = false;
			FieldDescEx.UseCodebook = false;
			store.FieldsEx.AddDat(FieldDesc.GetFieldNm(), FieldDescEx);        

			store.StoreSettings.InsertP = true;
		}

		store.StoreSettings.WindowType = gbcwtDriver;
		store.StoreSettings.FieldAsSource = FieldName;
		uint64 window_len = (int)Duration->GetNum();		

		char c = Maps.TimeWindowUnitMap.GetDat(Unit->GetStr()).GetCh(0);
		if (c == 'h') { window_len *= 60 * 60 * 1000; }
		else if (c == 'i') { window_len *= 60 * 1000; }
		else if (c == 's') { window_len *= 1000; }
		else if (c == 'd') { window_len *= 24 * 60 * 60 * 1000; }
		else if (c == 'w') { window_len *= 7 * 24 * 60 * 60 * 1000; }
		else if (c == 'm') { window_len *= (uint64)30 * 24 * 60 * 60 * 1000; }
		else if (c == 'y') { window_len *= (uint64)365 * 24 * 60 * 60 * 1000;  }

		store.StoreSettings.WindowSize = window_len;

	}
	return store;
}

TGenericSchema TGenericStoreUtils::ParseSchema(const PJsonVal& json) {
	TGenericSchema res;

	if (json->IsArr()) {
		for (int i = 0; i < json->GetArrVals(); i++) {
			res.Add(ParseStoreSchema(json->GetArrVal(i)));
		}    
	} else if (json->IsObjKey("stores")) {
		return ParseSchema(json->GetObjKey("stores"));
	} else {
		res.Add(ParseStoreSchema(json));
	}

	return res;
}

void TGenericStoreUtils::ValidateSchema(const TWPt<TBase>& Base, TGenericSchema& Schema){

	TStrH store_name_list;
	TIntV requested_store_id_list;

	for (int j=0; j<Schema.Len(); j++){
		// unique store names
		TGenericStoreSchema& store_schema = Schema[j];
		TStr store_name = store_schema.StoreName;
		if (store_name_list.IsKey(store_name))
			throw TQmExcept::New(TStr::Fmt("Duplicate store name %s", store_name.CStr()));
		store_name_list.AddDat(store_name, 0);

		// check unique store ids
		if (Schema[j].IdSpecified) {
			int requested_store_id = store_schema.Id;
			for (int k = 0; k < requested_store_id_list.Len(); k++) {
				if (requested_store_id == requested_store_id_list[k])
					throw TQmExcept::New(TStr::Fmt("Duplicate store id %d", requested_store_id));
			}
			requested_store_id_list.Add(requested_store_id);
		}

		// unique field names inside the store
		TStrH field_name_list;
		for (int i = 0; i < store_schema.Fields.Len(); i++) {
			TStr fn = store_schema.Fields[i].GetFieldNm();
			TValidNm::AssertValidNm(fn);
			if (field_name_list.IsKey(fn))
				throw TQmExcept::New(TStr::Fmt("Duplicate field name %s in store %s", fn.CStr(), store_schema.StoreName.CStr()));
			field_name_list.AddDat(fn, i);
		}

		// check that window parameter for field is valid
		if (store_schema.StoreSettings.WindowType == gbcwtDriver) {
			if (!field_name_list.IsKey(store_schema.StoreSettings.FieldAsSource))
				throw TQmExcept::New(TStr::Fmt("Field %s should be used as the source for window is store %s, but it doesn't exist.", store_schema.StoreSettings.FieldAsSource.CStr(), store_schema.StoreName.CStr()));
			int index = field_name_list.GetDat(store_schema.StoreSettings.FieldAsSource);
			const TFieldDesc& fd = store_schema.Fields[index];
			if (fd.GetFieldType() != oftTm)
				throw TQmExcept::New(TStr::Fmt("Field %s should be used as the source for window is store %s, but it is not of datetime type.", store_schema.StoreSettings.FieldAsSource.CStr(), store_schema.StoreName.CStr()));
			if (fd.IsNullable())
				throw TQmExcept::New(TStr::Fmt("Field %s should be used as the source for window is store %s, but it is nullable.", store_schema.StoreSettings.FieldAsSource.CStr(), store_schema.StoreName.CStr()));
		}		

		// joins
		TStrH join_name_list;
		for (int i = 0; i < store_schema.JoinsStr.Len(); i++){
			TStr jn = store_schema.JoinsStr[i].JoinName;
			if (join_name_list.IsKey(jn))
				throw TQmExcept::New(TStr::Fmt("Duplicate join name %s in store %s", jn.CStr(), store_schema.StoreName.CStr()));
			join_name_list.AddDat(jn, 0);

			// check if other store exists
			TStr other_store_name = store_schema.JoinsStr[i].OtherStoreName;
			
			// B: join can refer to same store
			//if (other_store_name == store_name)
			//	throw TQmExcept::New(TStr::Fmt("Illegal join %s in store %s - self-reference", jn.CStr(), store_name.CStr()));

            // check if a store exists with that name in the schemas given
			bool found = false;
			for (int k=0; k<Schema.Len(); k++){
				if (Schema[k].StoreName == other_store_name){
					found = true;
					break;
				}
			}
            // check for an existing store with that name
            if(!found) {
                for (int k=0; k < Base->GetStores(); k++){
                    if (Base->GetStoreByStoreN(k)->GetStoreNm() == other_store_name) {
                        found = true;
                        break;
                    }
                }
            }
			if (!found)
				throw TQmExcept::New(TStr::Fmt("Illegal join %s in store %s - other store %s not found", jn.CStr(), store_name.CStr(), other_store_name.CStr()));
		}
	}
}

////////////////////////////////////////////////////////////
void TGenericStore::GetMem(const bool& UseMemory, const uint64& RecId, TMem& Rec) const {
	if (UseMemory) JsonM.GetVal(RecId, Rec);
	else JsonC.GetVal(RecId, Rec);
}

bool TGenericStore::UseMemoryData(const int &FieldId) const { 
	return (ExtraData.GetDat(FieldId).FieldStoreLoc.Val != 'c'); 
}

PGenericStoreSerializator TGenericStore::GetAppropriateSerializator(const bool& MemoryData) const { 
	return (MemoryData ? SerializatorMem : SerializatorCache); 
}

PGenericStoreSerializator TGenericStore::GetAppropriateSerializator(const int &FieldId) const {
	if (ExtraData.GetDat(FieldId).FieldStoreLoc.Val == 'c') return SerializatorCache;
	return SerializatorMem;
}

void TGenericStore::ProcessChangedFields(const uint64& RecId, TVec<int>& changed_fields, 
		PGenericStoreSerializator& ser, TMem& src, TMem& dest) {

	TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
	for (int i = 0; i < changed_fields.Len(); i++) {
		int field_id = changed_fields[i];
		if (HasNameField.Val && field_id == NameFieldId) {			
			RemoveIdFromNmH(RecId);
			TStr new_name = ser->GetFieldStr(dest, field_id);
			if (!new_name.Empty()) {
				NmH.AddDat(new_name, RecId);
			}
		}

		// indexes
		for (int j = 0; j < KeyIndexes.Len(); j++) {
			TGenericKeyIndexDesc* index = &KeyIndexes[j];
			const int index_field_id = index->FieldId;
			if (index_field_id != field_id) { continue; }
			const TFieldDesc& fd = GetFieldDesc(index_field_id);
			const int index_key_id = IndexVoc->GetKeyId(this->GetStoreId(), index->KeyIndexName);
			if (ser->IsFieldNull(src, field_id)) {
				// if old value is null, just index new value
				if (fd.GetFieldType() == oftStr && index->IsValue()) {
					TStr new_val = ser->GetFieldStr(dest, field_id);
					Index->Index(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftStr && index->IsText()) {
					TStr new_val = ser->GetFieldStr(dest, field_id);
					Index->IndexText(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftStrV && index->IsValue()) {
					TStrV val_new; ser->GetFieldStrV(dest, field_id, val_new);
					for (int k = 0; k < val_new.Len(); k++) {
						Index->Index(GetStoreId(), index->KeyIndexName, val_new[k], RecId);
					}
				} else if (fd.GetFieldType() == oftTm && index->IsValue()) {
					// TODO: proper time indexing
					uint64 new_val_ui = ser->GetFieldTmAsUInt64(dest, field_id);
					TStr new_val = TUInt64::GetStr(new_val_ui);
					Index->Index(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftFltPr && index->IsLocation()) {
					TFltPr new_val = ser->GetFieldFltPr(dest, field_id);
					Index->Index(index_key_id, new_val, RecId);
				} else {
					throw TQmExcept::New(TStr::Fmt("Unsuperted field and index type combination: %s[%s]: %s", 
						fd.GetFieldNm().CStr(), fd.GetFieldTypeStr().CStr(), index->GetKeyType().CStr())); 
				}
			} else {
				// otherwise delete old and index new, if value changed
				if (fd.GetFieldType() == oftStr && index->IsValue()) {
					TStr old_val = ser->GetFieldStr(src, field_id);
					TStr new_val = ser->GetFieldStr(dest, field_id);
					if (old_val == new_val) { continue; }
					Index->Delete(index_key_id, old_val, RecId);
					Index->Index(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftStr &&index->IsText()) {
					TStr old_val = ser->GetFieldStr(src, field_id);
					TStr new_val = ser->GetFieldStr(dest, field_id);
					if (old_val == new_val) { continue; }
					Index->DeleteText(index_key_id, old_val, RecId);
					Index->IndexText(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftStrV && index->IsValue()) {
					TStrV val_old, val_new;
					ser->GetFieldStrV(src, field_id, val_old);
					ser->GetFieldStrV(dest, field_id, val_new);
					for (int k = 0; k < val_old.Len(); k++) {
						Index->Delete(GetStoreId(), index->KeyIndexName, val_old[k], RecId);
					}
					for (int k = 0; k < val_old.Len(); k++) {
						Index->Index(GetStoreId(), index->KeyIndexName, val_new, RecId);
					}
				} else if (fd.GetFieldType() == oftTm && index->IsValue()) {
					// TODO: proper time indexing
					uint64 old_val_ui = ser->GetFieldTmAsUInt64(src, field_id);
					uint64 new_val_ui = ser->GetFieldTmAsUInt64(dest, field_id);
					if (old_val_ui == new_val_ui) { continue; }
					TStr old_val = TUInt64::GetStr(old_val_ui);
					TStr new_val = TUInt64::GetStr(new_val_ui);
					Index->Delete(index_key_id, old_val, RecId);
					Index->Index(index_key_id, new_val, RecId);
				} else if (fd.GetFieldType() == oftFltPr && index->IsLocation()) {
					TFltPr old_val = ser->GetFieldFltPr(src, field_id);
					TFltPr new_val = ser->GetFieldFltPr(dest, field_id);
					if (Index->LocEquals(index_key_id, old_val, new_val)) { continue; }
					Index->Delete(index_key_id, old_val, RecId);
					Index->Index(index_key_id, new_val, RecId);
				} else {
					throw TQmExcept::New(TStr::Fmt("Unsuperted field and index type combination: %s[%s]: %s", 
						fd.GetFieldNm().CStr(), fd.GetFieldTypeStr().CStr(), index->GetKeyType().CStr())); 
				}
			}
		}

		// joins are not updated, the programmer needs to take care of that
	}
}


void TGenericStore::RemoveIdFromNmH(const uint64& RecId) {
	// return empty string when no primary key
	if (HasRecNm() && IsRecId(RecId)) { 
		TStr RecNm = GetRecNm(RecId);
		NmH.DelKey(RecNm);
	}
}

TGenericStore::TGenericStore(const TWPt<TGenericBase>& _GenericBase, 
        const uchar& StoreId, const TStr& StoreName, const TWPt<TIndexVoc>& IndexVoc, 
        const TStr& _StoreFNm, const int64& _MxCacheSize): TStore(StoreId, StoreName),
            StoreFNm(_StoreFNm), FAccess(faCreate), Base(_GenericBase->Base),
            GenericBase(_GenericBase), Index(_GenericBase->Base->GetIndex()), 
            JsonC(_StoreFNm + ".Cache", _MxCacheSize, 1024), JsonM(_StoreFNm + ".MemCache") { }

TGenericStore::TGenericStore(const TWPt<TGenericBase>& _GenericBase, 
        const TStr& _StoreFNm, const TFAccess& _FAccess, const int64& _MxCacheSize): 
            TStore(_StoreFNm+".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess), 
            Base(_GenericBase->Base), GenericBase(_GenericBase), 
            Index(_GenericBase->Base->GetIndex()), 
            JsonC(_StoreFNm + ".Cache", _FAccess, _MxCacheSize),
			JsonM(_StoreFNm + ".MemCache", _FAccess)  {

	TFIn FIn(StoreFNm + ".GenericStore");
	HasNameField.Load(FIn);
	NameField.Load(FIn);
	NameFieldId.Load(FIn);
	NmH.Load(FIn);
	KeyIndexes.Load(FIn);
	ExtraData.Load(FIn);
	InverseJoins.Load(FIn);
	StoreSettings.Load(FIn);
	if (StoreSettings.WindowType == gbcwtDriver) {
		StoreSettings.FieldId = GetFieldId(StoreSettings.FieldAsSource); }
	SerializatorCache = TGenericStoreSerializator::New(FIn);
	SerializatorMem = TGenericStoreSerializator::New(FIn);
}

TGenericStore::~TGenericStore() {
	// save if necessary
	if (FAccess != faRdOnly) {
		TEnv::Logger->OnStatus(TStr::Fmt("Saving store '%s'...", GetStoreNm().CStr()));
		// save base store
		{
			TFOut BaseFOut(StoreFNm + ".BaseStore"); 
			SaveStore(BaseFOut);
		}
		// save store parameters
		TFOut FOut(StoreFNm + ".GenericStore");
		HasNameField.Save(FOut);
		NameField.Save(FOut);
		NameFieldId.Save(FOut);
		NmH.Save(FOut); 
		KeyIndexes.Save(FOut);
		ExtraData.Save(FOut);
		InverseJoins.Save(FOut);
		StoreSettings.Save(FOut);
		SerializatorCache->Save(FOut);
		SerializatorMem->Save(FOut);
	} else {
		TEnv::Logger->OnStatus("No saving of generic store " + GetStoreNm() + " neccessary!");
	}
}

TStr TGenericStore::GetRecNm(const uint64& RecId) const { 
	// return empty string when no primary key
	if (!HasRecNm()) { return TStr(); }
	// get the name of primary key
	return GetFieldNmStr(RecId, NameField);
}

uint64 TGenericStore::GetRecId(const TStr& RecNm) const { 
	return (NmH.IsKey(RecNm) ? NmH.GetDat(RecNm) : TUInt64::Mx); 
}

PStoreIter TGenericStore::GetIter() const { 
	return TStoreIterVec::New(JsonC.GetFirstValId(), JsonC.GetLastValId() + 1); 
}

uint64 TGenericStore::AddRec(const PJsonVal& Json) {
	// check if we are given existing record
	if (Json->IsObjKey("$record")) {
		// at the start we don't know the record Id
		uint64 RecId = TUInt64::Mx; 
		// get the name or id of existing record
		PJsonVal RecVal = Json->GetObjKey("$record");
		if (RecVal->IsStr()) {
			// we got name, translate to Id
			TStr RecNm = RecVal->GetStr();
            // check if we can even find this record
            if (!HasRecNm()) { 
                ErrorLog("[TGenericStore::AddRec] $record with record name passed to store without primary key");
                return TUInt64::Mx;
            }
            if (!IsRecNm(RecNm)) {
                ErrorLog("[TGenericStore::AddRec] $record with unknown record name passed ");
                return TUInt64::Mx;
            }
			RecId = GetRecId(RecNm);
		} else if (RecVal->IsNum()) {
			// we got directly Id
			RecId = (uint64)TFlt::Round(RecVal->GetNum());
		} else {
            ErrorLog("[TGenericStore::AddRec] unsupported $record value:");
            ErrorLog(TJsonVal::GetStrFromVal(RecVal));
            return TUInt64::Mx;
		}
		// check if we have anything more than primary field, which would require calling UpdateRec
		if (Json->GetObjKeys() > 1) {
			// overwrite current value with new object
			UpdateRec(RecId, Json);
		}
		// return named record
		return RecId;
	}

	// determine record name
	TStr RecNm = TGenericStoreUtils::GetRecNameFromJson(Json, this);

	// check if inserting record with known name, which would require redirect to UpdateRec
	if (HasRecNm() && IsRecNm(RecNm)) {
		// we already have this record
		const uint64 RecId = GetRecId(RecNm);
		// check if we have anything more than primary field
		if (Json->GetObjKeys() > 1) {
			// overwrite current value with new object
			UpdateRec(RecId, Json);
		}
		// return named record
		return RecId;
	}

	// first insert children for field joins
	THash<TStr, uint64> FieldJoinIds; 
	TVec<TFieldJoinDat> field_joins_to_insert;
	for (int i = 0; i < GetJoins(); i++) {
		TJoinDesc join_desc = GetJoinDesc(i);
		if (!join_desc.IsFieldJoin())  { continue; }

		TStr join_name = join_desc.GetJoinNm();
		if (Json->IsObjKey(join_name)) {
			// get coined record json object
			PJsonVal json_child = Json->GetObjKey(join_name);
			const int join_id = join_desc.GetJoinId();
			TWPt<TStore> this_store = Base->GetStoreByStoreId(GetStoreId());
			const uchar join_store_id = join_desc.GetJoinStoreId();
			TWPt<TStore> join_store = Base->GetStoreByStoreId(join_store_id);
			// insert children record
			const uint64 child_id = GenericBase->AddRec(join_store_id, json_child);
			FieldJoinIds.AddDat(join_name + "Id", child_id);
			// check if inverse join is defined
			if (InverseJoins.IsKey(join_id)) {
				// get join weight (useful only for inverse index joins)
				int child_fq = TFlt::Round(json_child->GetObjNum("$fq", 1.0));
                if (child_fq < 1) { 
                    ErrorLog("[TGenericStore::AddRec] Join frequency must be positive");
                    child_fq = 1;
                }
				// remember inverse join to add later
				TFieldJoinDat field_join_to_insert;
				field_join_to_insert.JoinStore = join_store;
				field_join_to_insert.InverseJoinId = InverseJoins.GetDat(join_id);
				field_join_to_insert.JoinRecId = child_id;
				field_join_to_insert.JoinFq = child_fq;
				field_joins_to_insert.Add(field_join_to_insert);
			}
		} else if (join_desc.IsFieldJoin()) {
			// indicate missing join
			FieldJoinIds.AddDat(join_name + "Id", TUInt64::Mx);
		}
	}

	// always add system field that means "inserted_at"
	Json->AddToObj(TGenericStoreSettings::SysInsertedAtFieldName, TTm::GetCurUniTm().GetStr());

	// store the record to the cached vector 
	uint64 RecId;
	TMem RecCache;
	SerializatorCache->Serialize(Json, RecCache, this, FieldJoinIds);
	RecId = JsonC.AddVal(RecCache);
	// store the record to the memory vector 
	TMem RecMem;
	SerializatorMem->Serialize(Json, RecMem, this, FieldJoinIds);
	const uint64 _RecId = JsonM.AddVal(RecMem);
	EAssert(RecId == _RecId); // this is a serious issue if not true :-)
	// remember name-recordId map when primary field available
	if (!RecNm.Empty()) {
		NmH.AddDat(RecNm, RecId);
	}

	// update indexes
	TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
	for (int i = 0; i < KeyIndexes.Len(); i++) {
		TGenericKeyIndexDesc* index = &KeyIndexes[i];
		if (!Json->IsObjKey(index->FieldName)) { continue; }
		PJsonVal obj_key = Json->GetObjKey(index->FieldName);
		const int index_key_id = IndexVoc->GetKeyId(this->GetStoreId(), index->KeyIndexName);
		if (index->FieldType == oftStr && index->IsValue()){
			TStr val = obj_key->GetStr();
			Index->Index(index_key_id, val, RecId);
		} else if (index->FieldType == oftStr && index->IsText()) {
			TStr val = obj_key->GetStr();
			Index->IndexText(index_key_id, val, RecId);
		} else if (index->FieldType == oftStrV && index->IsValue()) {
			for (int j = 0; j < obj_key->GetArrVals(); j++) {
				TStr ValStr = obj_key->GetArrVal(j)->GetStr();
				Index->Index(index_key_id, ValStr, RecId);
			}
		} else if (index->FieldType == oftTm && index->IsValue()) {
			//TODO: proper time indexing
			uint64 val = GetFieldTmAsUInt64(RecId, index->FieldId);
			Index->Index(index_key_id, TUInt64::GetStr(val), RecId);
		} else if (index->FieldType == oftFltPr && index->IsLocation()) {
			TFltPr val = GetFieldFltPr(RecId, index->FieldId);
			Index->Index(index_key_id, val, RecId);
		} else {
			const TFieldDesc& fd = GetFieldDesc(index->FieldId);
			ErrorLog(TStr::Fmt("[TGenericStore::AddRec] Unsupported field and index type combination: %s[%s]: %s", 
				fd.GetFieldNm().CStr(), fd.GetFieldTypeStr().CStr(), index->GetKeyType().CStr())); 
		}
	}

	// insert inverse field joins
	for (int i = 0; i < field_joins_to_insert.Len(); i++) {
		Index->IndexJoin(
			field_joins_to_insert[i].JoinStore, 
			field_joins_to_insert[i].InverseJoinId, 
			field_joins_to_insert[i].JoinRecId, 
			RecId,
			field_joins_to_insert[i].JoinFq);
	}
	// insert all index joins
	for (int JoinN = 0; JoinN < GetJoins(); JoinN++) {
		TJoinDesc JoinDesc = GetJoinDesc(JoinN);
		if (!JoinDesc.IsIndexJoin()) { continue; }
		// get join parameters
		TStr JoinNm = JoinDesc.GetJoinNm();
		TInt JoinId = JoinDesc.GetJoinId();
		// check if we have it in the input data
		if (Json->IsObjKey(JoinNm)) {
			// index joins must be in an array
			PJsonVal JoinArrVal = Json->GetObjKey(JoinNm);
			if (!JoinArrVal->IsArr()) { 
                ErrorLog("]TGenericStore::AddRec] Expected array for join " + JoinNm); 
                continue;
            }
			// get join parameters
			uchar JoinStoreId = JoinDesc.GetJoinStoreId();			
			for (int JoinRecN = 0 ; JoinRecN < JoinArrVal->GetArrVals(); JoinRecN++) {
				PJsonVal JoinRecVal = JoinArrVal->GetArrVal(JoinRecN);
				uint64 JoinRecId = GenericBase->AddRec(JoinStoreId, JoinRecVal);
				// get join weight, default is 1
				int JoinFq = TFlt::Round(JoinRecVal->GetObjNum("$fq", 1.0));
                if (JoinFq < 1) { 
                    ErrorLog("[TGenericStore::AddRec] Join frequency must be positive");
                    JoinFq = 1;
                }
                // index join
                AddJoin(JoinId, RecId, JoinRecId, JoinFq);
			}
		}
	}
	// call triggers
	OnAdd(Base, RecId);
	// return record Id of the new record
	return RecId;
}

void TGenericStore::UpdateRec(const uint64& RecId, const PJsonVal& Json) {
	//TODO: check if there are any fields from JsonC, if not just ignore it to avoid any loading from disk
	THash<TStr, uint64> FieldJoinIds;
	TVec<int> changed_fields1, changed_fields2;

	TMem Source1;
	TMem Destination1;
	JsonC.GetVal(RecId, Source1);
	SerializatorCache->SerializeUpdate(Json, Source1, Destination1, this, FieldJoinIds, changed_fields1); // field joins cannot be updated

	TMem Source2;
	TMem Destination2;
	JsonM.GetVal(RecId, Source2);
	SerializatorMem->SerializeUpdate(Json, Source2, Destination2, this, FieldJoinIds, changed_fields2); // field joins cannot be updated

	JsonC.SetVal(RecId, Destination1);
	JsonM.SetVal(RecId, Destination2);

	ProcessChangedFields(RecId, changed_fields1, SerializatorCache, Source1, Destination1);
	ProcessChangedFields(RecId, changed_fields2, SerializatorMem, Source2, Destination2);

	OnUpdate(Base, RecId);
}

void TGenericStore::AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // make sure it's index join (changing field joins not supported yet)
    if (!JoinDesc.IsIndexJoin()) { 
        ErrorLog("[TGenericStore::AddJoin] Changing of field joins not supported.");
        return;
    }
    // index join
    Index->IndexJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    // check if inverse join is defined
    if (InverseJoins.IsKey(JoinId)) {
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(Base);
        const int InverseJoinId = InverseJoins.GetDat(JoinId);
        if (JoinStore->GetJoinDesc(InverseJoinId).IsIndexJoin()) {
            Index->IndexJoin(JoinStore, InverseJoinId, JoinRecId, RecId, JoinFq);
        } else {
            ErrorLog("[TGenericStore::AddJoin] Automatic indexing of inverse field joins not supported.");
        }
    }
}

void TGenericStore::DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // make sure it's index join (changing field joins not supported yet)
    if (!JoinDesc.IsIndexJoin()) { 
        ErrorLog("[DelJoin] Changing of field joins not supported.");
        return;
    }
    // delete join
    Index->DeleteJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    // check if inverse join is defined
    if (InverseJoins.IsKey(JoinId)) {
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(Base);
        const int InverseJoinId = InverseJoins.GetDat(JoinId);
        if (JoinStore->GetJoinDesc(InverseJoinId).IsIndexJoin()) {
            Index->DeleteJoin(JoinStore, InverseJoinId, RecId, JoinRecId, JoinFq);
        }else {
            ErrorLog("[DelJoin] Changing of inverse field joins not supported.");
        }
    }
}

void TGenericStore::InitFromSchema(const TGenericStoreSchema& store_schema) {

	StoreSettings =  store_schema.StoreSettings;

	// create fields
	HasNameField = false;
	for (int i = 0; i<store_schema.Fields.Len(); i++) {
		const TFieldDesc& FieldDesc = store_schema.Fields[i];
		int field_id = AddFieldDesc(FieldDesc);
		//FieldIds.AddDat(FieldDesc.GetFieldNm()) = field_id;

		TGenericStoreFieldDesc FieldDescEx = store_schema.FieldsEx[i];
		ExtraData.AddDat(field_id, FieldDescEx);

		// determine name field
		if (FieldDescEx.UseAsName) {
			HasNameField = true;
			NameField = FieldDesc.GetFieldNm();
			NameFieldId = FieldDesc.GetFieldId();
		}
	}
	if (StoreSettings.WindowType == gbcwtDriver) 
		StoreSettings.FieldId = GetFieldId(StoreSettings.FieldAsSource);

	// indexes  
	for (int i = 0; i<store_schema.Indexes.Len(); i++) {
		TGenericKeyIndexDesc key_index_desc = store_schema.Indexes[i];
		key_index_desc.FieldId = GetFieldId(key_index_desc.FieldName);

		// prepare new vocabulary or load existing one
		if (GenericBase->VocH.IsKey(key_index_desc.WordVocName)) {
			key_index_desc.WordVocId = GenericBase->VocH.GetDat(key_index_desc.WordVocName);
		} else {
			key_index_desc.WordVocId = Base->NewIndexWordVoc(key_index_desc.KeyType);
			if (!key_index_desc.WordVocName.Empty()) {
				GenericBase->VocH.AddDat(key_index_desc.WordVocName, key_index_desc.WordVocId);
			}
		}

		int index_key_id = Base->NewFieldIndexKey(
			Base->GetStoreByStoreId(GetStoreId()), 
			key_index_desc.KeyIndexName,
			key_index_desc.FieldId, 
			key_index_desc.WordVocId,
			key_index_desc.KeyType,
			key_index_desc.SortType);
		key_index_desc.KeyId = index_key_id;
		KeyIndexes.Add(key_index_desc);
	}

	// joins will be processed separately

	SerializatorCache = TGenericStoreSerializator::New(this, 'c');
	SerializatorMem = TGenericStoreSerializator::New(this, 'm');
}

bool TGenericStore::IsFieldNull(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->IsFieldNull(mem, FieldId);
}

int TGenericStore::GetFieldInt(const uint64& RecId, const int& FieldId) const { 
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldInt(mem, FieldId);
}

TStr TGenericStore::GetFieldStr(const uint64& RecId, const int& FieldId) const { 
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldStr(mem, FieldId);
}

bool TGenericStore::GetFieldBool(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldBool(mem, FieldId);
}

double TGenericStore::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldFlt(mem, FieldId);
}

TFltPr TGenericStore::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldFltPr(mem, FieldId);
}

uint64 TGenericStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldUInt64(mem, FieldId);
}

void TGenericStore::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	GetAppropriateSerializator(use_memory)->GetFieldStrV(mem, FieldId, StrV);
}

void TGenericStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	GetAppropriateSerializator(use_memory)->GetFieldIntV(mem, FieldId, IntV);
}

void TGenericStore::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	GetAppropriateSerializator(use_memory)->GetFieldFltV(mem, FieldId, FltV);
}

void TGenericStore::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	GetAppropriateSerializator(use_memory)->GetFieldTm(mem, FieldId, Tm);
}

uint64 TGenericStore::GetFieldTmAsUInt64(const uint64& RecId, const int& FieldId) const {
	TMem mem;
	bool use_memory = UseMemoryData(FieldId);
	GetMem(use_memory, RecId, mem);
	return GetAppropriateSerializator(use_memory)->GetFieldTmAsUInt64(mem, FieldId);
}

void TGenericStore::GarbageCollect() {
	if (StoreSettings.WindowType == gbcwtNone) { return; }
	if (Empty()) { return; }
	if (JsonC.Len() == 0) return;
	TEnv::Logger->OnStatusFmt("Gargage Collection in %s", GetStoreNm().CStr());
	TEnv::Logger->OnStatusFmt("  %s records at start", TUInt64::GetStr(GetRecs()).CStr());

	TUInt64V deleted_ids;
	if (StoreSettings.WindowType == gbcwtDriver) {
		// by time-field
		PStoreIter iter = GetIter();
		const uint64 LastRecId = JsonC.GetLastValId();
		if (!IsRecId(LastRecId)) { return; }
		uint64 CurMSecs = StoreSettings.InsertP ? TTm::GetCurUniMSecs() : 
			GetFieldTmAsUInt64(LastRecId, StoreSettings.FieldId);
		const uint64 WindowStartMSecs = CurMSecs - StoreSettings.WindowSize;
		TEnv::Logger->OnStatusFmt("  window: %s - %s", 
			TTm::GetTmFromMSecs(WindowStartMSecs).GetWebLogDateTimeStr(true, "T", false).CStr(),
			TTm::GetTmFromMSecs(CurMSecs).GetWebLogDateTimeStr(true, "T", false).CStr());
		while (iter->Next()) {
			uint64 id = iter->GetRecId();
			uint64 TmMSecs = GetFieldTmAsUInt64(id, StoreSettings.FieldId);
			if (TmMSecs >= WindowStartMSecs) break;
			deleted_ids.Add(id);
		}
		TEnv::Logger->OnStatusFmt("  purging %d records", deleted_ids.Len());
	} else {
		// length
		TEnv::Logger->OnStatusFmt("  window: last %d records", (int)StoreSettings.WindowSize);
		int to_delete_cnt = (int)(JsonM.Len() - (uint64)StoreSettings.WindowSize);
		if (to_delete_cnt > 0) {
			PStoreIter iter = GetIter();		
			while (iter->Next() && to_delete_cnt > 0) {
				deleted_ids.Add(iter->GetRecId());
				to_delete_cnt--;
			}
		}
	}

	TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
	for (int DeleteRecN = 0; DeleteRecN < deleted_ids.Len(); DeleteRecN++) {   
		if (DeleteRecN % 100 == 0) { TEnv::Logger->OnStatusFmt("    %d\r", DeleteRecN); }

		uint64 RecId = deleted_ids[DeleteRecN];
		// triggers
		OnDelete(Base, RecId);

		RemoveIdFromNmH(RecId);

		// delete data from indexes
		for (int KeyN = 0; KeyN < KeyIndexes.Len(); KeyN++) {
			TGenericKeyIndexDesc* KeyDesc = &KeyIndexes[KeyN];
			int IndexedFieldId = GetFieldId(KeyDesc->FieldName);
			const TFieldDesc& IndexedFieldDesc = GetFieldDesc(IndexedFieldId);
			const int KeyId = IndexVoc->GetKeyId(this->GetStoreId(), KeyDesc->KeyIndexName);
			if (IndexedFieldDesc.GetFieldType() == oftStr && KeyDesc->IsValue()) {
				TStr val = GetFieldStr(RecId, IndexedFieldId);
				Index->Delete(KeyId, val, RecId);
			} else if (IndexedFieldDesc.GetFieldType() == oftStr && KeyDesc->IsText()) {
				TStr val = GetFieldStr(RecId, IndexedFieldId);
				Index->DeleteText(KeyId, val, RecId);
			} else if (IndexedFieldDesc.GetFieldType() == oftStrV && KeyDesc->IsValue()) {
				TStrV val, val_new;
				GetFieldStrV(RecId, IndexedFieldId, val);
				for (int k = 0; k < val.Len(); k++) {
					Index->Delete(GetStoreId(), KeyDesc->KeyIndexName, val[k], RecId);
				}
			} else if (IndexedFieldDesc.GetFieldType() == oftTm && KeyDesc->IsValue()) {
				uint64 val = GetFieldTmAsUInt64(RecId, IndexedFieldId);
				Index->Delete(KeyId, TUInt64::GetStr(val), RecId);				
			} else {
				throw TQmExcept::New("Invalid data received for removing from index.");
			}
		}

		// delete data from joins
		for (int JoinN = 0; JoinN < GetJoins(); JoinN++) {
			TJoinDesc JoinDesc = GetJoinDesc(JoinN);
			if (!JoinDesc.IsIndexJoin()) {
                TInt JoinId = JoinDesc.GetJoinId();
                TUInt64IntKdV JoinRecIdFqV;
                Index->GetJoinRecIdFqV(JoinDesc.GetJoinKeyId(), RecId, JoinRecIdFqV);
                for (int j = 0; j < JoinRecIdFqV.Len(); j++) {
                    const uint64 JoinRecId = JoinRecIdFqV[j].Key;
                    const int JoinFq = JoinRecIdFqV[j].Dat;
                    DelJoin(JoinId, RecId, JoinRecId, JoinFq);
                }
            }
		}

	}
	JsonC.DelVals(deleted_ids.Len());
	JsonM.DelVals(deleted_ids.Len());

	TEnv::Logger->OnStatusFmt("  %s records at end", TUInt64::GetStr(GetRecs()).CStr());
}

/////////////////////////////////////////////////////////////////////////////////////////
// QMiner-Base


// constructor for creating new, empty database
TGenericBase::TGenericBase(const TStr& FPath, const uint64& IndexCacheSize, 
		const uint64& _StoreCacheSize, const bool& InitP): StoreCacheSize(_StoreCacheSize) {

	TEnv::Logger->OnStatus("Opening JSON database in create mode");
	Base = TBase::New(FPath, IndexCacheSize);  
	if (InitP) { Base->Init(); }
}


TGenericBase::TGenericBase(const TStr& FPath, const TFAccess& FAccess, 
		const uint64& IndexCacheSize, const uint64& _StoreCacheSize, 
		const TStrUInt64H& StoreCacheSizes, const bool& InitP): StoreCacheSize(_StoreCacheSize) {

	Base = TBase::Load(FPath, FAccess, IndexCacheSize);

	// load stores
	{
		TFIn FIn(FPath + "Master.dat");
		TStrV store_names;
		store_names.Load(FIn);
		for (int i = 0; i < store_names.Len(); i++) {
			TStr store_name = store_names[i];
			uint64 store_cache_size = _StoreCacheSize;
			if (StoreCacheSizes.IsKey(store_name)) {
				store_cache_size = StoreCacheSizes.GetDat(store_name);
			}
			PStore store = new TGenericStore(this, FPath + store_name, FAccess, store_cache_size);
			Base->AddStore(store);
		}
		VocH.Load(FIn);
	}

	if (InitP) { Base->Init(); }
}

void TGenericBase::CreateSchema(const PJsonVal& StoreDefVal) {
	TGenericSchema schema = TGenericStoreUtils::ParseSchema(StoreDefVal);
	TGenericStoreUtils::ValidateSchema(Base, schema);
	// create each store
	for (int i=0; i<schema.Len(); i++) {
		TGenericStoreSchema store = schema[i];
		TStr StoreName = store.StoreName;
		// get store id
		uchar tc = 0;
		if (store.IdSpecified) {
			double dtc = store.Id;
			if (dtc >= 0 && dtc < 255) {
				tc = (uchar) dtc;
				for (int j=0; i<Base->GetStores(); j++) {
					if (tc == Base->GetStoreByStoreN(j)->GetStoreId())
						throw TQmExcept::New("Store id already in use.");
				}
			}
		} else {
			for (int j=0; j<Base->GetStores(); j++) {
				uchar ttc = Base->GetStoreByStoreN(j)->GetStoreId();
				if (tc <= ttc)
					tc = ttc + 1;
			}
		}

		TStr root = Base->GetIndex()->GetIndexFPath();
		TGenericStore *store_obj = new TGenericStore(this, tc, StoreName, 
			Base->GetIndexVoc(), root + StoreName, StoreCacheSize);
		Base->AddStore(store_obj);

		store_obj->InitFromSchema(store);
	}

	// create joins
	for (int i = 0; i < schema.Len(); i++) {
		TGenericStoreSchema store = schema[i];
		TWPt<TGenericStore> store_obj = this->GetStoreByStoreNm(store.StoreName);
		for (int j = 0; j < store.JoinsStr.Len(); j++) {
			TGenericStoreJoinSchema& join = store.JoinsStr[j];
			TJoinDesc jd;
			uchar other_store_id = this->GetStoreId(join.OtherStoreName);
			if (join.IsFieldJoin) {
				// field join
				int field_id = store_obj->GetFieldId(join.JoinName + "Id");
				jd = TJoinDesc(join.JoinName, other_store_id, field_id);
			} else {
				// index join
				jd = TJoinDesc(join.JoinName, other_store_id, this->GetStoreId(store.StoreName), Base->GetIndexVoc());
			}  
			store_obj->AddJoinDesc(jd);
		}
	}

	// handle inverse joins
	for (int i = 0; i < schema.Len(); i++) {
		TGenericStoreSchema store = schema[i];
		TWPt<TGenericStore> store_obj = this->GetStoreByStoreNm(store.StoreName);
		for (int j = 0; j < store.JoinsStr.Len(); j++) {
			TGenericStoreJoinSchema& join = store.JoinsStr[j];
			if (!join.InverseJoinName.Empty()) {
				const int join_id = store_obj->GetJoinId(join.JoinName);
				TJoinDesc join_desc = store_obj->GetJoinDesc(join_id);
				const uchar other_store_id = join_desc.GetJoinStoreId();

				TJoinDesc inverse_join_desc(join.InverseJoinName, this->GetStoreId(store.StoreName), other_store_id, Base->GetIndexVoc());
				TWPt<TGenericStore> other_store_obj = this->GetStoreByStoreId(other_store_id);
				const int other_join_id = other_store_obj->GetJoinId(join.InverseJoinName);

				store_obj->AddInverseJoin(join_id, other_join_id);
			}
		}
	}
}

TGenericBase::~TGenericBase() { 
	if (!Base->IsRdOnly()) {
		Save(Base->GetFPath());
	} else {
		TEnv::Logger->OnStatus("No saving of generic base neccessary!");
	}
}

void TGenericBase::Save(const TStr& FPath) {
	TEnv::Logger->OnStatus("Saving generic base... ");
	TFOut FOut(FPath + "Master.dat");

	TStrV store_names;
	for (int i = 0; i < Base->GetStores(); i++) {
		store_names.Add(Base->GetStoreByStoreN(i)->GetStoreNm());
	}
	store_names.Save(FOut);
	VocH.Save(FOut);
}

uint64 TGenericBase::AddRec(const int StoreId, const PJsonVal& RecVal) {
	QmAssertR(RecVal->IsObj(), "Invalid input JSon, not an object");
	uint64 master_rec_id = GetStoreByStoreId(StoreId)->AddRec(RecVal); 
	return master_rec_id;
}

uint64 TGenericBase::AddRec(const TStr& StoreNm, const PJsonVal& RecVal) {
	QmAssertR(RecVal->IsObj(), "Invalid input JSon, not an object");
	QmAssertR(Base->IsStoreNm(StoreNm), TStr("Unknown store name ") + StoreNm);
	uint64 master_rec_id = GetStoreByStoreNm(StoreNm)->AddRec(RecVal); 
	return master_rec_id;
}

void TGenericBase::UpdateRec(const TWPt<TGenericStore>& Store, const PJsonVal& RecVal) {
	if (!RecVal->IsObj()) 
		throw TQmExcept::New("JSON for update is not an object");
	if (!RecVal->IsObjKey("$id")) 
		throw TQmExcept::New("JSON for update is missing $id");

	uint64 id = (uint64)RecVal->GetObjKey("$id")->GetNum();
	Store->UpdateRec(id, RecVal); 
}

void TGenericBase::UpdateRec(const int StoreId, const PJsonVal& RecVal) {
	UpdateRec(GetStoreByStoreId(StoreId), RecVal);
}

void TGenericBase::UpdateRec(const TStr& StoreNm, const PJsonVal& RecVal) {
	UpdateRec(GetStoreByStoreNm(StoreNm), RecVal);
}

///////////////////////////////////////////////////////////////////////////////// 
// archiving helpers

void TGenericBase::CopyAllFiles(TStr& Src, TStr& Dest) {
	TFFile LiveFiles(Src, ""); 
	TStr file_name;
	while (LiveFiles.Next(file_name)) { 
		TStr dest= file_name;
		dest.ChangeStrAll(Src, Dest);
		TFile::Copy(file_name, dest, false, false); 		
	}	
}

void TGenericBase::DeleteAllFiles(TStr& Src){
	TFFile ArchDir(Src, false); 
	TStr file_name;
	while (ArchDir.Next(file_name)) { 
		if (file_name.GetFPath() != Src)
			continue;
		TFile::Del(file_name); 
	}	
}

void TGenericBase::ArchiveFiles(TStr FPath) {
	TStr FPathArh = GetArchiveDir(FPath);
	DeleteAllFiles(FPathArh);
	TDir::DelDir(FPathArh);
	TDir::GenDir(FPathArh);
	CopyAllFiles(FPath, FPathArh);
}

void TGenericBase::RestoreArchivedFiles(TStr FPath) {
	TStr FPathArh = GetArchiveDir(FPath);
	DeleteAllFiles(FPath);
	CopyAllFiles(FPathArh, FPath);
}

}