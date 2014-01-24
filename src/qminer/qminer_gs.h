/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
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

#ifndef QMINER_GS_H
#define QMINER_GS_H

#include "qminer.h"

namespace TQm {

// window types
typedef enum { gbcwtNone = 0, gbcwtLength = 1, gbcwtDriver = 2 } TGenericBlockCacheWindowType;

///////////////////////////////
// Generic in-memory storage

class TGenericMemoryStorage : private TVec<TMem, int64> {
private:
	TStr FNmPrefix;
	TFAccess Access;
	TUInt64 FirstRecordOffset;

public:
	TGenericMemoryStorage(const TStr& _FNmPrefix);
	TGenericMemoryStorage(const TStr& _FNmPrefix, const TFAccess& _Access);
	~TGenericMemoryStorage();

	// asserts if we are allowed to change stuff
	void AssertReadOnly() const;
	bool IsReadOnly() const { return Access == faRdOnly; }

	bool IsValId(const uint64& ValId) const;
	void GetVal(const uint64& ValId, TMem& Val) const; 
	uint64 AddVal(const TMem& Val);
	void SetVal(const uint64& ValId, const TMem& Val);
	void DelVals(int to_delete);

	uint64 Len() const;
	uint64 GetMinId() const;
	uint64 GetMaxId() const;
};

//////////////////////////////////////////////////////////////////////////////
// This class performs (de)serialization of JSON object to 
// binary stream and back.

class TBsonObj {

public:

	static void Serialize(const TJsonVal& JsonVal, TSOut& SOut) { CreateBsonRecursive(JsonVal, NULL, SOut); };
	static void SerializeVoc(const TJsonVal& JsonVal, TStrHash<TInt, TBigStrPool>& Voc, TSOut& SOut) { CreateBsonRecursive(JsonVal, &Voc, SOut); };
	static int64 GetMemUsed(const TJsonVal& JsonVal) { return GetMemUsedRecursive(JsonVal, false); };
	static int64 GetMemUsedVoc(const TJsonVal& JsonVal) { return GetMemUsedRecursive(JsonVal, true); };

	static PJsonVal GetJson(TSIn& SIn) { return GetJsonRecursive(SIn, NULL); };
	static PJsonVal GetJsonVoc(TSIn& SIn, TStrHash<TInt, TBigStrPool>& Voc) { return GetJsonRecursive(SIn, &Voc); };

	static void UnitTest();

private:

	static void CreateBsonRecursive(const TJsonVal& JsonVal, TStrHash<TInt, TBigStrPool> *Voc, TSOut& SOut);
	static int64 GetMemUsedRecursive(const TJsonVal& JsonVal, bool UseVoc);
	static PJsonVal GetJsonRecursive(TSIn& SIn, TStrHash<TInt, TBigStrPool>* Voc);
};

///////////////////////////////////////////////////////////////////////////////////
// forward declarations

class TGenericBase; typedef TPt<TGenericBase> PGenericBase;
class TGenericStore; typedef TPt<TGenericStore> PGenericStore;
class TGenericStoreFieldDesc;

/////////////////////////////////////////////////////////////////////////
// interface for store definition
class IFieldDescProvider {
public:
	virtual int GetFieldCount() const = 0;
	virtual const TFieldDesc& GetFieldDescription(int i) const = 0;
	virtual const TGenericStoreFieldDesc& GetGenericStoreFieldDesc(int i) const = 0;
	virtual const TStr& GetNameField() const = 0;
};

///////////////////////////////////////////////////////////////////////////////////
// this class contains data about field serialization

class TGenericStoreSerialF {
public:
	// offset of byte that contains NULL bit indicator for this field
	TUCh NullMapByte;
	// mask to use on the NULL-bit indicator byte
	TUCh NullMapMask;
	// is this field stored in fixed or variable part
	TBool InFixedPart;
	// offset in fixed or var-index part
	TInt Offset;
	// id of this field inside TStore
	TInt FieldId;
	// is this field actually a string that is encoded using codebook
	TBool IsCodebookString;
	// is this field a short string?
	TBool IsShortString;

