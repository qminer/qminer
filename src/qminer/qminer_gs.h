/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef QMINER_GS_H
#define QMINER_GS_H

#include "qminer_core.h"

namespace TQm {

namespace TStorage {
	
///////////////////////////////
/// Location to where field is serialized
typedef enum { 
	slMemory, ///< in-memory storage
	slDisk    ///< disk storage with most-recently-used memory cache
} TStoreLoc;
	
///////////////////////////////
/// Schema description of field.
/// Contains extra stuff needed for serialization not included in TFieldDesc.
class TFieldDescEx  {
public:
	/// Where will this field be stored- true means 
	TStoreLoc FieldStoreLoc;
	/// Should be stored in codebook?
	TBool CodebookP;
	/// Is small string?
	TBool SmallStringP;  
	/// Default value if value not specified
	PJsonVal DefaultVal;  
public:
	TFieldDescEx() {}
	TFieldDescEx(const TStoreLoc& _FieldStoreLoc, const bool& _CodebookP, 
		const bool& _SmallStringP, const PJsonVal& _DefaultVal = NULL):
			FieldStoreLoc(_FieldStoreLoc), CodebookP(_CodebookP),
			SmallStringP(_SmallStringP), DefaultVal(_DefaultVal) { }
};  

///////////////////////////////
/// Schema description of index key
class TIndexKeyEx {
public:
	/// Indexed field name
	TStr FieldName;
	/// Key name (sometimes different from field name)
	TStr KeyIndexName;
	/// Index type
	TIndexKeyType KeyType;
	/// How to sort the key (if at all)
	TIndexKeySortType SortType;
	/// Word vocabulary name (used by inverted index)
	TStr WordVocName;
	/// Tokenizer (used by inverted index)
	PTokenizer Tokenizer;
	
public:
	TIndexKeyEx() {}

	/// Is indexed by value
	bool IsValue() const { return (KeyType & oiktValue) > 0; }
	/// Is indexed as text (tokenized)
	bool IsText() const { return (KeyType & oiktText) > 0; }
	/// Is indexed as geo-location
	bool IsLocation() const { return (KeyType & oiktLocation) > 0; }
	// get index type
	TStr GetKeyType() const { return IsValue() ? "value" : IsText() ? "text" : "location"; }

	/// Key sortable as string
	bool IsByStr() const { return SortType == oikstByStr; }
	/// Key sortable by ID
	bool IsById() const { return SortType == oikstById; }
	/// Key sortable as numeric value
	bool IsByFlt() const { return SortType == oikstByFlt; }
	
	/// Checkes if we have defined tokenizer
	bool IsTokenizer() const { return !Tokenizer.Empty(); }
};

///////////////////////////////
// Schema description of join 
class TJoinDescEx {
public:
	/// Join name
	TStr JoinName;
	/// Join store name
	TStr JoinStoreName;
	/// Join type
	TStoreJoinType JoinType;
	/// Name of reverse join (empty if none)
	TStr InverseJoinName;
	/// Flag if index should use small storage
	TBool IsSmall;
public:
	TJoinDescEx(): JoinType(osjtUndef) { }
};

///////////////////////////////
/// Store windowing type
typedef enum { 
	swtNone = 0,   ///< No windowing on the store
	swtLength = 1, ///< Record-number based windowing
	swtTime = 2    ///< Time-based windowing
} TStoreWndType;

///////////////////////////////
/// Store window description
class TStoreWndDesc {
public:
	/// Prefix used for fields inserted by system
	static TStr SysInsertedAtFieldName;   
	
public:    
	/// Windowing type
	TStoreWndType WindowType;
	/// For time window this is period length in milliseconds, otherwise it is max length
	TUInt64 WindowSize;   
	/// User insert time
	TBool InsertP;        
	/// Name of the field that serves as time-window indicator
	TStr TimeFieldNm;   
	
public:
	TStoreWndDesc(): WindowType(swtNone) { }
	TStoreWndDesc(TSIn& SIn){ Load(SIn); }

	void Save(TSOut& SOut) const;
	void Load(TSIn& SIn);
};

///////////////////////////////
/// Store schema definition.
/// Contains parsed version of store definition, which can be used to
/// initialize TStoreImpl and TBaseImpl.
class TStoreSchema {
private:   
	// class for internal static data
	class TMaps {
	public:
		TStrH FieldTypeMap;
		TStrHash<uint64> TimeWindowUnitMap;

		TMaps();  
	};