	// methods required for (de)serialization of this class inside TVec into TSOut and TSIn
	void Save(TSOut& SOut) const {
		SOut.Save(NullMapByte.Val);
		SOut.Save(NullMapMask.Val);
		InFixedPart.Save(SOut);
		Offset.Save(SOut);
		FieldId.Save(SOut); 
		IsCodebookString.Save(SOut);
		IsShortString.Save(SOut);
	}

	void Load(TSIn& SIn) {
		uchar tc; 
		SIn.Load(tc);
		NullMapByte = tc;
		SIn.Load(tc); 
		NullMapMask = tc; 
		InFixedPart.Load(SIn);
		Offset.Load(SIn); 
		FieldId.Load(SIn); 
		IsCodebookString.Load(SIn);
		IsShortString.Load(SIn);
	}

	TGenericStoreSerialF() {}
	TGenericStoreSerialF(TSIn& SIn){ Load(SIn); }
};

///////////////////////////////////////////////////////////////////////////////////
// this class contains extra data about field, specific to generic store

class TGenericStoreFieldDesc  {
public:
	// Should be used as name field?
	TBool UseAsName;
	// Should be stored in codebook?
	TBool UseCodebook;
	// Is small string?
	TBool IsSmallString;  
	// default value if value not specified
	PJsonVal DefValue;  
	// where will this field be stored- true means 
	TCh FieldStoreLoc;

	// methods required for (de)serialization of this class inside TVec into TSOut and TSIn
	void Save(TSOut& SOut) const {
		UseAsName.Save(SOut);
		UseCodebook.Save(SOut);
		IsSmallString.Save(SOut);
		DefValue.Save(SOut);
		FieldStoreLoc.Save(SOut);
	}

	void Load(TSIn& SIn) {
		UseAsName.Load(SIn);
		UseCodebook.Load(SIn);
		IsSmallString.Load(SIn);
		DefValue = PJsonVal(SIn);
		FieldStoreLoc = TCh(SIn);
	}

	TGenericStoreFieldDesc() {}
	TGenericStoreFieldDesc(TSIn& SIn){ Load(SIn); }
};

///////////////////////////////////////////////////////////////////////////////////
// description of key-index - can be serialized
class TGenericKeyIndexDesc {
public:
	// key id in QMiner index
	TInt KeyId;
	// indexed field
	TStr FieldName;
	TInt FieldId;
	TFieldType FieldType;
	// key name (sometimes different from field name)
	TStr KeyIndexName;
	// index type
	TIndexKeyType KeyType;
	// how to sort the key (if at all)
	TIndexKeySortType SortType;
	// word vocabulary (used by some inverted index)
	TStr WordVocName;
	TInt WordVocId;
	// list of fixed words when given at start
	TStrV WordVocWdV;

public:
	TGenericKeyIndexDesc() {}
	TGenericKeyIndexDesc(TSIn& SIn) { Load(SIn); }

	void Save(TSOut& SOut) const {
		KeyId.Save(SOut); FieldName.Save(SOut); FieldId.Save(SOut); TInt(FieldType).Save(SOut);
		KeyIndexName.Save(SOut); TInt(KeyType).Save(SOut); TInt(SortType).Save(SOut);
		WordVocName.Save(SOut); WordVocId.Save(SOut); WordVocWdV.Save(SOut);
	}

	void Load(TSIn& SIn) {
		KeyId.Load(SIn); FieldName.Load(SIn); FieldId.Load(SIn);
		FieldType = (TFieldType)TInt(SIn).Val; KeyIndexName.Load(SIn);
		KeyType = (TIndexKeyType)TInt(SIn).Val;
		SortType = (TIndexKeySortType)TInt(SIn).Val; 
		WordVocName.Load(SIn); WordVocId.Load(SIn); WordVocWdV.Load(SIn);
	}

	// get index type
	bool IsValue() const { return KeyType == oiktValue; }
	bool IsText() const { return KeyType == oiktText; }
	bool IsLocation() const { return KeyType == oiktLocation; }
	TStr GetKeyType() const { return IsValue() ? "value" : IsText() ? "text" : "location"; }

	// get sort type
	bool IsByStr() const { return SortType == oikstByStr; }
	bool IsById() const { return SortType == oikstById; }
	bool IsByFlt() const { return SortType == oikstByFlt; }
};

////////////////////////////////////////////////////////////////////
// generic-store settings

class TGenericStoreSettings {
public:
	TGenericBlockCacheWindowType WindowType;
	TUInt64 WindowSize;   // for time window this is period length in milliseconds, otherwise it is max length
	TBool InsertP;        // user insert time
	TStr FieldAsSource;   // name of the field that serves as time-window indicator
	TInt FieldId;         // id of FieldAsSource, calculated at run-time
	
	void Save(TSOut& SOut) const {     
		TInt i((int)WindowType);
		i.Save(SOut);
		WindowSize.Save(SOut);
		InsertP.Save(SOut);
		FieldAsSource.Save(SOut);
	}

	void Load(TSIn& SIn) {
		TInt i(SIn);
		WindowType = (TGenericBlockCacheWindowType)i.Val;
		WindowSize.Load(SIn);
		InsertP.Load(SIn);
		FieldAsSource.Load(SIn);
	}

	TGenericStoreSettings() { WindowType = gbcwtNone; }
	TGenericStoreSettings(TSIn& SIn){ Load(SIn); }

	static const char* const SysInsertedAtFieldName;
};

///////////////////////////////////////////////////////////////////////////////////
// this class contains join information that is given by the schema

class TGenericStoreJoinSchema {
public:
	TStr JoinName;
	TStr OtherStoreName;
	TBool IsFieldJoin;
	TStr InverseJoinName;

	TGenericStoreJoinSchema(): IsFieldJoin(false) {}
};

///////////////////////////////////////////////////////////////////////////////////
// this class is used to collect new store information after parsing the definition.
// It is used for validation and servers as the basis for actual creation of new store.

class TGenericStoreSchema {

public:
	TStr StoreName;
	TUCh Id;
	TBool IdSpecified;
	TGenericStoreSettings StoreSettings;
	TStrHash<TFieldDesc> Fields;
	TStrHash<TGenericStoreFieldDesc> FieldsEx;
	TVec<TGenericKeyIndexDesc> Indexes;
	TVec<TGenericStoreJoinSchema> JoinsStr;
};

typedef TVec<TGenericStoreSchema> TGenericSchema;

////////////////////////////////////////////////////////////////////////////////////////
// This clas handles smart serialization of JSON with respect to field definitions.
// It supports NULL flags.
// It packs fixed-width fields together.
// Varialble-width fields are store in two parts - first there's an index array
// so that for each field we store its offset inside the buffer.
// Then there is the real variable-length part where the content is stored.

class TGenericStoreSerializator;
typedef TPt<TGenericStoreSerializator> PGenericStoreSerializator;;

class TGenericStoreSerializator {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TGenericStoreSerializator>;

	// only store fields with this storage flag
	TCh TargetStorage;

	// offset of fixed part
	TInt FixedPartOffset;

	// offset of var-index part
	TInt VarIndexPartOffset;

	// offset of var-content part
	TInt VarContentPartOffset;

	// definition of fields - order of definitions in this vector
	// also means order of serialization
	TVec<TGenericStoreSerialF> FieldsF;

	// mapping from Field id (in TStore) to index inside FieldsF
	THash<TInt, TInt> FieldsH;
	// mapping from Field id (in TStore) to index inside FieldsF
	THash<TInt, TGenericStoreSerialF> FieldsH2;
	// codebook for encoding strings
	TStrHash<TInt, TBigStrPool> Codebook;

	// this method returns index of field record in internal vector
	int GetFRecIndex(const int& FieldId) const {
		if (!FieldsH.IsKey(FieldId))
			throw TQmExcept::New(TStr::Fmt("TGenericStoreSerializator: Field with ID not found: %d", FieldId));
		return FieldsH.GetDat(FieldId);
	}