	static TMaps Maps;
	
public:
	/// Store name
	TStr StoreName;
	/// Store type
	TStr StoreType;
	/// Store ID (not requireD)
	TUInt StoreId;
	/// True when specified store ID is valid
	TBool HasStoreIdP;
	/// Window settings
	TStoreWndDesc WndDesc;
	/// Field descriptions
	TStrHash<TFieldDesc> FieldH;
	/// Extended field descriptions
	TStrHash<TFieldDescEx> FieldExH;
	/// Index key descriptions
	TVec<TIndexKeyEx> IndexKeyExV;
	/// Join descriptions
	TVec<TJoinDescEx> JoinDescExV;
	/// Size of blocks for memory storage
	TInt BlockSizeMem;
	
private:
	/// Parse field description from JSon
	TFieldDesc ParseFieldDesc(const PJsonVal& FieldVal);
	/// Parse extended field description from JSon
	TFieldDescEx ParseFieldDescEx(const PJsonVal& FieldVal);
	/// Parse extended join description from JSon
	TJoinDescEx ParseJoinDescEx(const PJsonVal& JoinVal);
	/// Parse extended index key description from JSon
	TIndexKeyEx ParseIndexKeyEx(const PJsonVal& IndexKeyVal);
	
public:
	TStoreSchema() { }
	TStoreSchema(const PJsonVal& StoreVal);
	
	/// Parse JSon definition file and return vector of store schemas
	static void ParseSchema(const PJsonVal& SchemaVal, TVec<TStoreSchema>& SchemaV);
	/// Validate give vector of store schemas
	static void ValidateSchema(const TWPt<TBase>& Base, TVec<TStoreSchema>& SchemaV);
};
typedef TVec<TStoreSchema> TStoreSchemaV;

/////////////////////////////////////////////////
// Dirty flags for TInMemStorage

/// Flag for new unsaved entry
const uchar isdfNew = 1;
/// Flag for clean, already saved entry
const uchar isdfClean = 1 << 1;
/// Flag for dirty entry that needs to be saved
const uchar isdfDirty = 1 << 2;
/// Flag for entry that hasn't been loaded yet
const uchar isdfNotLoaded = 1 << 3;

///////////////////////////////
/// In-memory storage.
/// Wrapper around TVec of TMems.
class TInMemStorage {
private:
	/// Storage filename
	TStr FNm;
	/// Storage filename for Blob storage
	TStr BlobFNm;
	/// Access type with which the storage is opened
	TFAccess Access;
	/// Physical offset of the first non-deleted record
	/// If BlobkSize is 100 and 150 records have been deleted,
	/// this value would be 50.
	TUInt64 FirstValOffset;
	/// Logical offset of the first physical record
	/// If BlobkSize is 100 and 150 records have been deleted,
	/// this value would be 100.
	TUInt64 FirstValOffsetMem;
	/// Storage vector
	mutable TVec<TMem, int64> ValV;
	/// Blob-pointers - locations where TMem objects are stored inside Blob storage
	TVec<TBlobPt, int64> BlobPtV;
	/// "Dirty flags" - 0 - new and not saved yet, 1 - existing and clean, 2 - existing but dirty, 3 - existing but not loaded
	mutable TVec<uchar, int64> DirtyV;
	/// Blob storage
	PBlobBs BlobStorage;
	/// How many records are packed together into block;
	TInt BlockSize;

	/// Utility method for loading specific record
	inline void LoadRec(int64 RecN) const;

	/// Utility method for storing specific record
	int SaveRec(int RecN);

public:
	TInMemStorage(const TStr& _FNm, const int& _BlockSize = 1000);
	TInMemStorage(const TStr& _FNm, const TFAccess& _Access, const bool& _Lazy = false);
	~TInMemStorage();

	// asserts if we are allowed to change stuff
	void AssertReadOnly() const;
	bool IsReadOnly() const { return Access == faRdOnly; }

	bool IsValId(const uint64& ValId) const;
	void GetVal(const uint64& ValId, TMem& Val) const; 
	uint64 AddVal(const TMem& Val);
	void SetVal(const uint64& ValId, const TMem& Val);
	void DelVals(int Vals);

	uint64 Len() const;
	uint64 GetFirstValId() const;
	uint64 GetLastValId() const;

	int PartialFlush(int WndInMsec = 500);
	inline void LoadAll();