	// this method finds location inside the buffer for fixed-width fields
	uchar* GetLocationFixed(const TMem& Source, const int& FieldId) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		return GetLocationFixed(Source, rec);
	}

	// this method finds location inside the buffer for fixed-width fields
	uchar* GetLocationFixed(const TMem& Source, const TGenericStoreSerialF& rec) const {
		uchar* bf = (uchar*)Source.GetBf() + FixedPartOffset + rec.Offset;
		Assert(bf < ((uchar*)Source.GetBf() + Source.Len()));
		return bf;
	}

	// this method finds location inside the buffer for variable-width fields
	int GetOffsetVar(const TMem& Source, const int& FieldId) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		uchar* bf = (uchar*)Source.GetBf();
		int offset = *((int*)(bf + VarIndexPartOffset + rec.Offset));
		Assert(VarContentPartOffset + offset < Source.Len());
		return VarContentPartOffset + offset;
	}

	// this method finds location inside the buffer for variable-width fields
	uchar* GetLocationVar(const TMem& Source, const int& FieldId) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		return GetLocationVar(Source, rec);
	}

	// this method finds location inside the buffer for variable-width fields
	uchar* GetLocationVar(const TMem& Source, const TGenericStoreSerialF& rec) const {
		uchar* bf = (uchar*)Source.GetBf();
		int offset = *((int*)(bf + VarIndexPartOffset + rec.Offset));
		uchar* bf2 = (bf + VarContentPartOffset) + offset; 
		Assert(bf2 < ((uchar*)Source.GetBf() + Source.Len()));
		return bf2;
	}

	// this method saves content offset for specified variable field
	void SetLocationVar(const TMem& Bf, const TGenericStoreSerialF& rec, const int& var_content_offset) const {
		Assert(VarIndexPartOffset + rec.Offset <= (Bf.Len() - 4));
		*((int*)(Bf.GetBf() + VarIndexPartOffset + rec.Offset)) = var_content_offset;			
	}

	// this method sets or unsets NULL flag for specified field
	void SetNullFlag(const TMem& Source, const int FieldId, const bool is_null) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		SetNullFlag(Source,rec, is_null);
	}

	// this method sets or unsets NULL flag for specified field
	void SetNullFlag(const TMem& Source, const TGenericStoreSerialF& rec, const bool is_null) const {
		uchar* bf = (uchar*)Source.GetBf() + rec.NullMapByte;
		Assert(bf < ((uchar*)Source.GetBf() + Source.Len()));
		if (is_null)
			*bf |= rec.NullMapMask;
		else
			*bf &= ~rec.NullMapMask;
	}

	// this method calculates length of buffer where given var-length field is stored
	int GetVarPartBfLen(TMem& Source, const TGenericStoreSerialF rec) { 
		uchar* bf = (uchar*)Source.GetBf();
		int offset1 = *((int*)(bf + VarIndexPartOffset + rec.Offset));
		int offset2 = -1;
		if (VarIndexPartOffset + rec.Offset + (int)sizeof(int) < VarContentPartOffset)
			offset2 = *((int*)(bf + VarIndexPartOffset + rec.Offset + sizeof(int)));
		else
			offset2 = Source.Len() - VarContentPartOffset;
		Assert(offset2-offset1 >= 0);
		return offset2-offset1;
	}

	void SetFixedPart(TMem& internal_mem, const TGenericStoreSerialF& rec, const TFieldDesc& fd, const PJsonVal& json_child);
	void SetVarPart(TMem& internal_mem, TMOut& internal_sout, const TGenericStoreSerialF& rec, const TFieldDesc& fd, const PJsonVal& json_child);

	/////////////////////////////////////////////////
	// Thin Input-Memory
	// Used to present existing buffer as TSIn. 
	// It doesn't allocate or release any memory
	// TODO: extend TMIn so it can wrap around buffer and not claim it (simialr to TMOut)
	class TThinMIn: public TSIn{
	private:
		uchar* Bf;
		int BfC, BfL;
	private:
	public:
		TThinMIn(const TMem& Mem) : TSBase("Thin input memory"), TSIn("Thin input memory"), Bf(NULL), BfC(0), BfL(0) { 
			Bf = (uchar*)Mem.GetBf();
			BfL = Mem.Len();
		}
		TThinMIn(const void* _Bf, const int& _BfL) : TSBase("Thin input memory"), TSIn("Thin input memory"), Bf(NULL), BfC(0), BfL(_BfL) { 
			Bf = (uchar*)_Bf;
		}
		bool Eof(){return BfC==BfL;}
		int Len() const {return BfL-BfC;}
		char GetCh(){
			QmAssertR(BfC<BfL, "Reading beyond the end of stream.");
			return Bf[BfC++];
		}
		char PeekCh(){
			QmAssertR(BfC<BfL, "Reading beyond the end of stream.");
			return Bf[BfC];
		}
		int GetBf(const void* LBf, const TSize& LBfL){
			Assert(TSize(BfC+LBfL)<=TSize(BfL));
			int LBfS=0;
			for (TSize LBfC=0; LBfC<LBfL; LBfC++){
				LBfS+=(((char*)LBf)[LBfC]=Bf[BfC++]);}
			return LBfS;
		}
		void Reset(){Cs=TCs(); BfC=0;}
		uchar* GetBfAddr(){return Bf;}
		void MoveTo(int Offset){ 
			QmAssertR(Offset<BfL, "Reading beyond the end of stream.");
			BfC = Offset;
		}
		bool GetNextLnBf(TChA& LnChA) { FailR("TMIn::GetNextLnBf: not implemented"); return false; }
	};

public:
	// constructor inits internal mappings and other structures
	TGenericStoreSerializator(const IFieldDescProvider* FieldDescProvider, const char _TargetStorage);

	// for regenerating from existing settings
	TGenericStoreSerializator(TSIn& SIn) {				
		FixedPartOffset.Load(SIn);
		VarIndexPartOffset.Load(SIn);
		VarContentPartOffset.Load(SIn);
		FieldsF.Load(SIn);
		FieldsH.Load(SIn);
		Codebook.Load(SIn);
		TargetStorage = TCh(SIn);
	}

	static PGenericStoreSerializator New(const IFieldDescProvider* FieldDescProvider, const char _TargetStorage) {
		return new TGenericStoreSerializator(FieldDescProvider, _TargetStorage);
	}
	static PGenericStoreSerializator New(TSIn& SIn) {
		return new TGenericStoreSerializator(SIn);
	}

	// for storing to disk
	void Save(TSOut& SOut) {
		FixedPartOffset.Save(SOut);
		VarIndexPartOffset.Save(SOut);
		VarContentPartOffset.Save(SOut);
		FieldsF.Save(SOut);
		FieldsH.Save(SOut);
		Codebook.Save(SOut);
		TargetStorage.Save(SOut);
	}

	// go from JSON to binary
	void Serialize(const PJsonVal& JsonVal, TMem& Destination, const IFieldDescProvider* FieldDescProvider, const THash<TStr, uint64>& FieldJoinIds);
	void SerializeUpdate(const PJsonVal& JsonVal, TMem& Source, TMem& Destination, const IFieldDescProvider* FieldDescProvider, const THash<TStr, uint64>& FieldJoinIds, TVec<int>& ChangedFields);

	// go from binary to JSON
	void Deserialize(PJsonVal& JsonVal, const TMem& Source, const IFieldDescProvider* FieldDescProvider) const;

	// check if field inside
	bool IsFieldId(const int& FieldId) const {
		return FieldsH.IsKey(FieldId);
	}

	// access methods
	bool IsFieldNull(const TMem& Source, const int& FieldId) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		uchar* bf = (uchar*)Source.GetBf() + rec.NullMapByte;
		return ((*bf & rec.NullMapMask) != 0);
	}
	int GetFieldInt(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return *((int*)bf);
	}
	TStr GetFieldStr(const TMem& Source, const int& FieldId) const {
		int i = GetFRecIndex(FieldId);
		TGenericStoreSerialF rec = FieldsF[i];
		if (rec.InFixedPart) {
			uchar* bf = GetLocationFixed(Source, rec);      
			int codebook_id = *((int*)bf);
			return Codebook.GetKey(codebook_id);
		} else {
			TThinMIn MIn(Source);
			MIn.MoveTo(GetOffsetVar(Source, FieldId));
			TStr s;
			s.Load(MIn, rec.IsShortString);
			return s;
		}
	} 
	bool GetFieldBool(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return *((bool*)bf);
	}
	double GetFieldFlt(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return *((double*)bf);
	}
	TFltPr GetFieldFltPr(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return TFltPr(*((double*)bf), *(((double*)bf) + 1));
	}
	void GetFieldTm(const TMem& Source, const int& FieldId, TTm& Tm) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		uint64 val = *((uint64*)bf);		
		Tm = TTm::GetTmFromMSecs(val);
	}
	uint64 GetFieldTmAsUInt64(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return *((uint64*)bf);
	}
	uint64 GetFieldUInt64(const TMem& Source, const int& FieldId) const {
		uchar* bf = GetLocationFixed(Source, FieldId);
		return *((uint64*)bf);
	}	
	void GetFieldStrV(const TMem& Source, const int& FieldId, TStrV& StrV) const {
		TThinMIn MIn(Source);
		MIn.MoveTo(GetOffsetVar(Source, FieldId));
		StrV.Load(MIn);
	} 
	void GetFieldIntV(const TMem& Source, const int& FieldId, TIntV& IntV) const {
		TThinMIn MIn(Source);
		MIn.MoveTo(GetOffsetVar(Source, FieldId));
		IntV.Load(MIn);
	} 
	void GetFieldFltV(const TMem& Source, const int& FieldId, TFltV& FltV) const {
		TThinMIn MIn(Source);
		MIn.MoveTo(GetOffsetVar(Source, FieldId));
		FltV.Load(MIn);
	} 

	// access to unit tests for this class
	static void UnitTests();