	TBlobBsStats GetBlobBsStats() { return BlobStorage->GetStats(); }

#ifdef XTEST
private:
	friend class XTest;
	PBlobBs GetBlobStorage() { return BlobStorage; }
#endif
};

//////////////////////////////////////////////////////////////////////////////
/// Serialization and de-serialization of records to TMem.
/// This class handles smart serialization of JSON with respect to field 
/// serialization definitions. It supports NULL flags. It packs fixed-width 
/// fields together. Variable-width fields are stored in two parts - first 
/// there's an index array so that for each field we store its offset inside 
/// the buffer. Then there is the real variable-length part where the content
/// is stored.
class TRecSerializator {
private:

	///////////////////////////////
	/// Field serialization parameters.
	/// This class contains data about field serialization
	class TFieldSerialDesc {
	public:
		/// Id of this field inside TStore
		TInt FieldId;
		/// Where will this field be stored
		TStoreLoc StoreLoc;
		/// Offset of byte that contains NULL bit indicator for this field
		TUCh NullMapByte;
		/// Mask to use on the NULL-bit indicator byte
		TUCh NullMapMask;
		/// Is this field stored in fixed or variable part
		TBool FixedPartP;
		/// Offset in fixed or variable-index part
		TInt Offset;
		/// Is this field actually a string that is encoded using codebook
		TBool CodebookP;
		/// Is this field a short string?
		TBool SmallStringP;
		/// Default value if value not specified
		PJsonVal DefaultVal;  

	public:    
		TFieldSerialDesc() {}
		TFieldSerialDesc(TSIn& SIn){ Load(SIn); }

		// methods required for (de)serialization of this class inside TVec into TSOut and TSIn
		void Save(TSOut& SOut) const;
		void Load(TSIn& SIn);
	};

	/// Utility class for delaying deletes of deleted TOASTs
	class TToastWatcher {
	private:
		/// Parent serializator that needs to call store to delete the TOASTS
		TRecSerializator* Parent;
	public:
		/// Simple constructor
		TToastWatcher(TRecSerializator* Src) { Parent = Src; }
		/// Destructor that calls parent
		~TToastWatcher();
	};

	/// Flag if field is not TOAST-ed
	static const char ToastNo;
	/// Flag if field is TOAST-ed
	static const char ToastYes;
private: 
	/// Only store fields with this storage flag
	TStoreLoc TargetStorage;
	/// Offset of fixed part
	TInt FixedPartOffset;
	/// Offset of var-index part
	TInt VarIndexPartOffset;
	/// Offset of var-content part
	TInt VarContentPartOffset;
	/// Definition of fields. 
	/// Order of definitions in this vector defines the order of serialization
	TVec<TFieldSerialDesc> FieldSerialDescV;
	/// Mapping from Field id (in TStore) to index inside FieldsF
	THash<TInt, TInt> FieldIdToSerialDescIdH;	
	/// Codebook for encoding strings
	TStrHash<TInt, TBigStrPool> CodebookH;
	/// Flag if TOAST should be used
	TBool UseToast;
	/// Max length of non-TOAST-ed record
	TInt MxToastLen;
	/// Store to be used for TOAST-ing
	TWPt<TStore> Store;
	/// TOAST objects to delete
	TVec<TPgBlobPt> ToastPtToDel;	

	/// Dump report used on failed asserts
	TStr GetErrorMsg(const TMem& RecMem, const TFieldSerialDesc& FieldSerialDesc) const;
	/// Dump report used on failed asserts
	TStr GetErrorMsg(const char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc) const;