private:

	static void UnitTest_InitEmpty();
	static void UnitTest_InitSingleFixed();
	static void UnitTest_InitSingleFixedNullable();
	static void UnitTest_InitSingleFixedCodebook();
	static void UnitTest_InitSingleVariable();	
	static void UnitTest_InitPairFixed();
	static void UnitTest_InitPairVariable();
	static void UnitTest_InitPairMixed();
	static void UnitTest_InitComplex();
	static void UnitTest_InitTripleVariable();

	static void UnitTest_SerializeSingleFixed();	
	static void UnitTest_SerializeSingleFixedTm();	
	static void UnitTest_SerializeSingleFixedNull();	
	static void UnitTest_SerializeSingleFixedCodebook();	
	static void UnitTest_SerializeSingleVar();	
	static void UnitTest_SerializeSingleVarShort();
	static void UnitTest_SerializeSingleVarNull();	
	static void UnitTest_SerializePairFixed();
	static void UnitTest_SerializePairVar();
	static void UnitTest_SerializePairMixed();
	static void UnitTest_SerializePairMixedCodebook();
	static void UnitTest_SerializeTripleVariable();
	static void UnitTest_SerializeComplex();

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility class for parsing JSON store definitions etc.

class TGenericStoreUtils {
private:

	// class for internal static data
	class TMaps {
	public:
		TStrH FieldTypeMap;
		TStrHash<TStr> TimeWindowUnitMap;

		TMaps();  
	};

	static TMaps Maps;

public:
	static TGenericStoreSchema ParseStoreSchema(const PJsonVal& json);
	static TGenericSchema ParseSchema(const PJsonVal& json);
	static void ValidateSchema(const TWPt<TBase>& Base, TGenericSchema& Schema);
	static TFieldDesc ParseFieldDesc(const PJsonVal& json);
	static TGenericStoreFieldDesc ParseFieldDescEx(const PJsonVal& json);

	static TJoinDesc ParseJoinDesc(const PJsonVal& json, const TWPt<TGenericStore>& store, TGenericBase* base);
	static TGenericStoreJoinSchema ParseJoinDescInner(const PJsonVal& json);