	/// returns field serialization description
	const TFieldSerialDesc& GetFieldSerialDesc(const int& FieldId) const;
	/// finds location inside the buffer for fixed-width fields
	char* GetLocationFixed(const TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	int GetOffsetVar(const TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	char* GetLocationVar(const TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc) const;
	/// calculates length of buffer where given var-length field is stored
	int GetVarPartBfLen(const TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc);

	/// finds location inside the buffer for fixed-width fields
	char* GetLocationFixed(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	int GetOffsetVar(const char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	char* GetLocationVar(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc) const;
	/// calculates length of buffer where given var-length field is stored
	int GetVarPartBfLen(const char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc);

	/// finds location inside the buffer for fixed-width fields
	char* GetLocationFixed(TThinMIn min, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	int GetOffsetVar(TThinMIn min, const TFieldSerialDesc& FieldSerialDesc) const;
	/// finds location inside the buffer for variable-width fields
	char* GetLocationVar(TThinMIn min, const TFieldSerialDesc& FieldSerialDesc) const;
	/// calculates length of buffer where given var-length field is stored
	int GetVarPartBfLen(TThinMIn min, const TFieldSerialDesc& FieldSerialDesc);


	/// set content offset for specified variable field
	void SetLocationVar(TMem& RecMem, const TFieldSerialDesc& FieldSerialDesc, const int& VarOffset) const;
	/// sets or un-sets NULL flag for specified field
	void SetFieldNull(TMem& RecMem, const TFieldSerialDesc& FieldSerialDesc, const bool& NullP) const;
	/// set content offset for specified variable field
	void SetLocationVar(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const int& VarOffset) const;
	/// sets or un-sets NULL flag for specified field
	void SetFieldNull(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const bool& NullP) const;
	
	/// Fixed-length field setter
	void SetFieldInt(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const int& Int);
	/// Fixed-length field setter
	void SetFieldUInt64(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const uint64& UInt64);
	/// Fixed-length field setter
	void SetFieldStr(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const TStr& Str);
	/// Fixed-length field setter
	void SetFieldBool(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const bool& Bool);
	/// Fixed-length field setter
	void SetFieldFlt(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const double& Flt);
	/// Fixed-length field setter
	void SetFieldFltPr(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const TFltPr& FltPr);
	/// Fixed-length field setter
	void SetFieldTm(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const TTm& Tm);
	/// Fixed-length field setter
	void SetFieldTmMSecs(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc, const uint64& TmMSecs);
	/// Parse fixed-length type field JSon value and serialize it accordingly to it's type
	void SetFixedJsonVal(TMemBase& RecMem, const TFieldSerialDesc& FieldSerialDesc,
		const TFieldDesc& FieldDesc, const PJsonVal& JsonVal);

	/// Fixed-length field setter
	void SetFieldInt(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const int& Int);
	/// Fixed-length field setter
	void SetFieldUInt64(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const uint64& UInt64);
	/// Fixed-length field setter
	void SetFieldStr(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const TStr& Str);
	/// Fixed-length field setter
	void SetFieldBool(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const bool& Bool);
	/// Fixed-length field setter
	void SetFieldFlt(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const double& Flt);
	/// Fixed-length field setter
	void SetFieldFltPr(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const TFltPr& FltPr);
	/// Fixed-length field setter
	void SetFieldTm(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const TTm& Tm);
	/// Fixed-length field setter
	void SetFieldTmMSecs(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc, const uint64& TmMSecs);
	/// Parse fixed-length type field JSon value and serialize it accordingly to it's type
	void SetFixedJsonVal(char* Bf, const int& BfL, const TFieldSerialDesc& FieldSerialDesc,
		const TFieldDesc& FieldDesc, const PJsonVal& JsonVal);

	/// Variable-length field setter
	void SetFieldIntV(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const TIntV& IntV);
	/// Variable-length field setter
	void SetFieldStr(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const TStr& Str);
	/// Variable-length field setter
	void SetFieldStrV(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const TStrV& StrV);
	/// Variable-length field setter
	void SetFieldFltV(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const TFltV& FltV);
	/// Variable-length field setter
	void SetFieldNumSpV(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const TIntFltKdV& SpV);
	/// Variable-length field setter
	void SetFieldBowSpV(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, const PBowSpV& SpV);    
	/// parse variable-length field JSon value and serialize it accordingly to it's type
	void SetVarJsonVal(TMem& RecMem, TMOut& SOut, const TFieldSerialDesc& FieldSerialDesc, 
		const TFieldDesc& FieldDesc, const PJsonVal& JsonVal);
	/// copy variable-length field from InRecMem to FixedMem and SOut
	void CopyFieldVar(const TMemBase& InRecMem, TMem& FixedMem, TMOut& VarSOut, const TFieldSerialDesc& FieldSerialDesc);

	/// Extract fixed part from record
	void ExtractFixedMem(const TMemBase& InRecMem, TMem& FixedMem);
	/// Combine fixed and variable buffers into a record
	void Merge(const TMem& FixedMem, const TMOut& VarSOut, TMem& OutRecMem);

	/// Check this temporary buffer if it must be TOAST-ed
	void CheckToast(TMOut& SOut, const int& Offset);
	/// Check if given field value is currently TOAST-ed and delete it
	void CheckToastDel(const TMemBase& InRecMem, const TFieldSerialDesc& FieldSerialDesc);
public:
	TRecSerializator(const TWPt<TStore>& _Store) { Store = _Store; }
	/// Initialize object from store schema
	TRecSerializator(const TWPt<TStore>& Store, 
			const TStoreSchema& StoreSchema, const TStoreLoc& _TargetStorage);
	
	/// Load from input stream
	void Load(TSIn& SIn);
	/// Save to output stream
	void Save(TSOut& SOut);

	/// Serialize JSon object
	void Serialize(const PJsonVal& RecVal, TMem& RecMem, const TWPt<TStore>& Store);
	/// Update existing serialization with updated fields from JSon object
	void SerializeUpdate(const PJsonVal& RecVal, const TMemBase& InRecMem, TMem& OutRecMem, 
		const TWPt<TStore>& Store, TIntSet& ChangedFieldIdSet);
	/// Update existing serialization with updated fields from JSon object
	/// Allow only fixed-part fields
	void SerializeUpdateInPlace(const PJsonVal& RecVal,
		TThinMIn MIn, const TWPt<TStore>& Store, TIntSet& ChangedFieldIdSet);

	/// Check if field inside this serializator
	bool IsFieldId(const int& FieldId) const { return FieldIdToSerialDescIdH.IsKey(FieldId); }
	/// Check if field is in fixed part
	bool IsInFixedPart(const int& FieldId) const { return GetFieldSerialDesc(FieldId).FixedPartP; }

	/// Field getter
	bool IsFieldNull(const uint64& RecId, const int& FieldId) const;

	/// Field getter
	bool IsFieldNull(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	int GetFieldInt(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	void GetFieldIntV(TThinMIn& min, const int& FieldId, TIntV& IntV) const;
	/// Field getter
	uint64 GetFieldUInt64(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	TStr GetFieldStr(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	void GetFieldStrV(TThinMIn& min, const int& FieldId, TStrV& StrV) const;
	/// Field getter
	bool GetFieldBool(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	double GetFieldFlt(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	TFltPr GetFieldFltPr(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	void GetFieldFltV(TThinMIn& min, const int& FieldId, TFltV& FltV) const;
	/// Field getter
	void GetFieldTm(TThinMIn& min, const int& FieldId, TTm& Tm) const;
	/// Field getter
	uint64 GetFieldTmMSecs(TThinMIn& min, const int& FieldId) const;
	/// Field getter
	void GetFieldNumSpV(TThinMIn& min, const int& FieldId, TIntFltKdV& SpV) const;
	/// Field getter
	void GetFieldBowSpV(TThinMIn& min, const int& FieldId, PBowSpV& SpV) const;

	/// Field getter
	bool IsFieldNull(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	int GetFieldInt(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	void GetFieldIntV(const TMemBase& RecMem, const int& FieldId, TIntV& IntV) const;
	/// Field getter
	uint64 GetFieldUInt64(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	TStr GetFieldStr(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	void GetFieldStrV(const TMemBase& RecMem, const int& FieldId, TStrV& StrV) const;
	/// Field getter
	bool GetFieldBool(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	double GetFieldFlt(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	TFltPr GetFieldFltPr(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	void GetFieldFltV(const TMemBase& RecMem, const int& FieldId, TFltV& FltV) const;
	/// Field getter
	void GetFieldTm(const TMemBase& RecMem, const int& FieldId, TTm& Tm) const;
	/// Field getter
	uint64 GetFieldTmMSecs(const TMemBase& RecMem, const int& FieldId) const;
	/// Field getter
	void GetFieldNumSpV(const TMemBase& RecMem, const int& FieldId, TIntFltKdV& SpV) const;
	/// Field getter
	void GetFieldBowSpV(const TMemBase& RecMem, const int& FieldId, PBowSpV& SpV) const;

	/// Field setter
	void SetFieldNull(char* Bf, const int& BfL, const int& FieldId, const bool& NullP);
	/// Fixed-length field setter
	void SetFieldInt(char* Bf, const int& BfL, const int& FieldId, const int& Int);
	/// Fixed-length field setter
	void SetFieldUInt64(char* Bf, const int& BfL, const int& FieldId, const uint64& UInt64);
	/// Fixed-length field setter
	void SetFieldStr(char* Bf, const int& BfL, const int& FieldId, const TStr& Str);
	/// Fixed-length field setter
	void SetFieldBool(char* Bf, const int& BfL, const int& FieldId, const bool& Bool);
	/// Fixed-length field setter
	void SetFieldFlt(char* Bf, const int& BfL, const int& FieldId, const double& Flt);
	/// Fixed-length field setter
	void SetFieldFltPr(char* Bf, const int& BfL, const int& FieldId, const TFltPr& FltPr);
	/// Fixed-length field setter
	void SetFieldTm(char* Bf, const int& BfL, const int& FieldId, const TTm& Tm);
	/// Fixed-length field setter
	void SetFieldTmMSecs(char* Bf, const int& BfL, const int& FieldId, const uint64& TmMSecs);
	
	/// Field setter
	void SetFieldNull(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId);
	/// Field setter
	void SetFieldInt(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const int& Int);
	/// Field setter
	void SetFieldIntV(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TIntV& IntV);
	/// Field setter
	void SetFieldUInt64(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const uint64& UInt64);
	/// Field setter
	void SetFieldStr(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TStr& Str);
	/// Field setter
	void SetFieldStrV(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TStrV& StrV);
	/// Field setter
	void SetFieldBool(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const bool& Bool);
	/// Field setter
	void SetFieldFlt(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const double& Flt);
	/// Field setter
	void SetFieldFltPr(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TFltPr& FltPr);
	/// Field setter
	void SetFieldFltV(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TFltV& FltV);
	/// Field setter
	void SetFieldTm(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TTm& Tm);
	/// Field setter
	void SetFieldTmMSecs(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const uint64& TmMSecs);
	/// Field setter
	void SetFieldNumSpV(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const TIntFltKdV& SpV);
	/// Field setter
	void SetFieldBowSpV(const TMemBase& InRecMem, TMem& OutRecMem, const int& FieldId, const PBowSpV& SpV);
};

///////////////////////////////
/// Field indexer. 
/// Takes record and updates the QMiner index structures according to schema
class TRecIndexer {
private:
	/// Fast access to info for a particular index key
	class TFieldIndexKey {
	public:
		/// Indexed field id
		TInt FieldId;
		/// Indexed field name
		TStr FieldNm;
		/// Indexed field type
		TFieldType FieldType;
		TStr FieldTypeStr;
		/// Index key id
		TInt KeyId;
		/// Index type
		TIndexKeyType KeyType;
		/// Word vocabulary id (used by inverted index)
		TInt WordVocId;

	public:
		TFieldIndexKey() { }
		TFieldIndexKey(const int& _FieldId, const TStr& _FieldNm, const TFieldType& _FieldType, 
			const TStr& _FieldTypeStr, const int& _KeyId, const TIndexKeyType& _KeyType, 
			const int& _WordVocId): FieldId(_FieldId), FieldNm(_FieldNm), FieldType(_FieldType), 
				FieldTypeStr(_FieldTypeStr), KeyId(_KeyId), KeyType(_KeyType), WordVocId(_WordVocId) { }

		/// Is indexed by value
		bool IsValue() const { return (KeyType & oiktValue) > 0; }
		/// Is indexed as text (tokenized)
		bool IsText() const { return (KeyType & oiktText) > 0; }
		/// Is indexed as geo-location
		bool IsLocation() const { return (KeyType & oiktLocation) > 0; }
		// get index type
		TStr GetKeyType() const { return IsValue() ? "value" : IsText() ? "text" : "location"; }
	};    
	
private:
	/// Index shortcut
	TWPt<TIndex> Index;
	/// Index vocabulary shortcut
	TWPt<TIndexVoc> IndexVoc;
	// list of index keys set for particular store
	TVec<TFieldIndexKey> FieldIndexKeyV;
	// map from field id to key position in FieldIndexKeyV
	TIntH FieldIdToKeyN;
	
	/// Index a record using the given key
	void IndexKey(const TFieldIndexKey& Key, const TMemBase& RecMem, 
		const uint64& RecId, TRecSerializator& Serializator);
	/// Delete existing index of a record based on a given key
	void DeindexKey(const TFieldIndexKey& Key, const TMemBase& RecMem, 
		const uint64& RecId, TRecSerializator& Serializator);
	/// Update value of existing index of a record
	void UpdateKey(const TFieldIndexKey& Key, const TMemBase& OldRecMem, 
		const TMemBase& NewRecMem, const uint64& RecId, TRecSerializator& Serializator);
	/// Check what needs to be done to update index for a given key
	void ProcessKey(const TFieldIndexKey& Key, const TMemBase& OldRecMem, 
		const TMemBase& NewRecMem, const uint64& RecId, TRecSerializator& Serializator);
		
public:
	TRecIndexer() { }
	TRecIndexer(const TWPt<TIndex>& Index, const TWPt<TStore>& Store);
	
	/// Check if given field is used for indexing
	bool IsFieldIndexKey(const int& FieldId) const;
	/// Index new record
	void IndexRec(const TMemBase& RecMem, const uint64& RecId, TRecSerializator& Serializator);
	/// Deindex existing record
	void DeindexRec(const TMemBase& RecMem, const uint64& RecId, TRecSerializator& Serializator);
	/// Update index for existing record
	void UpdateRec(const TMemBase& OldRecMem, const TMemBase& NewRecMem,
		const uint64& RecId, const int& ChangedFieldId, TRecSerializator& Serializator);
	/// Update indexes for existing record
	void UpdateRec(const TMemBase& OldRecMem, const TMemBase& NewRecMem,
		const uint64& RecId, TIntSet& ChangedFieldIdSet, TRecSerializator& Serializator);
};

///////////////////////////////
/// Implementation of store which can be initialized from a schema.
class TStoreImpl: public TStore {
private:
	/// For temporarily storing inverse joins which need to be indexed after adding records
	struct TFieldJoinDat {
		TWPt<TStore> JoinStore;
		TInt InverseJoinId;
		TUInt64 JoinRecId;
		TInt JoinFq;
	};
	
private:
	/// Store filename
	TStr StoreFNm;
	/// Open mode
	TFAccess FAccess;
	
	/// Do we have a primary field which can act as record name
	TBool RecNmFieldP;
	/// Id of primary field (-1 if not defined)
	TInt PrimaryFieldId;
	/// Type of primary field
	TFieldType PrimaryFieldType;
	/// Hash map from TStr primary field to record ID
	THash<TStr, TUInt64> PrimaryStrIdH;
	/// Hash map from TInt primary field to record ID
	THash<TInt, TUInt64> PrimaryIntIdH;
	/// Hash map from TUInt64 primary field to record ID
	THash<TUInt64, TUInt64> PrimaryUInt64IdH;
	/// Hash map from TFlt primary field to record ID
	THash<TFlt, TUInt64> PrimaryFltIdH;
	/// Hash map from TTm primary field to record ID
	THash<TUInt64, TUInt64> PrimaryTmMSecsIdH;
	
	/// Flag if we are using cache store
	TBool DataCacheP;
	/// Store for parts of records that go to disk
	TWndBlockCache<TMem> DataCache;
	/// Flag if we are using in-memory store
	TBool DataMemP;
	/// Store for parts of records that should be in-memory
	TInMemStorage DataMem;
	/// Serializator to disk
	TRecSerializator *SerializatorCache;
	/// Serializator to memory
	TRecSerializator *SerializatorMem;
	/// Map from fields to storage location
	TVec<TStoreLoc> FieldLocV;
	
	// record indexer
	TRecIndexer RecIndexer;
	
	/// Time window settings
	TStoreWndDesc WndDesc;
	
	/// initialize field storage location map
	void InitFieldLocV();
	/// Get TMem serialization of record from specified storage
	void GetRecMem(const TStoreLoc& RecLoc, const uint64& RecId, TMem& Rec) const;
	/// Get TMem serialization of record from specified where field is stored
	void GetRecMem(const uint64& RecId, const int& FieldId, TMem& Rec) const;
	/// Set TMem serialization of record to a specified storage
	void PutRecMem(const TStoreLoc& RecLoc, const uint64& RecId, const TMem& Rec);
	/// Set TMem serialization of record to storage where field is stored
	void PutRecMem(const uint64& RecId, const int& FieldId, const TMem& Rec);
	/// True when field is stored on disk
	bool IsFieldDisk(const int &FieldId) const;
	/// True when field is stored in-memory
	bool IsFieldInMemory(const int &FieldId) const;
	/// Get serializator for given location
	TRecSerializator* GetSerializator(const TStoreLoc& StoreLoc);
	/// Get serializator for given location
	const TRecSerializator* GetSerializator(const TStoreLoc& StoreLoc) const;
	/// Get serializator for given field
	TRecSerializator* GetFieldSerializator(const int &FieldId);
	/// Get serializator for given field
	const TRecSerializator* GetFieldSerializator(const int &FieldId) const;
	/// Remove record from name-id map
	inline void DelRecNm(const uint64& RecId);    
    /// Do we have a primary field
    bool IsPrimaryField() const { return PrimaryFieldId != -1; }
    /// Set primary field map
    void SetPrimaryField(const uint64& RecId);
    /// Set primary field map for a given string value
    void SetPrimaryFieldStr(const uint64& RecId, const TStr& Str);
    /// Set primary field map for a given integer value
    void SetPrimaryFieldInt(const uint64& RecId, const int& Int);
    /// Set primary field map for a given uint64 value
    void SetPrimaryFieldUInt64(const uint64& RecId, const uint64& UInt64);
    /// Set primary field map for a given double value
    void SetPrimaryFieldFlt(const uint64& RecId, const double& Flt);
    /// Set primary field map for a given TTm value
    void SetPrimaryFieldMSecs(const uint64& RecId, const uint64& MSecs);
    /// Delete primary field map
    void DelPrimaryField(const uint64& RecId);
    /// Delete primary field map for a given string value
    void DelPrimaryFieldStr(const uint64& RecId, const TStr& Str);
    /// Delete primary field map for a given integer value
    void DelPrimaryFieldInt(const uint64& RecId, const int& Int);
    /// Delete primary field map for a given uint64 value
    void DelPrimaryFieldUInt64(const uint64& RecId, const uint64& UInt64);
    /// Delete primary field map for a given double value
    void DelPrimaryFieldFlt(const uint64& RecId, const double& Flt);
    /// Delete primary field map for a given TTm value
    void DelPrimaryFieldMSecs(const uint64& RecId, const uint64& MSecs);
    /// Transform Join name to it's corresponding field name
    TStr GetJoinFieldNm(const TStr& JoinNm) const { return JoinNm + "Id"; }
    

	/// Initialize from given store schema
	void InitFromSchema(const TStoreSchema& StoreSchema);    
	/// Initialize field location flags
	void InitDataFlags();

public:
	TStoreImpl(const TWPt<TBase>& _Base, const uint& StoreId, 
		const TStr& StoreName, const TStoreSchema& StoreSchema, 
		const TStr& _StoreFNm, const int64& _MxCacheSize, const int& BlockSize);
	TStoreImpl(const TWPt<TBase>& _Base, const TStr& _StoreFNm,
		const TFAccess& _FAccess, const int64& _MxCacheSize, const bool& _Lazy = false);
	// need to override destructor, to clear cache
	~TStoreImpl();

	bool IsRecId(const uint64& RecId) const;
	bool HasRecNm() const { return RecNmFieldP; }
	bool IsRecNm(const TStr& RecNm) const;
	TStr GetRecNm(const uint64& RecId) const;
	uint64 GetRecId(const TStr& RecNm) const;
	uint64 GetRecs() const;
	uint64 GetFirstRecId() const { 
		return (DataMemP ? DataMem.GetFirstValId(): DataCache.GetFirstValId()); 
	}
	uint64 GetLastRecId() const { 
		return (DataMemP ? DataMem.GetLastValId() : DataCache.GetLastValId());
	}

	PStoreIter GetIter() const;
	
	/// Gets the first record in the store (order defined by store implementation)
	uint64 FirstRecId() const;
	/// Gets the last record in the store (order defined by store implementation)
	uint64 LastRecId() const;
	/// Gets forward moving iterator (order defined by store implementation)
	PStoreIter ForwardIter() const { return GetIter(); }
	/// Gets backward moving iterator (order defined by store implementation)
	PStoreIter BackwardIter() const;
	
	/// Add new record
	uint64 AddRec(const PJsonVal& RecVal);
	/// Update existing record
	void UpdateRec(const uint64& RecId, const PJsonVal& RecVal);

	/// Purge records that fall out of store window (when it has one)
	void GarbageCollect();
	/// Deletes all records
	void DeleteAllRecs();
	/// Delete the first DelRecs records (the records that were inserted first)
	void DeleteFirstNRecs(int Recs);
	/// Delete specific record
	void DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK = true);

	/// Check if the value of given field for a given record is NULL
	bool IsFieldNull(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	int GetFieldInt(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
	/// Get field value using field id (default implementation throws exception)
	uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
	/// Get field value using field id (default implementation throws exception)
	bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
	/// Get field value using field id (default implementation throws exception)
	uint64 GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const;
	/// Get field value using field id (default implementation throws exception)
	void GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const;
	
	/// Set the value of given field to NULL
	void SetFieldNull(const uint64& RecId, const int& FieldId);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV);
	/// Set field value using field id (default implementation throws exception)
	void SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV);

	/// Helper function for returning JSon definition of store
	PJsonVal GetStoreJson(const TWPt<TBase>& Base) const;


	/// Save part of the data, given time-window
	int PartialFlush(int WndInMsec = 500);
	/// Retrieve performance statistics for this store
	PJsonVal GetStats();
};
} // TStorage name space

}

#endif