	static TGenericKeyIndexDesc ParseKeyIndexDescInner(const PJsonVal& json, const TGenericStoreSchema& store);

	static TStr GetRecNameFromJson(const PJsonVal& json, const IFieldDescProvider* field_defs);

	static void UnitTests();
};

//////////////////////////////////////////////////
// generic-Store - JSON based
class TGenericStore: public TStore, public IFieldDescProvider {
private:
	struct TFieldJoinDat {
		TWPt<TStore> JoinStore;
		TInt InverseJoinId;
		TUInt64 JoinRecId;
		TInt JoinFq;
	};
	
protected:
	TStr StoreFNm;
	TFAccess FAccess;
    // shortcuts
    TWPt<TBase> Base;
	TWPt<TGenericBase> GenericBase;
    TWPt<TIndex> Index;
    

	TBool HasNameField;
	TStr NameField;
	TInt NameFieldId;
	TVec<TGenericKeyIndexDesc> KeyIndexes;
	THash<TInt, TGenericStoreFieldDesc> ExtraData;
	THash<TInt, TInt> InverseJoins;
	
private:
	// hash tables with names
	THash<TStr, TUInt64> NmH; 
	// store with records that goes to disk
	TWndBlockCache<TMem> JsonC;
	// memory-only storage
	TGenericMemoryStorage JsonM; 
	// serializator
	PGenericStoreSerializator SerializatorCache;
	// serializator
	PGenericStoreSerializator SerializatorMem;
	// store settings
	TGenericStoreSettings StoreSettings;

	void GetMem(const bool& UseMemory, const uint64& RecId, TMem& Rec) const;
	bool UseMemoryData(const int &FieldId) const;
	PGenericStoreSerializator GetAppropriateSerializator(const bool& MemoryData) const;
	PGenericStoreSerializator GetAppropriateSerializator(const int &FieldId) const;
	void ProcessChangedFields(const uint64& RecId, TVec<int>& changed_fields, PGenericStoreSerializator& ser, TMem& src, TMem& dest);
	inline void RemoveIdFromNmH(const uint64& RecId);

public:
	TGenericStore(const TWPt<TGenericBase>& _GenericBase, const uchar& StoreId, const TStr& StoreName, 
		const TWPt<TIndexVoc>& IndexVoc, const TStr& _StoreFNm, const int64& _MxCacheSize);
	TGenericStore(const TWPt<TGenericBase>& _GenericBase, const TStr& _StoreFNm,
		const TFAccess& _FAccess, const int64& _MxCacheSize);
	// need to override destructor, to clear cache
	~TGenericStore();

	// tells is given ID is a valid ID of existing record
	bool IsRecId(const uint64& RecId) const { return JsonC.IsValId(RecId); }

	// mappings between record ID and record name
	bool HasRecNm() const { return HasNameField.Val; }
	bool IsRecNm(const TStr& RecNm) const { return NmH.IsKey(RecNm); }
	TStr GetRecNm(const uint64& RecId) const;
	uint64 GetRecId(const TStr& RecNm) const;
	
	// number of all the records
	uint64 GetRecs() const { return JsonC.Len(); }
	// iterator over the store
	PStoreIter GetIter() const;
	// add new record
	uint64 AddRec(const PJsonVal& Json);
	// update existing record
	void UpdateRec(const uint64& RecId, const PJsonVal& Json);
    
    // add join
    void AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq);
    // delete join
    void DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq);

	// initialize from given store schema
	void InitFromSchema(const TGenericStoreSchema& store_schema);
	// generic store settings
	TGenericStoreSettings GetStoreSettings() const { return StoreSettings; }

	// implementation of TFieldDescProvider
	int GetFieldCount() const { return GetFields(); };
	const TFieldDesc& GetFieldDescription(int i) const { return GetFieldDesc(i); };	
	const TGenericStoreFieldDesc& GetGenericStoreFieldDesc(int i) const { return ExtraData.GetDat(i); }
	const TStr& GetNameField() const { return NameField; };

	// field retrieval
	virtual bool IsFieldNull(const uint64& RecId, const int& FieldId) const;
	virtual int GetFieldInt(const uint64& RecId, const int& FieldId) const;
	virtual TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
	virtual bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
	virtual double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
	virtual TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
	virtual uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;	
	virtual void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
	virtual void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
	virtual void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
	virtual void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
	virtual uint64 GetFieldTmAsUInt64(const uint64& RecId, const int& FieldId) const;

	void GarbageCollect();
	void AddInverseJoin(const int& join_id1, const int& join_id2) { InverseJoins.AddDat(join_id1, join_id2); }
};

////////////////////////////////////////////////////////////////////////////////
// Generic-Base 
// This class provides "JSON based" database layer, where stores can be defined 
// on-the-fly by user using JSON syntax.
class TGenericBase {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TGenericBase>;
public:
	// qminer base
	PBase Base;
	// hashtable of used vocabularies 
	TStrHash<TInt> VocH;
protected:
	// size of cache size per store
	TUInt64 StoreCacheSize;

	// for creating a new database
	TGenericBase(const TStr& FPath, const uint64& IndexCacheSize, 
		const uint64& _StoreCacheSize, const bool& InitP);
	// for loading existing index and stores for update or read-only
	TGenericBase(const TStr& FPath, const TFAccess& FAccess, const uint64& IndexCacheSize, 
		const uint64& _StoreCacheSize, const TStrUInt64H& StoreCacheSizes, const bool& InitP);	
	// saving stores
	void Save(const TStr& FPath);
	void UpdateRec(const TWPt<TGenericStore>& Store, const PJsonVal& RecVal);

	static void CopyAllFiles(TStr& Src, TStr& Dest);
	static void DeleteAllFiles(TStr& Src);
	static TStr GetArchiveDir(TStr& FPath) { return FPath + "archive/"; }

public:

	// for creating new index and stores
	static PGenericBase New(const TStr& FPath, const uint64& IndexCacheSize, 
			const uint64& StoreCacheSize, const bool& InitP = true) {

		return new TGenericBase(FPath, IndexCacheSize, StoreCacheSize, InitP); 
	}

	// for loading existing index and stores
	static PGenericBase Load(const TStr& FPath, const TFAccess& FAccess,
			const uint64& IndexCacheSize, const uint64& StoreCacheSize, 
			const TStrUInt64H& StoreCacheSizes = TStrUInt64H(), const bool& InitP = true) {

		return new TGenericBase(FPath, FAccess, IndexCacheSize, StoreCacheSize, StoreCacheSizes, InitP); 
	}

	// changes still in cache saved only in destructor
	~TGenericBase();

	void CreateSchema(const PJsonVal& StoreDefVal);
	uchar GetStoreId(const TStr& StoreName) { 
		return Base->GetStoreByStoreNm(StoreName)->GetStoreId(); }

	// utility method for typecasting stores to generic stores
	TWPt<TGenericStore> GetStoreByStoreNm(const TStr& StoreNm) { 
		return (TGenericStore*)Base->GetStoreByStoreNm(StoreNm)(); }
	TWPt<TGenericStore> GetStoreByStoreId(const uchar StoreId) {
		return (TGenericStore*)Base->GetStoreByStoreId(StoreId)(); }
	TWPt<TGenericStore> GetStoreByStoreIndex(const int StoreIndex) { 
		return (TGenericStore*)Base->GetStoreByStoreN(StoreIndex)(); }

	uint64 AddRec(const TStr& StoreNm, const PJsonVal& RecVal);
	uint64 AddRec(const int StoreId, const PJsonVal& RecVal);
	void UpdateRec(const TStr& StoreNm, const PJsonVal& RecVal);
	void UpdateRec(const int StoreId, const PJsonVal& RecVal);

	void GarbageCollect() { 
		for (int i=0; i<Base->GetStores(); i++) {
			GetStoreByStoreIndex(i)->GarbageCollect();
		}
	}

	static void ArchiveFiles(TStr FPath);
	static void RestoreArchivedFiles(TStr FPath);
};

}

#endif
