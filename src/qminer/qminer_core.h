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

#ifndef QMINER_CORE_H
#define QMINER_CORE_H

#include <base.h>
#include <mine.h>
#include <funrouter.h>

namespace TQm {

///////////////////////////////
// Forward Declarations
class TBase; typedef TPt<TBase> PBase;
class TStore; typedef TPt<TStore> PStore;
class TRec;
class TRecSet; typedef TPt<TRecSet> PRecSet;
class TIndexVoc; typedef TPt<TIndexVoc> PIndexVoc;
class TIndex; typedef TPt<TIndex> PIndex;
class TOp; typedef TPt<TOp> POp;
class TAggr; typedef TPt<TAggr> PAggr;
class TStreamAggr; typedef TPt<TStreamAggr> PStreamAggr;
class TStreamAggrBase; typedef TPt<TStreamAggrBase> PStreamAggrBase;
class TFtrExt; typedef TPt<TFtrExt> PFtrExt;

///////////////////////////////
/// QMiner Environment.
class TEnv {
private:
	static bool InitP;
public:
	/// QMiner version
	static TIntTr Version;
	/// Get current version as string
	static TStr GetVersion() { return TStr::Fmt("%d.%d.%d", Version.Val1.Val, Version.Val2.Val, Version.Val3.Val); }
	
    /// Maximal number of stores allowed, also sets the upper limit to valid store IDs
    static uint GetMxStores() { return 0x4000; } // == 16384
    
    /// True when QMiner is running in sandboxed mode
    /// TODO: make it configurable
    static bool IsSandbox() { return false; }
    
	/// Path to QMiner
	static TStr QMinerFPath;
	/// Path to 
	static TStr RootFPath;
	/// Default QMiner notification facility
	static PNotify Error;
	static PNotify Logger;	
    static PNotify Debug;
    /// Verbosity level for debugging.
    /// 0 => Silent, 1 => Info, 2=> Debug
    static TInt Verbosity;
    
    /// Return code when exiting QMiner
    static TInt ReturnCode;
	
	/// Setups QMiner static functions.
	static void Init();
	/// Checks if initialization done
	static bool IsInit() { return InitP; }

	/// Initialize logger.
	/// @param FPath		Specify logger output (standard output `std'; No output `null')
	/// @param TimestampP	Show timestamp with each status
	static void InitLogger(const int& _Verbosity = 1, const TStr& FPath = TStr(), const bool& TimestampP = false);
    
    /// Set return code
    static void SetReturnCode(const int& _ReturnCode) { ReturnCode = _ReturnCode; }
};

void ErrorLog(const TStr& MsgStr);
void InfoLog(const TStr& MsgStr);
void DebugLog(const TStr& MsgStr);

///////////////////////////////
/// QMiner Valid Name Enforcer.
class TValidNm {
private:
	/// Valid non-alpha-numeric first character of store/field/join/key name
	static TChA ValidFirstCh;
	/// Valid non-alpha-numeric non-first character of store/field/join/key name
	static TChA ValidCh;
public:
	/// Validate if given string is a valid store/field/join/key name.
	/// Valid names begin with alphabetical or ValidFirstCh and continues with
	/// any alphanumerical or ValidCh character (e.g. '_ThisIsValid_123_Name').
	static void AssertValidNm(const TStr& NmStr);
};

///////////////////////////////
/// QMiner Exception
class TQmExcept : public TExcept {
private:
	TQmExcept(const TStr& MsgStr, const TStr& LocStr): TExcept(MsgStr, LocStr) { }

	// private to catch old calls to this function
	static PExcept Throw(const TStr& MsgStr, const TStr& LocStr = TStr()) { return NULL; }
public:
	/// Create new QMiner exception
	static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr()) { 
		return PExcept(new TQmExcept(MsgStr, LocStr));
	}
};

#define QmAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : throw TQm::TQmExcept::New(TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

#define QmAssertR(Cond, MsgStr) \
  ((Cond) ? static_cast<void>(0) : throw TQm::TQmExcept::New(MsgStr, TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

///////////////////////////////
/// Join Types
typedef enum { 
	osjtUndef, 
	osjtIndex, ///< Index join
	osjtField  ///< Field join
} TStoreJoinType;

///////////////////////////////
/// Join Description
class TJoinDesc {
private:
	/// Join ID, unique within a store
    TInt JoinId;
	/// Join name, unique within a store
	TStr JoinNm;
	/// Destination store of the join
	TUInt JoinStoreId;
	/// Join type
	TStoreJoinType JoinType;
	/// Corresponding KeyID, used for index joins
	TInt JoinKeyId;
	/// Corresponding FieldId, used for field joins
	TInt JoinRecFieldId;
    /// Corresponding Frequency, used for field joins
    TInt JoinFqFieldId;
    /// Inverse join ID (-1 means not defined)
    TInt InverseJoinId;

public:
	/// Create an empty join description
	TJoinDesc(): JoinId(-1), JoinStoreId(TUInt::Mx), JoinType(osjtUndef), InverseJoinId(-1) { } 
	/// Create an index based join (1-N or N-M)
	TJoinDesc(const TStr& _JoinNm, const uint& _JoinStoreId,
		const uint& StoreId, const TWPt<TIndexVoc>& IndexVoc);
	/// Create a field based join (1-1)
	TJoinDesc(const TStr& _JoinNm, const uint& _JoinStoreId, const int& _JoinRecFieldId,
        const int& _JoinFqFieldId): JoinId(-1), JoinNm(_JoinNm), JoinStoreId(_JoinStoreId), 
        JoinType(osjtField), JoinKeyId(-1), JoinRecFieldId(_JoinRecFieldId), 
        JoinFqFieldId(_JoinFqFieldId), InverseJoinId(-1) { TValidNm::AssertValidNm(JoinNm); }

	TJoinDesc(TSIn& SIn);
	void Save(TSOut& SOut) const;
	
	void PutJoinId(const int& _JoinId) { JoinId = _JoinId; }
    int GetJoinId() const { return JoinId; };
	const TStr& GetJoinNm() const { return JoinNm; }
	uint GetJoinStoreId() const { return JoinStoreId; }
	TWPt<TStore> GetJoinStore(const TWPt<TBase>& Base) const;
	bool IsIndexJoin() const { return JoinType == osjtIndex; }
	int GetJoinKeyId() const { return JoinKeyId; }
	bool IsFieldJoin() const { return JoinType == osjtField; }
	int GetJoinRecFieldId() const { return JoinRecFieldId; }
	int GetJoinFqFieldId() const { return JoinFqFieldId; }
    void PutInverseJoinId(const int& _InverseJoinId) { InverseJoinId = _InverseJoinId; }
    bool IsInverseJoinId() const { return InverseJoinId != -1; }
    int GetInverseJoinId() const { return InverseJoinId; }
};
typedef TVec<TJoinDesc> TJoinDescV;

///////////////////////////////////////////////
/// Join Sequence
class TJoinSeq {
private:
	/// Initial store ID
	TUInt StartStoreId;
	/// Sequence of (JoinID, SampleSize) pairs, defining the join sequence.
	/// Sample size of -1 corresponds to everything.
	TIntPrV JoinIdV;

public:
	TJoinSeq(): StartStoreId(TUInt::Mx) { }
	/// Create empty join sequence (valid, but performs no joins)
	TJoinSeq(const TWPt<TStore>& StartStore);
	/// Create empty join sequence (valid, but performs no joins)
	TJoinSeq(const uint& _StartStoreId): StartStoreId(_StartStoreId) { }
	/// Create single step join sequence	
	TJoinSeq(const uint& _StartStoreId, const int& JoinId, const int& Sample = -1);
	/// Create a sequence from given (join,sample) pair list.
	TJoinSeq(const uint& _StartStoreId, const TIntPrV& _JoinIdV);
	/// Extracts join sequence from JSon
	TJoinSeq(const TWPt<TBase>& Base, const uint& _StartStoreId, const PJsonVal& JoinSeqVal);
	
	TJoinSeq(TSIn& SIn): StartStoreId(SIn), JoinIdV(SIn) { }
	void Load(TSIn& SIn) { StartStoreId.Load(SIn); JoinIdV.Load(SIn); }
	void Save(TSOut& SOut) const { StartStoreId.Save(SOut); JoinIdV.Save(SOut); }

	/// Is the join sequence valid
	bool Empty() const { return (StartStoreId.Val == TUInt::Mx); }
	/// Does sequnence have any joins
	bool IsJoin() const { return !JoinIdV.Empty(); }

	/// Get join start store
	TWPt<TStore> GetStartStore(const TWPt<TBase>& Base) const;
	/// Get join start store ID
	uint GetStartStoreId() const { return StartStoreId.Val; }
	/// Get join end store
	TWPt<TStore> GetEndStore(const TWPt<TBase>& Base) const;
	/// Get join end store ID
	uint GetEndStoreId(const TWPt<TBase>& Base) const;
	/// Get join sequence
	const TIntPrV& GetJoinIdV() const { return JoinIdV; }

	/// Readable string representation of join sequence
	TStr GetJoinPathStr(const TWPt<TBase>& Base, const TStr& SepStr = ".") const;

	int GetPrimHashCd() const {return TPairHashImpl::GetHashCd(StartStoreId.GetPrimHashCd(), JoinIdV.GetPrimHashCd()); }
	int GetSecHashCd() const {return TPairHashImpl::GetHashCd(StartStoreId.GetSecHashCd(), JoinIdV.GetSecHashCd()); }
	
};
typedef TVec<TJoinSeq> TJoinSeqV;

///////////////////////////////
/// Field Type
typedef enum { 
    oftUndef    = -1,///< Undefined
	oftInt		= 0, ///< Integer
	oftIntV		= 9, ///< Vector of integers
	oftUInt64	= 8, ///< Unsigned 64bit integer
	oftStr		= 1, ///< String
	oftStrV		= 2, ///< Vector of strings
	oftBool		= 4, ///< Boolean
	oftFlt		= 5, ///< Double precision number
	oftFltPr	= 6, ///< Pair of double precision numbers, useful for storing geographic coordinates
	oftFltV		= 10,///< Vector of double precision numbers
	oftTm		= 7, ///< Date and time
	oftNumSpV	= 11,///< Sparse vector -- vector of (integer,double) pairs 
	oftBowSpV	= 12 ///< Bag-of-words sparse vector
} TFieldType;

///////////////////////////////
/// Field Description
class TFieldDesc {
private:
	typedef enum {
		ofdfNull = (1 << 0),
		ofdfInternal = (1 << 1),
		ofdfPrimary = (1 << 2)
	} TFieldDescFlags;

private:
	/// Field ID
    TInt FieldId;
	/// Field name
    TStr FieldNm;
	/// Field type
	TFieldType FieldType;
	/// Property flags
	TInt Flags;
	/// Linked index keys (can be used to search over this field)
	TIntV KeyIdV;

public:
	TFieldDesc(): FieldId(-1), FieldNm(), FieldType(oftUndef) { }
	/// Create new field description
	/// @param _FieldNm Field name
	/// @param _FieldType Field type
	/// @param NullP Can field be empty for some records
	/// @param InternalP Filed was created is and being used internally by QMiner (E.g. for field joins)
    TFieldDesc(const TStr& _FieldNm, TFieldType _FieldType, const bool& Primary,
        const bool& NullP, const bool& InternalP);
	
	TFieldDesc(TSIn& SIn);
	void Save(TSOut& SOut) const;

	// basic properties
	void PutFieldId(const int& _FieldId) { FieldId = _FieldId; }
    int GetFieldId() const { return FieldId; };
    const TStr& GetFieldNm() const { return FieldNm; }

	// field data-type
    TFieldType GetFieldType() const { return FieldType; }
	TStr GetFieldTypeStr() const;
    bool IsInt() const { return FieldType == oftInt; }
    bool IsIntV() const { return FieldType == oftIntV; }
    bool IsUInt64() const { return FieldType == oftUInt64; }
    bool IsStr() const { return FieldType == oftStr; }
    bool IsStrV() const { return FieldType == oftStrV; }
    bool IsBool() const { return FieldType == oftBool; }
    bool IsFlt() const { return FieldType == oftFlt; }
    bool IsFltPr() const { return FieldType == oftFltPr; }
    bool IsFltV() const { return FieldType == oftFltV; }
    bool IsTm() const { return FieldType == oftTm; }
    bool IsNumSpV() const { return FieldType == oftNumSpV; }
    bool IsBowSpV() const { return FieldType == oftBowSpV; }

	// flags
	bool IsNullable() const { return ((Flags & ofdfNull) != 0); }
	bool IsInternal() const { return ((Flags & ofdfInternal) != 0); }
    bool IsPrimary() const { return ((Flags & ofdfPrimary) != 0); }

	/// Link index key with ID `KeyId` to this field
	void AddKey(const int& KeyId) { KeyIdV.AddUnique(KeyId); }
	/// True if there are any index keys associated with the field
	bool IsKeys() const { return !KeyIdV.Empty(); }
	int GetKeys() const { return KeyIdV.Len(); }
	int GetKeyId(const int& KeyIdN) const { return KeyIdV[KeyIdN]; }
	int GetKeyId() const { QmAssert(IsKeys()); return KeyIdV[0]; }
};
typedef TVec<TFieldDesc> TFieldDescV;

///////////////////////////////
/// Store iterator
class TStoreIter {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TStoreIter>;
public:
	virtual ~TStoreIter() { }
	/// Move to next record, return false when there is none left
	virtual bool Next() = 0;
	/// Get ID of the current record
	virtual uint64 GetRecId() const = 0;
};
typedef TPt<TStoreIter> PStoreIter;

///////////////////////////////
/// Store Vector Iterator.
/// Useful for stores using TVec to store records
class TStoreIterVec : public TStoreIter {
private:
	/// True before first call to Next()
	bool FirstP;
    /// True when we reach the end
    bool LastP;
    /// Direction (increment or decrement)
    bool AscP;
	/// Current Record ID
	uint64 RecId;
	/// End
	uint64 EndId;

    /// Empty vector
	TStoreIterVec();
    /// Vector has elements from MinId till MaxId
	TStoreIterVec(const uint64& _StartId, const uint64& _EndId, const bool& _AscP);
public:
	/// Create new iterator for empty vector
	static PStoreIter New() { return new TStoreIterVec; }
	/// Create new iterator for vector, which starts with RecId = MinId
	static PStoreIter New(const uint64& StartId, const uint64& EndId, const bool& AscP);

	bool Next();
	uint64 GetRecId() const { QmAssert(!FirstP); return RecId; }
};

///////////////////////////////
/// Store Hash Iterator.
/// Useful for stores using THash or TStrHash to store records
/// Example: TStrH TestH; PStoreIter TestIter = TStoreIterHash<TStrH>::New(TestH);
template <class THash>
class TStoreIterHash : public TStoreIter {
private:
	/// Reference to hash table with KeyIds corresponding to Record IDs
	const THash& Hash;
	/// Current key ID (== record ID)
	int KeyId;
	
	TStoreIterHash(const THash& _Hash): Hash(_Hash), KeyId(Hash.FFirstKeyId()) { }
public:
	// Create new iterator from a given hash table
	static PStoreIter New(const THash& Hash) { return new TStoreIterHash<THash>(Hash); }

	bool Next() { return Hash.FNextKeyId(KeyId); }
	uint64 GetRecId() const { return (uint64)KeyId; }
};

///////////////////////////////
/// Store Trigger.
/// Interface for defining triggers called when records are added, deleted or updated.
/// Each trigger gets unique internal ID when created (GUID).
class TStoreTrigger {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TStoreTrigger>;
	/// Each trigger has a unique internal ID
	TStr Guid;
public:
	TStoreTrigger(): Guid(TGuid::GenGuid()) { }
	virtual ~TStoreTrigger() { }

	/// Called when trigger added to the store
    virtual void Init(const TWPt<TStore>& Store) { }
	/// Called after record added to the store
    virtual void OnAdd(const TRec& Rec) = 0;
	/// Called after record updated in the store
    virtual void OnUpdate(const TRec& Rec) = 0;
	/// Called before record from the store
    virtual void OnDelete(const TRec& Rec) = 0;

	/// Unique ID of the trigger
	const TStr& GetGuid() const { return Guid; }
};
typedef TPt<TStoreTrigger> PStoreTrigger;
typedef TVec<PStoreTrigger> TStoreTriggerV;

///////////////////////////////
/// Store. 
/// Main interface to accessing records and their fields.
/// Keeps meta-data descriptions of fields and joins associated with the store.
/// Contains callbacks for triggers (PStoreTrigger) which were added to the store.
class TStore {
private:
	// smart-pointer
	TCRef CRef;
	friend class TPt<TStore>;
    
    /// Shortcut to base
    TWPt<TBase> Base;
    /// Shortcut to index
    TWPt<TIndex> Index;
	
	/// Store unique ID
    TUInt StoreId;
	/// Store unique Name
    TStr StoreNm;
	/// Join meta-data description
    TJoinDescV JoinDescV;
	/// Map from join name to join ID
    TStrH JoinNmToIdH;
	/// Map from join name to index key ID (only for index join)
    TStrH JoinNmToKeyIdH;
	/// Fields meta-data description
    TFieldDescV FieldDescV;
	/// Map from field name to ID
    TStrH FieldNmToIdH;
    /// List of active triggers
    TStoreTriggerV TriggerV;

	/// Load store from stream (to be called only by base class!)
    void LoadStore(TSIn& SIn);
protected:
	/// Create new store with given ID and name
	TStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm);
	/// Load store from input stream
	TStore(const TWPt<TBase>& _Base, TSIn& SIn);
	/// Load store from file
	TStore(const TWPt<TBase>& _Base, const TStr& FNm);
    /// Store to disk
    virtual ~TStore() { }
    
	/// Save store to output stream
	void SaveStore(TSOut& SOut) const;
public:    
    /// Access to base
    const TWPt<TBase>& GetBase() const { return Base; }
protected:
	/// Access to index
    const TWPt<TIndex>& GetIndex() const { return Index; }
    
	/// Register new field to the store
    int AddFieldDesc(const TFieldDesc& FieldDesc);
	/// Default error when accessing wrong field-type combination
    PExcept FieldError(const int& FieldId, const TStr& TypeStr) const;

    /// Should be called after record RecId added; executes OnAdd event in all register triggers
    void OnAdd(const uint64& RecId);
    /// Should be called after record RecId updated; executes OnUpdate event in all register triggers
    void OnUpdate(const uint64& RecId);
    /// Should be called before record RecId deleted; executes OnDelete event in all register triggers
    void OnDelete(const uint64& RecId);

	/// Helper function for handling string and vector pools
    void StrVToIntV(const TStrV& StrV, TStrHash<TInt, TBigStrPool>& WordH, TIntV& IntV);
	/// Helper function for handling string and vector pools
    void IntVToStrV(const TIntV& IntV, const TStrHash<TInt, TBigStrPool>& WordH, TStrV& StrV) const;
	/// Helper function for handling string and vector pools
	void IntVToStrV(const TIntV& IntV, TStrV& StrV) const;
    
    /// Processing nested join records in JSon
    void AddJoinRec(const uint64& RecId, const PJsonVal& RecVal);

public:
    /// Get store ID
    uint GetStoreId() const { return StoreId; }
	/// Get store name
    TStr GetStoreNm() const { return StoreNm; }

    /// Load store ID from the stream and retrieve store
    static TWPt<TStore> LoadById(const TWPt<TBase>& Base, TSIn& SIn);
    /// Save store ID to the output stream
    void SaveId(TSOut& SOut) const { StoreId.Save(SOut); }
    
	/// Register new join
    int AddJoinDesc(const TJoinDesc& JoinDesc);
	/// Get number of registered joins
	int GetJoins() const { return JoinDescV.Len(); }
	/// Checks if join with given ID exists
	bool IsJoinId(const int& JoinId) const { return (JoinId >=0) && (JoinId < JoinDescV.Len()); }
	/// Checks if join with given name exists
	bool IsJoinNm(const TStr& JoinNm) const { return JoinNmToIdH.IsKey(JoinNm); }
	/// Get the name of join with the given ID
	const TStr& GetJoinNm(const int JoinId) const { return JoinDescV[JoinId].GetJoinNm(); }
	/// Get the ID of join with the given name
	int GetJoinId(const TStr& JoinNm) const { return JoinNmToIdH.GetDat(JoinNm); }
	/// Get index KeyId for a join with the given name
	int GetJoinKeyId(const TStr& JoinNm) const { return JoinNmToKeyIdH.GetDat(JoinNm); }
	/// Get index KeyId for a join with the given ID
	int GetJoinKeyId(const int& JoinId) const { return JoinDescV[JoinId].GetJoinKeyId(); }
	/// Get full join description for the join with the given ID
	const TJoinDesc& GetJoinDesc(const int& JoinId) const { return JoinDescV[JoinId]; }
    /// Register inverse join
    void PutInverseJoinId(const int& JoinId, const int& InverseJoinId);

    /// Get number of registered fields
    int GetFields() const { return FieldDescV.Len(); }
	/// Checks if field with the given ID exists
	bool IsFieldId(const int& FieldId) const { return (FieldId >=0) && (FieldId < FieldDescV.Len()); }
	/// Get name of a field with the given ID
    const TStr& GetFieldNm(const int& FieldId) const { return FieldDescV[FieldId].GetFieldNm(); }
	/// Check if field with the given name exists
    bool IsFieldNm(const TStr& FieldNm) const { return FieldNmToIdH.IsKey(FieldNm); }
	/// Get ID of a field with the given name
    int GetFieldId(const TStr& FieldNm) const { return FieldNmToIdH.GetDat(FieldNm); }
	/// Get full field description for the field with the given ID
    const TFieldDesc& GetFieldDesc(const int& FieldId) const { return FieldDescV[FieldId]; }
	/// Get vector of all existing field IDs
	TIntV GetFieldIdV(const TFieldType& Type);
	/// Link field and index key
	void AddFieldKey(const int& FieldId, const int& KeyId) { FieldDescV[FieldId].AddKey(KeyId); }
	/// Get linked index key for a given field
	int GetFieldKeyId(const int& FieldId) const { return FieldDescV[FieldId].GetKeyId(); }
    
    /// Register new trigger to the store
    void AddTrigger(const PStoreTrigger& Trigger);
	/// Unregister given trigger
	void DelTrigger(const PStoreTrigger& Trigger);

    /// True when records have names (default is false)
	virtual bool HasRecNm() const { return false; }
	/// Check if record with given ID exists
    virtual bool IsRecId(const uint64& RecId) const = 0;
	/// check if record with given name exists
	virtual bool IsRecNm(const TStr& RecNm) const = 0;
	/// Get record name for a given id
	virtual TStr GetRecNm(const uint64& RecId) const = 0;
	/// Get record id for a given name
    virtual uint64 GetRecId(const TStr& RecNm) const = 0;
	/// Get record with a given ID
	TRec GetRec(const uint64& RecId);
	/// Get record with a given name
	TRec GetRec(const TStr& RecNm);
	/// Get number of records in the store
	virtual uint64 GetRecs() const = 0; 
	/// Get iterator to go over all records in the store
	virtual PStoreIter GetIter() const = 0;
	/// Get record set with all the records in the store
	virtual PRecSet GetAllRecs();
	/// Get record set with random subset of records
	/// @param SampleSize	Number of records to be sampled out
	virtual PRecSet GetRndRecs(const uint64& SampleSize);
	/// Checks if no records in the store
	bool Empty() const { return (GetRecs() == uint64(0)); }

    /// Gets the first record in the store (order defined by store implementation)
    virtual uint64 FirstRecId() const { throw TQmExcept::New("Not implemented"); }
    /// Gets the last record in the store (order defined by store implementation)
    virtual uint64 LastRecId() const { throw TQmExcept::New("Not implemented"); };
    /// Gets forward moving iterator (order defined by store implementation)
    virtual PStoreIter ForwardIter() const { throw TQmExcept::New("Not implemented"); };
    /// Gets backward moving iterator (order defined by store implementation)
    virtual PStoreIter BackwardIter() const { throw TQmExcept::New("Not implemented"); };
    
	/// Add new record provided as JSon
	virtual uint64 AddRec(const PJsonVal& RecVal) = 0;
	/// Update existing record with updates in provided JSon
	virtual void UpdateRec(const uint64& RecId, const PJsonVal& RecVal) = 0;
    
    /// Add join
    void AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq);
    /// Delete join
    void DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq);
    
    /// Signal to purge any old stuff, e.g. records that fall out of time window when store has one
	virtual void GarbageCollect() { }
	/// Delete the first DelRecs records (the records that were inserted first)
	virtual void DeleteFirstNRecs(int DelRecs) { };
    
    /// Check if the value of given field for a given record is NULL
	virtual bool IsFieldNull(const uint64& RecId, const int& FieldId) const { return false; }
    /// Get field value using field id (default implementation throws exception)
    virtual int GetFieldInt(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
    virtual void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
    /// Get field value using field id (default implementation throws exception)
    virtual uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
	virtual TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
	virtual void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
    /// Get field value using field id (default implementation throws exception)
	virtual bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
    virtual double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
    virtual TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
    virtual void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
    /// Get field value using field id (default implementation throws exception)
    virtual void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
    /// Get field value using field id (default implementation throws exception)
    virtual uint64 GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id (default implementation throws exception)
    virtual void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const;
    /// Get field value using field id (default implementation throws exception)
    virtual void GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const;

	/// Check if the value of given field for a given record is NULL
	bool IsFieldNmNull(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    int GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    void GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const;
    /// Get field value using field name (default implementation throws exception)
    uint64 GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
	TStr GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
	void GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const;
    /// Get field value using field name (default implementation throws exception)
	bool GetFieldNmBool(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    double GetFieldNmFlt(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    TFltPr GetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    void GetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, TFltV& FltV) const;
    /// Get field value using field name (default implementation throws exception)
    void GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const;
    /// Get field value using field name (default implementation throws exception)
    uint64 GetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name (default implementation throws exception)
    void GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const;
    /// Get field value using field name (default implementation throws exception)
    void GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const;
 
	/// Set the value of given field to NULL
	virtual void SetFieldNull(const uint64& RecId, const int& FieldId);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64);
    /// Set field value using field id (default implementation throws exception)
	virtual void SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str);
    /// Set field value using field id (default implementation throws exception)
	virtual void SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV);
    /// Set field value using field id (default implementation throws exception)
	virtual void SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV);
    /// Set field value using field id (default implementation throws exception)
    virtual void SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV);

	/// Set the value of given field to NULL
	void SetFieldNmNull(const uint64& RecId, const TStr& FieldNm);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmInt(const uint64& RecId, const TStr& FieldNm, const int& Int);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, const TIntV& IntV);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm, const uint64& UInt64);
    /// Set field value using field name (default implementation throws exception)
	void SetFieldNmStr(const uint64& RecId, const TStr& FieldNm, const TStr& Str);
    /// Set field value using field name (default implementation throws exception)
	void SetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, const TStrV& StrV);
    /// Set field value using field name (default implementation throws exception)
	void SetFieldNmBool(const uint64& RecId, const TStr& FieldNm, const bool& Bool);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmFlt(const uint64& RecId, const TStr& FieldNm, const double& Flt);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm, const TFltPr& FltPr);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, const TFltV& FltV);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmTm(const uint64& RecId, const TStr& FieldNm, const TTm& Tm);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm, const uint64& TmMSecs);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, const TIntFltKdV& SpV);
    /// Set field value using field name (default implementation throws exception)
    void SetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, const PBowSpV& SpV);   
    
	/// Get field value as JSon object using field id
	virtual PJsonVal GetFieldJson(const uint64& RecId, const int& FieldId) const;
	/// Get field value as human-readable text using field id
	virtual TStr GetFieldText(const uint64& RecId, const int& FieldId) const;
	/// Get field value as JSon object using field name
	virtual PJsonVal GetFieldNmJson(const uint64& RecId, const TStr& FieldNm) const;
	/// Get field value as human-readable text using field name
	virtual TStr GetFieldNmText(const uint64& RecId, const TStr& FieldNm) const;

	/// Helper function for returning JSon definition of fields
    PJsonVal GetStoreFieldsJson() const;
	/// Helper function for returning JSon definition of index keys
    PJsonVal GetStoreKeysJson(const TWPt<TBase>& Base) const;
	/// Helper function for returning JSon definition of joins
    PJsonVal GetStoreJoinsJson(const TWPt<TBase>& Base) const;
	/// Helper function for returning JSon definition of store
    PJsonVal GetStoreJson(const TWPt<TBase>& Base) const;
    /// Parse out record id from record JSon serialization
    uint64 GetRecId(const PJsonVal& RecVal) const;
	
    /// Prints record set with all the field values, useful for debugging
	void PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, TSOut& SOut) const;
    /// Prints record set with all the field values, useful for debugging
	void PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, const TStr& FNm) const;
    /// Prints all records with all the field values, useful for debugging
    void PrintAll(const TWPt<TBase>& Base, TSOut& SOut) const;
    /// Prints all records with all the field values, useful for debugging
    void PrintAll(const TWPt<TBase>& Base, const TStr& FNm) const;
    /// Prints registered fields and joins, useful for debugging
    void PrintTypes(const TWPt<TBase>& Base, TSOut& SOut) const;
    /// Prints registered fields and joins, useful for debugging
    void PrintTypes(const TWPt<TBase>& Base, const TStr& FNm) const;
};
//typedef THash<TUCh, PStore> TUChStoreH;

///////////////////////////////
/// Record.
/// Holds record by reference (store ID and record ID) or by value (store ID and all field values).
class TRec {
private:
    /// Record store
    TWPt<TStore> Store;
    /// True when holding record by reference
	TBool ByRefP;
	/// Record ID (by reference)
	TUInt64 RecId;
    /// Field position in serialization (by value)
    THash<TInt, TInt> FieldIdPosH;
    /// Join position in serialization (by value)
    THash<TInt, TInt> JoinIdPosH;
    /// Record serialization (by value);    
    TMem RecVal;
    /// Output stream for serialization; points to RecVal (by value)
    TRefMemOut RecValOut;

private:
	/// Get QMiner exception for requesting wrong field-type combinations
    PExcept FieldError(const int& FieldId, const TStr& TypeStr) const;

public:
	/// Create empty record (no reference, no value)
    TRec(): ByRefP(false), RecId(TUInt64::Mx), RecValOut(RecVal) { }
	/// Create empty record from a given store. Used to create records by value, 
	/// expects field values to be added using AddField methods.
    TRec(const TWPt<TStore>& _Store): Store(_Store), 
        ByRefP(false), RecId(TUInt64::Mx), RecValOut(RecVal) { }
	/// Create record by reference
    TRec(const TWPt<TStore>& _Store, const uint64& _RecId): Store(_Store), 
        ByRefP(true), RecId(_RecId), RecValOut(RecVal) { }
    /// Constructor from JSon
    TRec(const TWPt<TStore>& _Store, const PJsonVal& JsonVal);    
    /// Copy-constructor
    TRec(const TRec& Rec);
    /// Assignment operator
    TRec& operator=(const TRec& Rec);
    
    // TODO
    TRec(TSIn& SIn): RecValOut(RecVal) { Fail; }
    void Save(TSOut& SOut) const { Fail; };

	/// Check if record is well defined
    bool IsDef() const { return !Store.Empty() && ((ByRefP && (RecId != TUInt64::Mx)) || !ByRefP); }
	/// Get record's store
	const TWPt<TStore>& GetStore() const { return Store; }
	/// Get record's store ID    
	uint GetStoreId() const { return Store->GetStoreId(); }
	/// True when by reference
    bool IsByRef() const { return ByRefP; }
	/// True when by value
    bool IsByVal() const { return !ByRefP; }
	/// Get record id (only valid when by reference)
    uint64 GetRecId() const { return RecId; }
    /// Get record name (only valid when by reference)
    TStr GetRecNm() const { return Store->GetRecNm(RecId); }

    /// Checks if field value is null
	bool IsFieldNull(const int& FieldId) const;
    /// Field value retrieval
    int GetFieldInt(const int& FieldId) const;
    /// Field value retrieval
    void GetFieldIntV(const int& FieldId, TIntV& IntV) const;
    /// Field value retrieval
    uint64 GetFieldUInt64(const int& FieldId) const;
    /// Field value retrieval
	TStr GetFieldStr(const int& FieldId) const;
    /// Field value retrieval
	void GetFieldStrV(const int& FieldId, TStrV& StrV) const;
    /// Field value retrieval
	bool GetFieldBool(const int& FieldId) const;
    /// Field value retrieval
    double GetFieldFlt(const int& FieldId) const;
    /// Field value retrieval
    TFltPr GetFieldFltPr(const int& FieldId) const;
    /// Field value retrieval
    void GetFieldFltV(const int& FieldId, TFltV& FltV) const;
    /// Field value retrieval
    void GetFieldTm(const int& FieldId, TTm& Tm) const;
    /// Field value retrieval
    uint64 GetFieldTmMSecs(const int& FieldId) const;    
    /// Field value retrieval
    void GetFieldNumSpV(const int& FieldId, TIntFltKdV& NumSpV) const;
    /// Field value retrieval
    void GetFieldBowSpV(const int& FieldId, PBowSpV& BowSpV) const;

	/// Get field value as JSon object using field id
	PJsonVal GetFieldJson(const int& FieldId) const;
	/// Get field value as human-readable text using field id
	TStr GetFieldText(const int& FieldId) const;
    
    /// Set field value to NULL
	void SetFieldNull(const int& FieldId);
    /// Set field value
	void SetFieldInt(const int& FieldId, const int& Int);
    /// Set field value
    void SetFieldIntV(const int& FieldId, const TIntV& IntV);
    /// Set field value
    void SetFieldUInt64(const int& FieldId, const uint64& UInt64);
    /// Set field value
	void SetFieldStr(const int& FieldId, const TStr& Str);
    /// Set field value
	void SetFieldStrV(const int& FieldId, const TStrV& StrV);
    /// Set field value
	void SetFieldBool(const int& FieldId, const bool& Bool);
    /// Set field value
    void SetFieldFlt(const int& FieldId, const double& Flt);
    /// Set field value
    void SetFieldFltV(const int& FieldId, const TFltV& FltV);
    /// Set field value
    void SetFieldFltPr(const int& FieldId, const TFltPr& FltPr);
    /// Set field value
    void SetFieldTm(const int& FieldId, const TTm& Tm);
    /// Set field value
    void SetFieldNumSpV(const int& FieldId, const TIntFltKdV& NumSpV);
    /// Set field value
    void SetFieldBowSpV(const int& FieldId, const PBowSpV& BowSpV);
	/// Add join
	void AddJoin(const int& JoinId, const PRecSet& JoinRecSet);

    /// Get record set containing only this record (by reference)
    PRecSet ToRecSet() const;
	/// Execute join with the given id
	PRecSet DoJoin(const TWPt<TBase>& Base, const int& JoinId) const;
	/// Execute join with the given name
	PRecSet DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const;
	/// Execute given join sequence. Each join is given by pair (id, sample size).
	PRecSet DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const;
	/// Execute given join sequence
	PRecSet DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const;
	/// Execute join with given id, returns only one record. 
	/// In case more records in the result, returns the first record.
	TRec DoSingleJoin(const TWPt<TBase>& Base, const int& JoinId) const;
	/// Execute join with given name, returns only one record. 
	/// In case more records in the result, returns the first record.
	TRec DoSingleJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const;
	/// Execute given join sequence, returns only one record.
	/// In case more records in the result, returns the first record.
	TRec DoSingleJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const;
	/// Execute given join sequence, returns only one record. Each join is given
	/// by pair (id, sample size). In case more records in the result, returns 
	/// the first record.
	TRec DoSingleJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const;

	/// Get record as JSon object
	PJsonVal GetJson(const TWPt<TBase>& Base, const bool& FieldsP = true, 
		const bool& StoreInfoP = true, const bool& JoinRecsP = false, 
		const bool& JoinRecFieldsP = false, const bool& RecInfoP = true) const;
};

///////////////////////////////
/// Record Comparator by Frequency. If same, sort by ID
class TRecCmpByFq {
private:
    TBool Asc;
public:
    TRecCmpByFq(const bool& _Asc) : Asc(_Asc) { }

    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        if (Asc) {
            return (RecIdWgt1.Dat == RecIdWgt2.Dat) ?
                (RecIdWgt1.Key < RecIdWgt2.Key) : (RecIdWgt1.Dat < RecIdWgt2.Dat);
        } else {
            return (RecIdWgt2.Dat == RecIdWgt1.Dat) ?
                (RecIdWgt2.Key < RecIdWgt1.Key) : (RecIdWgt2.Dat < RecIdWgt1.Dat);
        }
    }
};

///////////////////////////////
/// Record Comparator by Integer Field. 
class TRecCmpByFieldInt {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFieldInt(const TWPt<TStore>& _Store, const int& _FieldId,
        const bool& _Asc): Store(_Store), FieldId(_FieldId), Asc(_Asc) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        const int RecVal1 = Store->GetFieldInt(RecIdWgt1.Key, FieldId);
        const int RecVal2 = Store->GetFieldInt(RecIdWgt2.Key, FieldId);
        if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
    }
};

///////////////////////////////
/// Record Comparator by Numeric Field. 
class TRecCmpByFieldFlt {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFieldFlt(const TWPt<TStore>& _Store, const int& _FieldId,
        const bool& _Asc): Store(_Store), FieldId(_FieldId), Asc(_Asc) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        const double RecVal1 = Store->GetFieldFlt(RecIdWgt1.Key, FieldId);
        const double RecVal2 = Store->GetFieldFlt(RecIdWgt2.Key, FieldId);
        if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
    }
};

///////////////////////////////
/// Record Comparator by String Field. 
class TRecCmpByFieldStr {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFieldStr(const TWPt<TStore>& _Store, const int& _FieldId,
        const bool& _Asc): Store(_Store), FieldId(_FieldId), Asc(_Asc) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        const TStr RecVal1 = Store->GetFieldStr(RecIdWgt1.Key, FieldId);
        const TStr RecVal2 = Store->GetFieldStr(RecIdWgt2.Key, FieldId);
        if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
    }
};

///////////////////////////////
/// Record Comparator by Time Field. 
class TRecCmpByFieldTm {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFieldTm(const TWPt<TStore>& _Store, const int& _FieldId,
        const bool& _Asc): Store(_Store), FieldId(_FieldId), Asc(_Asc) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        const uint64 RecVal1 = Store->GetFieldTmMSecs(RecIdWgt1.Key, FieldId);
        const uint64 RecVal2 = Store->GetFieldTmMSecs(RecIdWgt2.Key, FieldId);
        if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
    }
};

///////////////////////////////
/// Record Filter by Record Exists. 
class TRecFilterByExists {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
public:
    TRecFilterByExists(const TWPt<TStore>& _Store): Store(_Store) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        return Store->IsRecId(RecIdWgt.Key);
    }
};

///////////////////////////////
/// Record Filter by Record Id. 
class TRecFilterByRecId {
private:
    /// Minimal value
    TUInt64 MinRecId;
    /// Maximal value
    TUInt64 MaxRecId;
public:
    TRecFilterByRecId(const uint64& _MinRecId, const uint64& _MaxRecId): 
        MinRecId(_MinRecId), MaxRecId(_MaxRecId) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        return (MinRecId <= RecIdWgt.Key) && (RecIdWgt.Key <= MaxRecId);
    }
};

///////////////////////////////
/// Record Filter by Record Id Set. 
class TRecFilterByRecIdSet {
private:
    /// Store from which we are sorting the records 
    const TUInt64Set& RecIdSet;
    /// Check for in our out
    TBool InP;
public:
    TRecFilterByRecIdSet(const TUInt64Set& _RecIdSet, const bool _InP): 
        RecIdSet(_RecIdSet), InP(_InP) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        return InP ? RecIdSet.IsKey(RecIdWgt.Key) : !RecIdSet.IsKey(RecIdWgt.Key);
    }
};

///////////////////////////////
/// Record Filter by Record Id. 
class TRecFilterByRecFq {
private:
    /// Minimal value
    TInt MinFq;
    /// Maximal value
    TInt MaxFq;
public:
    TRecFilterByRecFq(const int& _MinFq, const int& _MaxFq): 
        MinFq(_MinFq), MaxFq(_MaxFq) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        return (MinFq <= RecIdWgt.Dat) && (RecIdWgt.Dat <= MaxFq);
    }
};

///////////////////////////////
/// Record Filter by Integer Field. 
class TRecFilterByFieldInt {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Minimal value
    TInt MinVal;
    /// Maximal value
    TInt MaxVal;
public:
    TRecFilterByFieldInt(const TWPt<TStore>& _Store, const int& _FieldId, const int& _MinVal,
        const int& _MaxVal): Store(_Store), FieldId(_FieldId), MinVal(_MinVal), MaxVal(_MaxVal) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        const int RecVal = Store->GetFieldInt(RecIdWgt.Key, FieldId);
        return (MinVal <= RecVal) && (RecVal <= MaxVal);
    }
};

///////////////////////////////
/// Record Filter by Numeric Field. 
class TRecFilterByFieldFlt {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Minimal value
    TFlt MinVal;
    /// Maximal value
    TFlt MaxVal;
public:
    TRecFilterByFieldFlt(const TWPt<TStore>& _Store, const int& _FieldId, const double& _MinVal,
        const double& _MaxVal): Store(_Store), FieldId(_FieldId), MinVal(_MinVal), MaxVal(_MaxVal) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        const double RecVal = Store->GetFieldFlt(RecIdWgt.Key, FieldId);
        return (MinVal <= RecVal) && (RecVal <= MaxVal);
    }
};

///////////////////////////////
/// Record Filter by String Field. 
class TRecFilterByFieldStr {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// String value
    const TStr& StrVal;
public:
    TRecFilterByFieldStr(const TWPt<TStore>& _Store, const int& _FieldId, 
        const TStr& _StrVal): Store(_Store), FieldId(_FieldId), StrVal(_StrVal) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        const TStr RecVal = Store->GetFieldStr(RecIdWgt.Key, FieldId);
        return StrVal == RecVal;
    }
};

///////////////////////////////
/// Record Filter by String Field Set. 
class TRecFilterByFieldStrSet {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// String values
    const TStrSet& StrSet;
public:
    TRecFilterByFieldStrSet(const TWPt<TStore>& _Store, const int& _FieldId, 
        const TStrSet& _StrSet): Store(_Store), FieldId(_FieldId), StrSet(_StrSet) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        const TStr RecVal = Store->GetFieldStr(RecIdWgt.Key, FieldId);
        return StrSet.IsKey(RecVal);
    }
};


///////////////////////////////
/// Record Filter by Time Field. 
class TRecFilterByFieldTm {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Minimal value
    TUInt64 MinVal;
    /// Maximal value
    TUInt64 MaxVal;
public:
    TRecFilterByFieldTm(const TWPt<TStore>& _Store, const int& _FieldId, const uint64& _MinVal,
        const uint64& _MaxVal): Store(_Store), FieldId(_FieldId), MinVal(_MinVal), MaxVal(_MaxVal) { }
    TRecFilterByFieldTm(const TWPt<TStore>& _Store, const int& _FieldId, 
        const TTm& _MinVal, const TTm& _MaxVal): Store(_Store), FieldId(_FieldId), 
        MinVal(_MinVal.IsDef() ? TTm::GetMSecsFromTm(_MinVal) : (uint64)TUInt64::Mn), 
        MaxVal(_MaxVal.IsDef() ? TTm::GetMSecsFromTm(_MaxVal) : (uint64)TUInt64::Mx) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt) const {
        const uint64 RecVal = Store->GetFieldTmMSecs(RecIdWgt.Key, FieldId);
        return (MinVal <= RecVal) && (RecVal <= MaxVal);
    }
};

///////////////////////////////
/// Record Splitter by Time Field. 
class TRecSplitterByFieldTm {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Maximal difference value
    TUInt64 DiffMSecs;
    
public:
    TRecSplitterByFieldTm(const TWPt<TStore>& _Store, const int& _FieldId, const uint64& _DiffMSecs):
        Store(_Store), FieldId(_FieldId), DiffMSecs(_DiffMSecs) { }
    
    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
        const uint64 RecVal1 = Store->GetFieldTmMSecs(RecIdWgt1.Key, FieldId);
        const uint64 RecVal2 = Store->GetFieldTmMSecs(RecIdWgt2.Key, FieldId);
        return (RecVal2 - RecVal1) > DiffMSecs;
    }
};

///////////////////////////////
/// Record Set. 
/// Holds a collection of record IDs from one store.
/// Records are stored internally as a vector of ids.
/// Records can have corresponding integer weights.
/// Holds pointers to aggregates computed over records in the set.
class TRecSet {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TRecSet>;
private:
	/// Store
	TWPt<TStore> Store;
	/// True when records have valid weights
    TBool WgtP;
	/// Vector of pairs (record id, weight)
	TUInt64IntKdV RecIdFqV;
    /// Vector of computed aggregates
    TVec<PAggr> AggrV;

private:
	/// Samples records from result set
	/// @param SampleSize number of records to sample out
	/// @param SortedP true when records are ordered according to the weight
	/// @param SampleRecIdFqV reference to vector for storing sampled records
	void GetSampleRecIdV(const int& SampleSize, 
		const bool& SortedP, TUInt64IntKdV& SampleRecIdFqV) const;
	/// Removes records from this result set that are not part of the provided
	void LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV);

	TRecSet() { }
    TRecSet(const TWPt<TStore>& Store, const uint64& RecId, const int& Wgt);
    TRecSet(const TWPt<TStore>& Store, const TUInt64V& RecIdV);
	TRecSet(const TWPt<TStore>& Store, const TIntV& RecIdV);
	TRecSet(const TWPt<TStore>& Store, const TUInt64IntKdV& _RecIdFqV, const bool& _WgtP);
	TRecSet(const TWPt<TBase>& Base, TSIn& SIn);

public:
	/// Create empty set for a given store
	static PRecSet New(const TWPt<TStore>& Store);
	/// Create record set with one record
	static PRecSet New(const TWPt<TStore>& Store, const uint64& RecId, const int& Wgt = 1);
	/// Create record set with one record (must be by reference)
	static PRecSet New(const TWPt<TStore>& Store, const TRec& Rec);
	/// Create record set from a given vector of record ids
	static PRecSet New(const TWPt<TStore>& Store, const TUInt64V& RecIdV);
	/// Create record set from a given vector of record ids
	static PRecSet New(const TWPt<TStore>& Store, const TIntV& RecIdV);
	/// Create record set from given vector of (Record id, weight) pairs
	/// @param WgtP true when RecIdFqV contains valid weights 
	static PRecSet New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV, const bool& WgtP);

	/// Load record set from input stream.
	static PRecSet Load(const TWPt<TBase>& Base, TSIn& SIn){ return new TRecSet(Base, SIn); }
	/// Save record set from input stream. Does not serialize aggregates.
	void Save(TSOut& SOut);

    /// True when record set contains valid record weights
    bool IsWgt() const { return WgtP; }
	/// True when no record
	bool Empty() const { return RecIdFqV.Empty(); }
    /// Get store of the record set
    const TWPt<TStore>& GetStore() const { return Store; }
	/// Get store id of the record set
	uint GetStoreId() const { return Store->GetStoreId(); }

	/// Number of records in the set
	int GetRecs() const { return RecIdFqV.Len(); }	// FIXME this method should return uint64
	/// Get RecN-th record as TRec by reference
    TRec GetRec(const int& RecN) const { return TRec(GetStore(), RecIdFqV[RecN].Key); }
	/// Get id of RecN-th record
	uint64 GetRecId(const int& RecN) const { return RecIdFqV[RecN].Key; }
	/// Get weight of RecN-th record
	int GetRecFq(const int& RecN) const { return WgtP ? RecIdFqV[RecN].Dat.Val : 1; }
	/// Get last record in the set as TRec by reference
	TRec GetLastRec() const { return TRec(GetStore(), RecIdFqV.Last().Key); }
	/// Get id of the last record in the set
	uint64 GetLastRecId() const { return RecIdFqV.Last().Key; }
	/// Get reference to complete vector of pairs (record id, weight)
    const TUInt64IntKdV& GetRecIdFqV() const { return RecIdFqV; }
    /// Get direct reference to elements of vecotr
    const TUInt64IntKd& GetRecIdFq(const int& RecN) const { return RecIdFqV[RecN]; }

	/// Load record ids into the provided vector
    void GetRecIdV(TUInt64V& RecIdV) const;
	/// Load record ids into the provided hashset
	void GetRecIdSet(TUInt64Set& RecIdSet) const;
	/// Load record ids and weights into the provided hash table (ids map to weights)
	void GetRecIdFqH(THash<TUInt64, TInt>& RecIdFqH) const;

    /// Set weight for the RecN-th record to `Fq'
	void PutRecFq(const int& RecN, const int& Fq) { RecIdFqV[RecN].Dat = Fq; }
	/// Use provided map (record id -> weight) to set record weights
	void PutAllRecFq(const THash<TUInt64, TInt>& RecIdFqH);
	/// Remove the last record from the set
    void DelLastRec() { RecIdFqV.DelLast(); }
	/// Randomly shuffle the order of records in the set. Uses provided random number generator.
    void Shuffle(TRnd& Rnd) { RecIdFqV.Shuffle(Rnd); }
	/// Reverse the order of records in the set
	void Reverse() { RecIdFqV.Reverse(); }
	/// Keep only first `Recs' records
	void Trunc(const int& Recs) { RecIdFqV.Trunc(Recs); }
	/// Sort records by their record ids
	/// @param Asc True for sorting in increasing order
	void SortById(const bool& Asc = true);
	/// Sort records by their weight
	/// @param Asc True for sorting in increasing order
	void SortByFq(const bool& Asc = true);
	/// Sort records according to filed with id `SortFieldId'
	/// @param Asc True for sorting in increasing order
	void SortByField(const bool& Asc, const int& SortFieldId);
	/// Sort records according to given comparator
	template <class TCmp> void SortCmp(const TCmp& Cmp) { RecIdFqV.SortCmp(Cmp); }

	/// Filter records to keep only the ones which actually exist
	void FilterByExists();
	/// Filter records to keep only the ones with id between `MinRecId' and `MaxRecId'.
	void FilterByRecId(const uint64& MinRecId, const uint64& MaxRecId);
	/// Filter records to keep only the ones that are present in provided `RecIdSet'
	void FilterByRecIdSet(const TUInt64Set& RecIdSet);
	/// Filter records to keep only the ones with weight between `MinFq' and `MaxFq'
	void FilterByFq(const int& MinFq, const int& MaxFq);
	/// Filter records to keep only the ones with values of a given field within given range
	void FilterByFieldInt(const int& FieldId, const int& MinVal, const int& MaxVal);
	/// Filter records to keep only the ones with values of a given field within given range
	void FilterByFieldFlt(const int& FieldId, const double& MinVal, const double& MaxVal);
	/// Filter records to keep only the ones with values of a given field equal to `FldVal'
	void FilterByFieldStr(const int& FieldId, const TStr& FldVal);
	/// Filter records to keep only the ones with values of a given field present in `ValSet'
	void FilterByFieldStrSet(const int& FieldId, const TStrSet& ValSet);
	/// Filter records to keep only the ones with values of a given field within given range
	void FilterByFieldTm(const int& FieldId, const uint64& MinVal, const uint64& MaxVal);
	/// Filter records to keep only the ones with values of a given field within given range
	void FilterByFieldTm(const int& FieldId, const TTm& MinVal, const TTm& MaxVal);
	/// Filter records to keep only the ones with values of a given field within given range
	template <class TFilter> void FilterBy(const TFilter& Filter);
    
    /// Split records into several whenever value of two consecutive records above threshold
    TVec<PRecSet> SplitByFieldTm(const int& FieldId, const uint64& DiffMSecs) const;
    /// Split records into several whenever value of two consecutive records above threshold
    template <class TSplitter> TVec<PRecSet> SplitBy(const TSplitter& Splitter) const;

	/// Remove record from the set (warning: time complexity O(GetRecs()) )
	void RemoveRecId(const TUInt64& RecId);
	/// Remove records present in `RemoveItemIdSet' from the set
	/// (warning: time complexity O(GetRecs()*RemoveItemIdSet.Length()) )
	void RemoveRecIdSet(THashSet<TUInt64>& RemoveItemIdSet);
	
    /// Create a cloned record set. Forgets aggregations.
    PRecSet Clone() const;
    /// Returns a new record set generated by sampling this one
	PRecSet GetSampleRecSet(const int& SampleSize, const bool& SortedP) const;
	/// Get record set containing `Limit' records starting from `RecN=Offset'
	PRecSet GetLimit(const int& Limit, const int& Offset) const;

	/// Merge this record set with the provided one. Result is stored in a new record set.
	/// Merging does not assume any sort order. In the process, records in this record set 
	/// are sorted by ids.
	PRecSet GetMerge(const PRecSet& RecSet) const;
	/// Merges provided record set with `this'. Merging does not assume  any sort order. 
	/// In the process, records in this record set are sorted by ids.
	void Merge(const PRecSet& RecSet);
	/// Merges all provided record sets with `this'. Merging does not assume any sort order.
	/// In the process, records in this record set are sorted by ids.
	void Merge(const TVec<PRecSet>& RecSetV);
	/// Interest this record set with the provided one. Result is stored in a new record set.
	PRecSet GetIntersect(const PRecSet& RecSet);

	/// Execute join with the given id
	/// @param SampleSize Sample size used to do the join. When set to -1, all the records are used.
	/// @param SortedP True when records in this record set are sorted according the weight, to help with sampling
	PRecSet DoJoin(const TWPt<TBase>& Base, const int& JoinId, 
		const int& SampleSize = -1, const bool& SortedP = false) const;
	/// Execute join with the given name
	/// @param SampleSize Sample size used to do the join. When set to -1, all the records are used.
	/// @param SortedP True when records in this record set are sorted according the weight, to help with sampling
	PRecSet DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm, 
		const int& SampleSize = -1, const bool& SortedP = false) const;
	/// Execute given join sequence. Each join is given by pair (id, sample size).
	/// @param SortedP True when records in this record set are sorted according the weight, to help with sampling
	PRecSet DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV, const bool& SortedP) const;
	/// Execute given join sequence.
	/// @param SortedP True when records in this record set are sorted according the weight, to help with sampling
	PRecSet DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const bool& SortedP) const;

    /// Get number of aggregations in the record set
	int GetAggrs() const { return AggrV.Len(); }
	/// Get AggrN-th aggregation
	const PAggr& GetAggr(const int& AggrN) const { return AggrV[AggrN]; }
	/// Add new aggregation to the record set
	void AddAggr(const PAggr& Aggr) { AggrV.Add(Aggr); }

	/// Generate human readable printout of the records stored in the record set
	void Print(const TWPt<TBase>& Base, TSOut& SOut);
	/// Generate human readable printout of the records stored in the record set
	void Print(const TWPt<TBase>& Base, TStr& FNm);

	/// Export records set aggregations to JSon object. 
	PJsonVal GetAggrJson() const;
	/// Export records to JSon object.
	PJsonVal GetJson(const TWPt<TBase>& Base, const int& _MxHits = -1, const int& Offset = 0, 
		const bool& FieldsP = false, const bool& AggrsP = true, const bool& StoreInfoP = true,
		const bool& JoinRecsP = false, const bool& JoinRecFieldsP = false) const;
};
typedef TVec<PRecSet> TRecSetV;

// implementation of template functions
template <class TFilter> 
void TRecSet::FilterBy(const TFilter& Filter) {
	// prepare an empty key-dat vector for storing records that pass the filter
	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
    for (int RecN = 0; RecN < Recs; RecN++) {
        const TUInt64IntKd& RecIdFq = RecIdFqV[RecN];
        // check the filter
        if (Filter(RecIdFq)) { NewRecIdFqV.Add(RecIdFq); }
    }
	// overwrite old result vector with filtered list
	RecIdFqV = NewRecIdFqV;    
}

template <class TSplitter> 
TVec<PRecSet> TRecSet::SplitBy(const TSplitter& Splitter) const {
    TRecSetV ResV;
    // if no records, nothing to do
    if (Empty()) { return ResV; }
    // initialize with the first record
    TUInt64IntKdV NewRecIdFqV; NewRecIdFqV.Add(RecIdFqV[0]);
    // go over the rest and see when to split
    for (int RecN = 1; RecN < GetRecs(); RecN++) {
        if (Splitter(RecIdFqV[RecN-1], RecIdFqV[RecN])) {
            // we need to split, first we create record set for all existing records
            ResV.Add(TRecSet::New(Store, NewRecIdFqV, IsWgt()));
            // and initialize a new one
            NewRecIdFqV.Clr(false);            
        }
        // add new record to the next record set
        NewRecIdFqV.Add(RecIdFqV[RecN]);
    }
    // add last record set to the result list
    ResV.Add(TRecSet::New(GetStore(), NewRecIdFqV, IsWgt()));
    // done
    return ResV;
}

///////////////////////////////
// QMiner-Index-Typedefs
typedef TIntUInt64Pr TKeyWord;
typedef TIntUInt64PrV TKeyWordV;

///////////////////////////////
/// Index Key Type
typedef enum {
	oiktUndef    = 0,
	oiktValue    = (1 << 0), ///< Index by exact value, using inverted index
	oiktText     = (1 << 1), ///< Index as free text, using inverted index 
	oiktLocation = (1 << 2), ///< Index as location. using geoindex 
	oiktInternal = (1 << 3)  ///< Index used internaly for joins, using inverted index
} TIndexKeyType;

///////////////////////////////
/// Index Key Sort Type
typedef enum { 
	oikstUndef = 0, 
	oikstByStr = 1, ///< Sort lexicograficly as string
	oikstById  = 2, ///< Sort by index word id
	oikstByFlt = 3  ///< Sort as number
} TIndexKeySortType;

///////////////////////////////
/// Index Key. 
/// Information about one index key.
class TIndexKey {
private:
	/// Store ID of records indexed by this key
	TUInt StoreId;
	/// Key ID, unique on the level of Base
	TInt KeyId;
	/// Key name, unique on the level of Base
	TStr KeyNm;
	/// Vocabulary ID, which provides range of possible values for this key
	TInt WordVocId;
	/// Key type
	TIndexKeyType TypeFlags;
	/// Sort type of the key
	TIndexKeySortType SortType;
	/// List of fields indexed by this key
	TIntV FieldIdV;
	/// Linked join (only for internal field)
	TStr JoinNm;
    /// Tokenizer, when key requires one (e.g. text)
    PTokenizer Tokenizer;

public:
	/// Empty constructor creates undefined key
	TIndexKey(): StoreId(TUInt::Mx), KeyId(-1), KeyNm(""), 
		WordVocId(-1), TypeFlags(oiktUndef), SortType(oikstUndef) { }
	/// Create internal key, used for index joins
	TIndexKey(const uint& _StoreId, const TStr& _KeyNm, const TStr& _JoinNm): 
		StoreId(_StoreId), KeyNm(_KeyNm), WordVocId(-1), TypeFlags(oiktInternal), 
		SortType(oikstUndef), JoinNm(_JoinNm) { TValidNm::AssertValidNm(KeyNm); }
	/// Create new key using given word vocabulary
	TIndexKey(const uint& _StoreId, const TStr& _KeyNm, const int& _WordVocId, 
		const TIndexKeyType& _Type, const TIndexKeySortType& _SortType);
	
	/// Deserialize key from the stream
	TIndexKey(TSIn& SIn);
	/// Serialization key to the stream
	void Save(TSOut& SOut) const;

	/// True when key is well defined
	bool IsDef() const { return !KeyNm.Empty(); }
	/// Get store id which is being index by the key
	uint GetStoreId() const { return StoreId; }
	/// Get key id
	int GetKeyId() const { return KeyId; }
	/// Get key name
	TStr GetKeyNm() const { return KeyNm; }
	/// Set key id (used only when creating new keys)
	void PutKeyId(const int& _KeyId) { KeyId = _KeyId; }


    /// Get key type
    TIndexKeyType GetTypeFlags() const { return TypeFlags; }
	/// Checks key type is value
	bool IsValue() const { return ((TypeFlags & oiktValue) != 0); }
	/// Checks key type is text
	bool IsText() const { return ((TypeFlags & oiktText) != 0); }
	/// Checks key type is location
	bool IsLocation() const { return ((TypeFlags & oiktLocation) != 0); }
	/// Checks key type is internal
	bool IsInternal() const { return ((TypeFlags & oiktInternal) != 0); }

	/// Get key sort type
	TIndexKeySortType GetSortType() const { return SortType; }
	/// Checks if key is sortable
	bool IsSort() const { return SortType != oikstUndef; }
	/// Checks if key is sortable lexicographically
	bool IsSortByStr() const { return SortType == oikstByStr; }
	/// Checks if key is sortable as number
	bool IsSortByFlt() const { return SortType == oikstByFlt; }
	/// Checks if key is sortable by word id in the vocabulary
	bool IsSortById() const { return SortType == oikstById; }
    
	/// Checks if the key has assigned word vocabulary (e.g. locations and joins do not)
	bool IsWordVoc() const { return WordVocId != -1; }
	/// Get id of word vocabulary used by the key
	int GetWordVocId() const { return WordVocId; }    

	/// Link key to store fields
	void AddField(const int& FieldId) { FieldIdV.Add(FieldId); }
	/// Check if there are any linked fields
	bool IsFields() const { return !FieldIdV.Empty(); }
	/// Get number of connected fields
	int GetFields() const { return FieldIdV.Len(); }
	/// Get id of FieldIdN-th field
	int GetFieldId(const int& FieldIdN) const { return FieldIdV[FieldIdN]; }

	/// Get name of associated join
	const TStr& GetJoinNm() const { return JoinNm; }
    
    /// Do we have a tokenizer
    bool IsTokenizer() const { return !Tokenizer.Empty(); }
    /// Get the tokenizer
    const PTokenizer& GetTokenizer() const { return Tokenizer; }
    /// Set the tokenizer
    void PutTokenizer(const PTokenizer& _Tokenizer) { Tokenizer = _Tokenizer; }
};

///////////////////////////////
// QMiner-Index-Word-Vocabulary
class TIndexWordVoc {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TIndexWordVoc>;
    /// Word vocabulary name
    TStr WordVocNm;
	/// Count of records sent through this vocabulary
	TUInt64 Recs; 
	/// Hash table with all the words
	TStrHash<TInt> WordH;

	TIndexWordVoc() { }
	TIndexWordVoc(TSIn& SIn): WordVocNm(SIn), WordH(SIn) { }
public:	
	/// Create new empty vocabulary
	static TPt<TIndexWordVoc> New() { return new TIndexWordVoc; }
	/// Load existing word vocabulary from stream
	static TPt<TIndexWordVoc> Load(TSIn& SIn) { return new TIndexWordVoc(SIn); }
	
	/// Serialize vocabulary to stream
	void Save(TSOut& SOut) { WordVocNm.Save(SOut); WordH.Save(SOut); }

	/// Check if word with given ID exists
	bool IsWordId(const uint64& WordId) const { return WordH.IsKeyId((int)WordId); }
	/// Check if given word exists
	bool IsWordStr(const TStr& WordStr) const { return WordH.IsKey(WordStr); }
	/// Get number of words in the vocabulary
	uint64 GetWords() const { return (uint64)WordH.Len(); }
	/// Get ID of a given word
	uint64 GetWordId(const TStr& WordStr) const { return (uint64)WordH.GetKeyId(WordStr); }
	/// Get word corresponding to the given ID
	TStr GetWordStr(const uint64& WordId) const { return WordH.GetKey((int)WordId); }
	/// Get number of time given word was indexed so far
	uint64 GetWordFq(const uint64& WordId) const { return WordH[(int)WordId]; }
	/// Get all the words from the vocabulary as a vector
	void GetAllWordV(TStrV& WordStrV) const { WordH.GetKeyV(WordStrV); }
	/// Get all the words and count of their occurrences 
	void GetAllWordFqV(TStrIntPrV& WordStrFqV) const { WordH.GetKeyDatPrV(WordStrFqV); }
	/// Get vector of all words that match given wildchar query
	void GetWcWordIdV(const TStr& WcStr, TUInt64V& WcWordIdV);
	
	/// Get all words that have ID greater than `startWordId'
	void GetAllGreaterById(const uint64& StartWordId, TUInt64V& AllGreaterV);
	/// Get all words that have value lexicographically greater than `startWordId'
	void GetAllGreaterByStr(const uint64& StartWordId, TUInt64V& AllGreaterV);
	/// Get all words that have value numerically greater than `startWordId'
	void GetAllGreaterByFlt(const uint64& StartWordId, TUInt64V& AllGreaterV);
	/// Get all words that have ID smaller than `startWordId'
	void GetAllLessById(const uint64& StartWordId, TUInt64V& AllLessV);
	/// Get all words that have value lexicographically smaller the `startWordId'
	void GetAllLessByStr(const uint64& StartWordId, TUInt64V& AllLessV);
	/// Get all words that have value numerically smaller the `startWordId'
	void GetAllLessByFlt(const uint64& StartWordId, TUInt64V& AllLessV);
	
	/// Increase count of records that were sent through this vocabulary (useful for document frequency counts)
	void IncRecs() { Recs++; }
	/// Add new word to the vocabulary (if existing, it increases its count)
	uint64 AddWordStr(const TStr& WordStr);
    
    /// Check if vocabulary has a name assigned (used for easier referencing in schemas)
    bool IsWordVocNm() const { return !WordVocNm.Empty(); }
    /// Get name of the vocabulary
    const TStr& GetWordVocNm() const { return WordVocNm; }    
    /// Set vocabulary name
    void SetWordVocNm(const TStr& _WordVocNm) { WordVocNm = _WordVocNm; }
};
typedef TPt<TIndexWordVoc> PIndexWordVoc;
typedef TVec<PIndexWordVoc> TIndexWordVocV;

///////////////////////////////
// QMiner-Index-Vocabulary
class TIndexVoc {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TIndexVoc>;
    /// List of all the keys
    THash<TUIntStrPr, TIndexKey> KeyH;
    /// Keys split by stores
    THash<TUInt, TIntSet> StoreIdKeyIdSetH;
	/// Word vocabularies
    TIndexWordVocV WordVocV;
	/// Used to return empty set by reference
	TIntSet EmptySet;

	/// Get editable word vocabulary for a given key
	PIndexWordVoc& GetWordVoc(const int& KeyId);
	/// Get constant word vocabulary for a given key
	const PIndexWordVoc& GetWordVoc(const int& KeyId) const;

	TIndexVoc() { }
    TIndexVoc(TSIn& SIn);
public:
	/// Create new index vocabulary
    static PIndexVoc New() { return new TIndexVoc; }
	/// Load existing vocabulary from stream
    static PIndexVoc Load(TSIn& SIn) { return new TIndexVoc(SIn); }
	/// Serialize vocabulary to stream
    void Save(TSOut& SOut) const;

    /// Get number of keys
	int GetKeys() const { return KeyH.Len(); }
	/// Checks if key with given id exists
	bool IsKeyId(const int& KeyId) const;
	/// Checks if key with given name exists
	bool IsKeyNm(const uint& StoreId, const TStr& KeyNm) const;
	/// Get ID of key with a given name from a given store
	int GetKeyId(const uint& StoreId, const TStr& KeyNm) const;
	/// Get store indexed by given key
	uint GetKeyStoreId(const int& KeyId) const;
	/// Get name of the given key
	TStr GetKeyNm(const int& KeyId) const;
	/// Get reference to key description
	const TIndexKey& GetKey(const int& KeyId) const;
	/// Get reference to key description
	const TIndexKey& GetKey(const uint& StoreId, const TStr& KeyNm) const;
	
	/// Create new word vocabulary, returns its ID
	int NewWordVoc() { return WordVocV.Add(TIndexWordVoc::New()); }
    /// Get Id of word vocabulary with a given name if one exists, -1 otherwise
    int GetWordVoc(const TStr& WordVocNm) const;
    /// Set the name of word vocabulary 
    void SetWordVocNm(const int& WordVocId, const TStr& WordVocNm);
    
	/// Create new key in the index vocabulary
	int AddKey(const uint& StoreId, const TStr& KeyNm, const int& WordVocId, 
		const TIndexKeyType& Type, const TIndexKeySortType& SortType = oikstUndef);
	/// Create new internal key
	int AddInternalKey(const uint& StoreId, const TStr& KeyNm, const TStr& JoinNm);
	/// Linking key to a field
	void AddKeyField(const int& KeyId, const uint& StoreId, const int& FieldId);
    /// Check if store has any index keys
    bool IsStoreKeys(const uint& StoreId) const;
	/// Get set of all the keys for a given store
    const TIntSet& GetStoreKeys(const uint& StoreId) const;

    /// Checks if given key contains a word vocabulary
	bool IsWordVoc(const int& KeyId) const;
	/// Checks if word is in the vocabulary of a key
	bool IsWordId(const int& KeyId, const uint64& WordId) const;
	/// Checks if word is in the vocabulary of a key
	bool IsWordStr(const int& KeyId, const TStr& WordStr) const;
	/// Get size of word vocabulary of a key
	uint64 GetWords(const int& KeyId) const;
	/// Get all words in the vocabulary of a key
	void GetAllWordStrV(const int& KeyId, TStrV& WordStrV) const;
	/// Get all words and weights in the vocabulary of a key
	void GetAllWordStrFqV(const int& KeyId, TStrIntPrV& WordStrV) const;
	/// Get word with a given id of a key
	TStr GetWordStr(const int& KeyId, const uint64& WordId) const;
	/// Get weight of a word of a key
	uint64 GetWordFq(const int& KeyId, const uint64& WordId) const;
	/// Get word id from a key for a given string (does not add new words)
    uint64 GetWordId(const int& KeyId, const TStr& WordStr) const;
	/// Get word ids from a key for a given text (does not add new words)
	void GetWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) const;
	/// For parsing strings (adds new words)
    uint64 AddWordStr(const int& KeyId, const TStr& WordStr);
	/// Get word ids from a key for a given text (adds new words)
	void AddWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV);
	/// Get word ids from a key for a given texts (adds new words)
	void AddWordIdV(const int& KeyId, const TStrV& TextStr, TUInt64V& WordIdV);
	/// Get vector of all words from a key that match given wildchar query
	void GetWcWordIdV(const int& KeyId, const TStr& WcStr, TUInt64V& WcWordIdV);
    /// Get all words from a key that are greater than `startWordId
    void GetAllGreaterV(const int& KeyId, const uint64& StartWordId, TKeyWordV& AllGreaterV);
    /// Get all words from a key that are smaller than `startWordId
    void GetAllLessV(const int& KeyId, const uint64& StartWordId, TKeyWordV& AllLessV);
    
    /// Get tokenizer from a key
    const PTokenizer& GetTokenizer(const int& KeyId) const;
    /// Set tokenizer for a key
    void PutTokenizer(const int& KeyId, const PTokenizer& Tokenizer);

	/// Save human-readable statistics to a file
	void SaveTxt(const TWPt<TBase>& Base, const TStr& FNm) const;
};

///////////////////////////////
/// Index Query Element Type. 
/// Types of nodes in the parsed query tree
typedef enum { 
	oqitUndef        = 0, 
	oqitLeafGix      = 1, ///< Leaf inverted index query
	oqitGeo          = 8, ///< Geoindex query
	oqitAnd          = 2, ///< AND between two or more queries
	oqitOr           = 3, ///< OR between two or more queries
	oqitNot          = 4, ///< NOT on current matching records
	oqitJoin         = 5, ///< Execute join on given records
	oqitRecSet       = 6, ///< Pass on a records set
	oqitRec          = 7, ///< Pass on a record
	oqitStore        = 9  ///< Incude all records from a store
} TQueryItemType;

///////////////////////////////
/// Index Query Comparison Operators.
/// Comparison operators that can be specified between a field and a value
typedef enum { 
	oqctUndef    = 0,
	oqctEqual    = 1, ///< Equalst (==)
	oqctGreater  = 2, ///< Greater then (>)
	oqctLess     = 3, ///< Less then (<)
	oqctNotEqual = 4, ///< Not equal (!=)
	oqctWildChar = 5  ///< Wildchar string matching (* for zero or more chars, ? for exactly one char)
} TQueryCmpType;

///////////////////////////////
/// Query Item
class TQueryItem; typedef TVec<TQueryItem> TQueryItemV;

class TQueryItem {
private:
	/// type of query item
	TQueryItemType Type;
	/// Index key (for leaf node)
    TInt KeyId;
	/// Value or text query (for leaf node)
    TUInt64V WordIdV;
	/// Comparison between field and value (for leaf node)
    TQueryCmpType CmpType;
	/// Geo. coordinates (for location query)
	TFltPr Loc;
	/// Radius of search space in meters (for location query)
	TFlt LocRadius;
	/// Number of nearest neighbors of search space (for location query)
	TInt LocLimit;
	/// List of subordinate query items.
	/// Has exactly one element when NOT or JOIN node type
	TQueryItemV ItemV;
    /// Join ID (for join nodes)
	TInt JoinId;
	/// Join sampling size (for join nodes). Value -1 means everything.
	TInt SampleSize;
    /// Record set which this query node returns
	PRecSet RecSet;
    /// Record which this query node returns
	TRec Rec;
	/// Store which this query node returns
	TUInt StoreId;
	
	/// Parse Value for leaf nodes (result stored in WordIdV)
	void ParseWordStr(const TStr& WordStr, const TWPt<TIndexVoc>& IndexVoc);

	/// Parse join query from json (can be one or an array of joins)
	TWPt<TStore> ParseJoins(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
	/// Parse single join from json
	TWPt<TStore> ParseJoin(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
	/// Parse store of query
	TWPt<TStore> ParseFrom(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
	/// Parse conditions keys
	void ParseKeys(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
		const PJsonVal& JsonVal, const bool& IgnoreOrP);
	/// Constructor for parsing query from json
	TQueryItem(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
	/// Construct for parsing query from json, where store is specified in advanced
	TQueryItem(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const PJsonVal& JsonVal);
	/// Construct query item, where key value is given as json
	TQueryItem(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
		const TStr& KeyNm, const PJsonVal& KeyVal);

public:
	/// New undefined item (necessary for resizing vectors)
	TQueryItem() { };
	/// Create query returning known record
    TQueryItem(const TWPt<TStore>& Store, const uint64& RecId);
	/// Create query returning known record
	TQueryItem(const TRec& _Rec);
	/// Create query returning known record set
	TQueryItem(const PRecSet& _RecSet);
	/// Create new inverted index leaf query
    TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, const uint64& WordId, 
		const TQueryCmpType& _CmpType = oqctEqual);
	/// Create new inverted index leaf query
    TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, const TStr& WordStr, 
		const TQueryCmpType& _CmpType = oqctEqual);
	/// Create new inverted index leaf query
    TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm, 
		const TStr& WordStr, const TQueryCmpType& _CmpType = oqctEqual);
	/// Create new inverted index leaf query
    TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& KeyNm, 
		const TStr& WordStr, const TQueryCmpType& _CmpType = oqctEqual);
	/// New leaf location query (limit always required, range used when positive)
	TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, 
		const TFltPr& _Loc, const int& _LocLimit = 100, 
		const double& _LocRadius = -1.0);
	/// New leaf location query (limit always required, range used when positive)
	TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, 
		const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit = 100,
		const double& _LocRadius = -1.0);
	/// New leaf location query (limit always required, range used when positive)
	TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, 
		const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit = 100,
		const double& _LocRadius = -1.0);
	/// New non-leaf query item
	TQueryItem(const TQueryItemType& _Type);
	/// New non-leaf query item, with given subordinate item
	TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item);
	/// New non-leaf query item, with given subordinate items
	TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item1, const TQueryItem& Item2);
	/// New non-leaf query item, with given subordinate items
	TQueryItem(const TQueryItemType& _Type, const TQueryItemV& _ItemV);
	/// New join query item
	TQueryItem(const int& _JoinId, const int& _SampleSize, const TQueryItem& Item);
	/// New join query item
	TQueryItem(const TWPt<TBase>& Base, const TStr& JoinNm, const int& _SampleSize, const TQueryItem& Item);

	/// Check if query well defined
	bool IsDef() const { return Type != oqitUndef; }
	/// Get query type
	TQueryItemType GetType() const { return Type; }
	/// Check query type
	bool IsRec() const { return (Type == oqitRec); }
	/// Check query type
	bool IsRecSet() const { return (Type == oqitRecSet); }
	/// Check query type
	bool IsLeafGix() const { return (Type == oqitLeafGix); }
	/// Check query type
	bool IsGeo() const { return (Type == oqitGeo); }
	/// Check query type
	bool IsAnd() const { return (Type == oqitAnd); }
	/// Check query type
	bool IsOr() const { return (Type == oqitOr); }
	/// Check query type
	bool IsNot() const { return (Type == oqitNot); }
	/// Check query type
	bool IsJoin() const { return (Type == oqitJoin); }
	/// Check query type
	bool IsStore() const { return (Type == oqitStore); }

	/// Get result store id
	uint GetStoreId(const TWPt<TBase>& Base) const;
	/// Get result store id
	TWPt<TStore> GetStore(const TWPt<TBase>& Base) const;
	/// Check if there are no subordinate items or values
	bool Empty() const { return !IsItems() && !IsWordIds(); }
	/// Check if result is weighted (only or-items)
	bool IsWgt() const;

	/// Get number of values (for inverted index queries)
	bool IsWordIds() const { return !WordIdV.Empty(); }
	/// Get Index key (for inverted index queries)
	int GetKeyId() const { return KeyId; }
	/// Get (first) word id  (for inverted index queries)
    uint64 GetWordId() const { return WordIdV[0]; }
	/// Get index key and all values (for inverted index queries)
    void GetKeyWordV(TKeyWordV& KeyWordPrV) const;
	/// Get location (for location queries)
	const TFltPr& GetLoc() const { return Loc; }
	/// Check if location query has radios (for location queries)
	bool IsLocRadius() const { return (LocRadius > 0.0); }
	/// Get location query radios (for location queries)
	double GetLocRadius() const { return LocRadius; }
	/// Get location query maximal number of neighbors (for location queries)
	int GetLocLimit() const { return LocLimit; }
	/// Get comparison type
	TQueryCmpType GetCmpType() const { return CmpType; }
	/// Check comparison type
	bool IsEqual() const { return (CmpType == oqctEqual); }
	/// Check comparison type
    bool IsGreater() const { return (CmpType == oqctGreater); }
	/// Check comparison type
    bool IsLess() const { return (CmpType == oqctLess); }
	/// Check comparison type
    bool IsNotEqual() const { return (CmpType == oqctNotEqual); }
	/// Check comparison type
	bool IsWildChar() const { return (CmpType == oqctWildChar); }

	/// Are there subordinate items
	bool IsItems() const { return !ItemV.Empty(); }
	/// Get number of subordinate items
	int GetItems() const { return ItemV.Len(); }
	/// Get ItemN-th subordinate item
	const TQueryItem& GetItem(const int& ItemN) const { return ItemV[ItemN]; }
	/// Get join ID
	int GetJoinId() const { return JoinId; }
	/// Get join sample size
	int GetSampleSize() const { return SampleSize; }
	/// Get query item record
	const TRec& GetRec() const { return Rec; }
	/// Get query item record set
	const PRecSet& GetRecSet() const { return RecSet; }
	/// Access to store parameters
	uint GetStoreId() const { return StoreId; }

	friend class TQuery;
};

///////////////////////////////
/// Aggregate Query Item
class TQueryAggr {
private:
	/// Aggregate name
	TStr AggrNm;
	/// Aggregate type
	TStr AggrType;
	/// Aggregate query parameters (free form, as json)
	PJsonVal ParamVal;

public:
	TQueryAggr()  { }
	/// Create new aggregate query
	TQueryAggr(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const PJsonVal& AggrVal);
	/// Load aggregate query from input stream
	TQueryAggr(TSIn& SIn);
	/// Serialize aggregate query to output stream
	void Save(TSOut& SOut) const;

	/// Get query name
	const TStr& GetNm() const { return AggrNm; }
	/// Get query type
	const TStr& GetType() const { return AggrType; }
	/// Get query parameters as JSon object
	const PJsonVal& GetParamVal() const { return ParamVal; }

	/// Load one or more aggregate queries from given JSon query
	/// @param QueryAggrV Vector to store resulting aggregate query items
	static void LoadJson(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
		const PJsonVal& AggrVal, TVec<TQueryAggr>& QueryAggrV);
};
typedef TVec<TQueryAggr> TQueryAggrV;

///////////////////////////////
/// Query
class TQuery {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TQuery>;

	/// Query definition
	TQueryItem QueryItem;
	/// Aggregate query definition
	TQueryAggrV QueryAggrV;

	/// Field according which to sort after (-1 for no sort)
	TInt SortFieldId;
	/// In which order to sort (true for ascending)
	TBool SortAscP;
	/// Limit number of records to return in the query
	TInt Limit;
	/// Return only records after (and including the) Offset-th record
	TInt Offset;

	TQuery(const TWPt<TBase>& Base, const TQueryItem& _QueryItem, const int& _SortFieldId, 
		const bool& _SortAscP, const int& _Limit, const int& _Offset);
public:
	/// Create query around given QueryItem
	static TPt<TQuery> New(const TWPt<TBase>& Base, const TQueryItem& QueryItem, 
		const int& SortFieldId = -1, const bool& SortAscP = true,
		const int& Limit = -1, const int& Offset = 0);
	/// Create query from parsing JSon object
	static TPt<TQuery> New(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
	/// Create query from parsing JSon object as a string
	static TPt<TQuery> New(const TWPt<TBase>& Base, const TStr& TStr);

	/// Is query well-defined
	bool IsDef() const { return QueryItem.IsDef(); }
	/// Does query have any item
	bool Empty() const { return QueryItem.Empty(); }
	/// Are results weighted by default?
	bool IsWgt() const { return QueryItem.IsWgt(); }
	/// Get the result store
	TWPt<TStore> GetStore(const TWPt<TBase>& Base);
	/// Is there any sorting specified
	bool IsSort() const { return SortFieldId != -1; }
	/// Do the sort
	void Sort(const TWPt<TBase>& Base, const PRecSet& RecSet);
	/// Is there any limit restriction
	bool IsLimit() const { return (Limit != -1) || (Offset != 0); }
	/// Do the range limit, when specified
	PRecSet GetLimit(const PRecSet& RecSet);
	
	/// Check if query is valid
	bool IsOk(const TWPt<TBase>& Base, TStr& MsgStr) const;

	/// Access to the query definition
	const TQueryItem& GetQueryItem() const { return QueryItem; }
	/// Access to aggregate query definitions
	const TQueryAggrV& GetAggrItemV() const { return QueryAggrV; }
};
typedef TPt<TQuery> PQuery;

///////////////////////////////
// GeoIndex
//   Implemented in core.cpp, to avoid external dependancy on sphere.h
class TGeoIndex; typedef TPt<TGeoIndex> PGeoIndex;

///////////////////////////////
/// Index
class TIndex {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TIndex>;
public:
    // gix template definitions
	typedef TKeyWord TQmGixKey; // (KeyId, WordId)
	typedef TKeyDat<TUInt64, TInt> TQmGixItem; // [RecId, Freq]
	typedef TVec<TQmGixItem> TQmGixItemV;
	typedef TPt<TGixMerger<TQmGixKey, TQmGixItem> > PQmGixMerger;
	typedef TPt<TGixKeyStr<TQmGixKey> > PQmGixKeyStr;
	typedef TGixItemSet<TQmGixKey, TQmGixItem> TQmGixItemSet;
	typedef TPt<TQmGixItemSet> PQmGixItemSet;
	typedef TGix<TQmGixKey, TQmGixItem> TQmGix;
	typedef TPt<TQmGix> PQmGix;
	typedef TGixExpItem<TQmGixKey, TQmGixItem> TQmGixExpItem;
	typedef TPt<TQmGixExpItem> PQmGixExpItem;

    /// Merger which sums up the frequencies
	class TQmGixDefMerger : public TGixMerger<TQmGixKey, TQmGixItem> {
	public:
		static PGixMerger New() { return new TQmGixDefMerger(); }

		void Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
		void Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
		void Minus(const TQmGixItemV& MainV, const TQmGixItemV& JoinV, TQmGixItemV& ResV) const;
		void Merge(TQmGixItemV& ItemV) const;
		void Def(const TQmGixKey& Key, TQmGixItemV& MainV) const  { }
	};

	/// Merger which sums the frequencies but removes the duplicates (e.g. 3+1 = 1+1 = 2)
	class TQmGixRmDupMerger : public TQmGixDefMerger {
	public:
		static PGixMerger New() { return new TQmGixRmDupMerger(); }

		void Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
		void Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
	};

	/// Giving pretty names to GIX keys when printing debug statistics
	class TQmGixKeyStr : public TGixKeyStr<TQmGixKey> {
	private:
		TWPt<TBase> Base;
		TWPt<TIndexVoc> IndexVoc;

		TQmGixKeyStr(const TWPt<TBase>& _Base, const TWPt<TIndexVoc>& _IndexVoc);
	public:
		static PQmGixKeyStr New(const TWPt<TBase>& Base, const TWPt<TIndexVoc>& IndexVoc) {
			return new TQmGixKeyStr(Base, IndexVoc); }

		TStr GetKeyNm(const TQmGixKey& Key) const;
	};

private:    
	/// Remember index location
    TStr IndexFPath;
    /// Remember access mode to the index
    TFAccess Access;
    /// Inverted index
    mutable PQmGix Gix;
	/// Location index
	THash<TInt, PGeoIndex> GeoIndexH;
    /// Index Vocabulary
    PIndexVoc IndexVoc;
	/// Inverted Index Default Merger
	PQmGixMerger DefMerger;

    /// Converts query item tree to GIX query expression
	PQmGixExpItem ToExpItem(const TQueryItem& QueryItem) const;
    /// Executes GIX query expression against the index
    bool DoQuery(const PQmGixExpItem& ExpItem, const PQmGixMerger& Merger, 
		TQmGixItemV& RecIdFqV) const;

    TIndex(const TStr& _IndexFPath, const TFAccess& _Access, 
        const PIndexVoc& IndexVoc, const int64& CacheSize);
public:
	/// Create (Access==faCreate) or open existing index
    static PIndex New(const TStr& IndexFPath, const TFAccess& Access, 
        const PIndexVoc& IndexVoc, const int64& CacheSize) {
            return new TIndex(IndexFPath, Access, IndexVoc, CacheSize); }
	/// Checks if there is an existing index at the given path
	static bool Exists(const TStr& IndexFPath) {
		return TFile::Exists(IndexFPath + "Index.Gix"); }
    
	/// Close the query
	~TIndex();

	/// Get index location
	TStr GetIndexFPath() const { return IndexFPath; }
	/// Get index cache size
	uint64 GetIndexCacheSize() const { return Gix->GetMxCacheSize(); }
    /// Get index vocabulary
    TWPt<TIndexVoc> GetIndexVoc() const { return IndexVoc; }
	/// Get default index merger
	PQmGixMerger GetDefMerger() const { return DefMerger; }

    /// Index RecId under (Key, Word)
    void Index(const int& KeyId, const uint64& WordId, const uint64& RecId);
	/// Index RecId under (Key, Word). WordStr is sent through index vocabulary.
    void Index(const int& KeyId, const TStr& WordStr, const uint64& RecId);
	/// Index RecId under (Key, Word). WordStrV is sent through index vocabulary.
	/// Repeated words have associated weight based on their count.
    void Index(const int& KeyId, const TStrV& WordStrV, const uint64& RecId);
	/// Index RecId under (Key, Word). WordStrV is sent through index vocabulary.
	/// Each word indexed given the weight (frequency) from the input.
	void Index(const int& KeyId, const TStrIntPrV& WordStrFqV, const uint64& RecId);	
	/// Index RecId under (Key, Word). WordStr is sent through index vocabulary.
	/// Repeated words have associated weight based on their count, in case of text keys.
    void Index(const uint& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId);
	/// Index RecId under (Key, Word). WordStrV is sent through index vocabulary.
    void Index(const uint& StoreId, const TStr& KeyNm, const TStrV& WordStrV, const uint64& RecId);
	/// Index RecId under (Key, Word). WordStrV is sent through index vocabulary.
	/// Each word indexed given the weight (frequency) from the input.
	void Index(const uint& StoreId, const TStr& KeyNm, const TStrIntPrV& WordStrFqV, const uint64& RecId);
	/// Index RecId under (Key, Word). Each word is sent through index vocabulary
	void Index(const uint& StoreId, const TStrPrV& KeyWordV, const uint64& RecId);
	/// Index RecId under given Key. Tokenize and clean given free text to derive words.
	void IndexText(const int& KeyId, const TStr& TextStr, const uint64& RecId);
	/// Index RecId under given Key. Tokenize and clean given free text to derive words.
	void IndexText(const uint& StoreId, const TStr& KeyNm, const TStr& TextStr, const uint64& RecId);
	/// Index RecId under given Key. Tokenize and clean given free text to derive words.
	void IndexText(const int& KeyId, const TStrV& TextStrV, const uint64& RecId);
	/// Index RecId under given Key. Tokenize and clean given free text to derive words.
	void IndexText(const uint& StoreId, const TStr& KeyNm, const TStrV& TextStrV, const uint64& RecId);
	/// Index a join between RecId and JoinRecId
	void IndexJoin(const TWPt<TStore>& Store, const int& JoinId, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = 1);
	/// Index a join between RecId and JoinRecId
	void IndexJoin(const TWPt<TStore>& Store, const TStr& JoinNm, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = 1);
    /// Add to inverted index (RecId, RecFq) under key (KeyId, WordId).
    void Index(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq);

    /// Delete index for RecId under (Key, Word). WordStr is sent through index vocabulary.
	void Delete(const int& KeyId, const TStr& WordStr, const uint64& RecId);
	/// Delete index for RecId under (Key, Word). WordStrV is sent through index vocabulary.
	/// Repeated words have associated weight based on their count.
	void Delete(const int& KeyId, const TStrV& WordStrV, const uint64& RecId);
	/// Delete index for RecId under (Key, Word). WordStr is sent through index vocabulary.
	void Delete(const uint& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId);
    /// Delete index for RecId under (Key, Word)
	void Delete(const uint& StoreId, const TStr& KeyNm, const uint64& WordId, const uint64& RecId);
	/// Delete index for RecId under (Key, Word). Each word is sent through index vocabulary
	void Delete(const uint& StoreId, const TStrPrV& KeyWordV, const uint64& RecId);
	/// Delete index for RecId under given Key. Tokenize and clean given free text to derive words.
	void DeleteText(const int& KeyId, const TStr& TextStr, const uint64& RecId);
	/// Delete index for RecId under given Key. Tokenize and clean given free text to derive words.
	void DeleteText(const uint& StoreId, const TStr& KeyNm, const TStr& TextStr, const uint64& RecId);
	/// Delete index for RecId under given Key. Tokenize and clean given free text to derive words.
	void DeleteText(const int& KeyId, const TStrV& TextStrV, const uint64& RecId);
	/// Delete index for RecId under given Key. Tokenize and clean given free text to derive words.
	void DeleteText(const uint& StoreId, const TStr& KeyNm, const TStrV& TextStrV, const uint64& RecId);
	// Remove join from index
	void DeleteJoin(const TWPt<TStore>& Store, const int& JoinId, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq);
	// Remove join from index
	void DeleteJoin(const TWPt<TStore>& Store, const TStr& JoinNm, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq);
	// Delete record from inverted index
	void Delete(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq);

	/// Add RecId to location index under key (Key, Loc)
    void Index(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc, const uint64& RecId);
	/// Add RecId to location index under key (Key, Loc)
	void Index(const int& KeyId, const TFltPr& Loc, const uint64& RecId);
	/// Delete RecId from location index under (Key, Loc)
	void Delete(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc, const uint64& RecId);
	/// Delete RecId from location index under (Key, Loc)
	void Delete(const int& KeyId, const TFltPr& Loc, const uint64& RecId);
	/// Checks if two locations point to the same place
	bool LocEquals(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc1, const TFltPr& Loc2) const;
	/// Checks if two locations point to the same place
	bool LocEquals(const int& KeyId, const TFltPr& Loc1, const TFltPr& Loc2) const;

    /// Check if the index is taking all the available cache space
    bool IsCacheFull() const { return Gix->IsCacheFull(); }
	/// Check if index opened in read-only mode
	bool IsReadOnly() const { return Access == faRdOnly; }
    /// Merge with another index
    void MergeIndex(const TWPt<TIndex>& TmpIndex);

	/// Do flat AND search, given the vector of inverted index queries
	void SearchAnd(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const;
	/// Do flat OR search, given the vector of inverted index queries
	void SearchOr(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const;
	/// Search with special Merger (does not handle joins)
	TPair<TBool, PRecSet> Search(const TWPt<TBase>& Base, const TQueryItem& QueryItem, const PQmGixMerger& Merger) const;
	/// Do geo-location range (in meters) search
	PRecSet SearchRange(const TWPt<TBase>& Base, const int& KeyId, 
        const TFltPr& Loc, const double& Radius, const int& Limit) const;
	/// Do geo-location nearest-neighbor search
	PRecSet SearchNn(const TWPt<TBase>& Base, const int& KeyId, 
        const TFltPr& Loc, const int& Limit) const;
	/// Get records ids and counts that are joined with given RecId (via given join key)
	void GetJoinRecIdFqV(const int& JoinKeyId, const uint64& RecId, TUInt64IntKdV& JoinRecIdFqV) const;

	/// Save debug statistics to a file
	void SaveTxt(const TWPt<TBase>& Base, const TStr& FNm);
};

///////////////////////////////
/// Temporary Index.
/// Useful for fast batch indexing, where we create a new index each time we fill
/// the given cache space. At the end all the indices are merged together.
class TTempIndex {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TTempIndex>;

	/// Maximal size of temporary index
	int64 IndexCacheSize;
	/// Location of the temporary index
	TStr TempFPath;
	/// List of previous temporary index locations, used at final merging into the main index
	TStrQ TempIndexFPathQ;
	/// Current temporary index
	PIndex TempIndex;

	UndefDefaultCopyAssign(TTempIndex);
	TTempIndex(const TStr& _TempFPath, const int64& _IndexCacheSize): 
		 IndexCacheSize(_IndexCacheSize), TempFPath(_TempFPath) { }
public:
	/// Create new empty temporary index
	static TPt<TTempIndex> New(const TStr& TempFPath, const int64& IndexCacheSize) { 
		return new TTempIndex(TempFPath, IndexCacheSize); }

	/// Is the temporary index full
	bool IsIndexFull() const { return TempIndex->IsCacheFull(); }
	/// Returns index to which new items can be added
    TWPt<TIndex> GetIndex() const { return TempIndex; }
	/// Initialize new empty temporary index
	void NewIndex(const PIndexVoc& IndexVoc);
	/// Merge all temporary indices with the given main index
	void Merge(const TWPt<TIndex>& Index);
};
typedef TPt<TTempIndex> PTempIndex;

///////////////////////////////
/// Operator. 
/// Abstraction for functions working with record sets. 
///   Input: zero or more record sets, parameters as JSon object
///   Output: one or more record sets.
/// Operators returning exactly one record set are called "Functional".
class TOp {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TOp>;
	/// Operator name
	const TStr OpNm;

protected:
	/// Create new operator with a given name. Name is validated against naming constraints.
	TOp(const TStr& _OpNm);
public:	
	virtual ~TOp() { }

	/// Get operator name
	TStr GetOpNm() const { return OpNm; }
    
	/// Execute the operator
	/// @param InRecSetV   Input record sets, can be empty
	/// @param ParamVal    Operator parameters
	/// @param OutRecSetV  Output record sets, should not be empty.
	virtual void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV) = 0;
	/// True when operator always returns exactly one record set
	virtual bool IsFunctional() = 0;
	/// Wrapper for easier calling of functional operators.
	PRecSet Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal);
};

///////////////////////////////
/// Aggregator.
/// Computes and holds statistics from a given record set.
/// Works in batch mode: record set in, wait, aggregate out.
class TAggr {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TAggr>;

	/// New constructor delegate
	typedef PAggr (*TNewF)(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PJsonVal& ParamVal);
    /// Stream aggregate descriptions
	static TFunRouter<PAggr, TNewF> NewRouter;   
public:
    /// Register default aggregates
    static void Init();
    /// Register new aggregate
    template <class TObj> static void Register() { 
        NewRouter.Register(TObj::GetType(), TObj::New);
    }

private:
    /// QMiner Base pointer
    TWPt<TBase> Base;    
	/// Aggreagte name
	const TStr AggrNm;

protected:
	TAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm);
    
    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }
public:
	/// Create new aggregate of a given type.
	/// @param RecSet    Record collection on which to compute the aggregates
	/// @param QueryAggr Aggregate query details (e.g. type, parameters)
	static PAggr New(const TWPt<TBase>& Base, const PRecSet& RecSet, const TQueryAggr& QueryAggr); 
	virtual ~TAggr() { }

	/// Get aggreagte name
	const TStr& GetAggrNm() const { return AggrNm; }
	/// Serialize aggregate to readable JSon object
	virtual PJsonVal SaveJson() const = 0;
};

///////////////////////////////
/// Stream Aggregator.
/// Computes and holds statistics from a record stream.
class TStreamAggr {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TStreamAggr>;

private:
	/// New constructor delegate
	typedef PStreamAggr (*TNewF)(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    /// Stream aggregate New constructor router
	static TFunRouter<PStreamAggr, TNewF> NewRouter;   
    /// Load constructor delegate
	typedef PStreamAggr(*TLoadF)(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
    /// Stream aggregate Load constructor router
	static TFunRouter<PStreamAggr, TLoadF> LoadRouter;
public:
    /// Register default stream aggregates
    static void Init();
    /// Register new stream aggregate
    template <class TObj> static void Register() { 
        NewRouter.Register(TObj::GetType(), TObj::New);
        LoadRouter.Register(TObj::GetType(), TObj::Load);
    }
    
private:
    /// QMiner Base pointer
    TWPt<TBase> Base;
	/// Stream aggreagte name
	const TStr AggrNm;

	/// Each trigger has a unique internal ID
	TStr Guid;
protected:
	/// Create new stream aggregate
	TStreamAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm);
    /// Create new stream aggregate from JSon parameters
	TStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);       
	/// Load basic class of stream aggregate
	TStreamAggr(const TWPt<TBase>& _Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	
    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }
public:
	/// Create new stream aggregate based on provided JSon parameters
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    /// Virtual destructor!
	virtual ~TStreamAggr() { }
    
	/// Load stream aggregate from stream
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	virtual void _Load(TSIn& SIn) { throw TQmExcept::New("TStreamAggr::_Load not implemented:" + GetAggrNm()); };
	/// Save basic class of stream aggregate to stream
	virtual void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	virtual void _Save(TSOut& SOut) const { throw TQmExcept::New("TStreamAggr::_Save not implemented:" + GetAggrNm()); };

	/// Get aggregate name
	const TStr& GetAggrNm() const { return AggrNm; }
	/// Is the aggregate initialized. Used for aggregates, which require some time to get started.
	virtual bool IsInit() const { return true; }

	/// Add new record to aggregate
	virtual void OnAddRec(const TRec& Rec) = 0;
	/// Recored already added to the aggregate is being updated
	virtual void OnUpdateRec(const TRec& Rec) { }
	/// Recored already added to the aggregate is being deleted from the store 
	virtual void OnDeleteRec(const TRec& Rec) { }

    // retrieving input aggregate names
    virtual void GetInAggrNmV(TStrV& InAggrNmV) const { };

	/// Print latest statistics to logger
	virtual void PrintStat() const { }
	/// Serialization current status to JSon
	virtual PJsonVal SaveJson(const int& Limit) const = 0;
    
	/// Unique ID of the stream aggregate
	const TStr& GetGuid() const { return Guid; }  

	virtual TStr Type() const = 0;
};

///////////////////////////////
// QMiner-Stream-Aggregator-Data-Interfaces
namespace TStreamAggrOut {
	class IInt {
	public:
		// retireving value from the aggregate
		virtual int GetInt() const = 0;
	};

	class IFlt {
	public:
		// retireving value from the aggregate
		virtual double GetFlt() const = 0;
	};

	class ITm {
	public:
		// retireving value from the aggregate
		virtual uint64 GetTmMSecs() const = 0;
	};
    
    // combination of numeric value and timestamp
    class IFltTm: public IFlt, public ITm { };
    
    class IFltTmIO {
    public:
        virtual double GetInFlt() const = 0;
        virtual uint64 GetInTmMSecs() const = 0;
		virtual void GetOutFltV(TFltV& ValV) const = 0;
        virtual void GetOutTmMSecsV(TUInt64V& MSecsV) const = 0;
        virtual int GetN() const = 0;
    };

	class IFltVec {
	public:
		// retrieving vector of values from the aggregate
		virtual int GetFltLen() const = 0;
		virtual double GetFlt(const TInt& ElN) const = 0;
		virtual void GetFltV(TFltV& ValV) const = 0;
	};

	class ITmVec {
	public:
		// retrieving vector of timestamps from the aggregate
		virtual int GetTmLen() const = 0;
		virtual uint64 GetTm(const TInt& ElN) const = 0;
		virtual void GetTmV(TUInt64V& MSecsV) const = 0;
	};

	class IFltVecTm : public IFltVec, public ITm { };

	class INmFlt {
	public:
		// retrieving named values
		virtual bool IsNmFlt(const TStr& Nm) const = 0;
		virtual double GetNmFlt(const TStr& Nm) const = 0;
		virtual void GetNmFltV(TStrFltPrV& NmFltV) const = 0;
	};

	class INmInt {
	public:
		// retrieving named values
		virtual bool IsNm(const TStr& Nm) const = 0;
		virtual double GetNmInt(const TStr& Nm) const = 0;
		virtual void GetNmIntV(TStrIntPrV& NmIntV) const = 0;
	};
}
///////////////////////////////
// QMiner-Stream-Aggregator-Base
class TStreamAggrBase {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TStreamAggrBase>;

	// stream aggregates
	THash<TStr, PStreamAggr> StreamAggrH;

	// create emptyp base
	TStreamAggrBase() { }
	// serialization
	TStreamAggrBase(const TWPt<TBase>& Base, TSIn& SIn);
public:
	static PStreamAggrBase New();
	static PStreamAggrBase Load(const TWPt<TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;

	// managament
	bool Empty() const;
	int Len() const;
	bool IsStreamAggr(const TStr& StreamAggrNm) const;	
	const PStreamAggr& GetStreamAggr(const TStr& StreamAggrNm) const;
	const PStreamAggr& GetStreamAggr(const int& StreamAggrId) const;
	void AddStreamAggr(const PStreamAggr& StreamAggr);
	int GetFirstStreamAggrId() const;
	bool GetNextStreamAggrId(int& AggrId) const;
	
	// forward the calls to stream aggregates
    void OnAddRec(const TRec& Rec);
    void OnUpdateRec(const TRec& Rec);
	void OnDeleteRec(const TRec& Rec);
	
	// serialize the base into json
	PJsonVal SaveJson(const int& Limit) const;
};

///////////////////////////////
// QMiner-Stream-Aggregator-Trigger
class TStreamAggrTrigger : public TStoreTrigger {
private:
	PStreamAggrBase StreamAggrBase;
	
	TStreamAggrTrigger(const PStreamAggrBase& _StreamAggrBase);
public:
	static PStoreTrigger New(const PStreamAggrBase& StreamAggrBase);

	// forward the calls to stream aggreagte base
    void OnAdd(const TRec& Rec);
    void OnUpdate(const TRec& Rec);
	void OnDelete(const TRec& Rec);
};

///////////////////////////////
// QMiner-Base
class TBase {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TBase>;
	
	// is store initialized
	TBool InitP;

	// location
    TStr FPath;
	// access type
    TFAccess FAccess;
	// is temporary folder defined
	TBool TempFPathP;
	// default temporary directory
	TStr TempFPath;

	// index
    PIndexVoc IndexVoc;
    PIndex Index;
    // stores
	TVec<PStore> StoreV;
    THash<TStr, PStore> StoreH;
	// stream aggregate base for each store
    TVec<PStreamAggrBase> StreamAggrBaseV;
	// default stream aggregate base (store independent)
	PStreamAggrBase StreamAggrDefaultBase;
	// operators
	THash<TStr, POp> OpH;

	// temporary indices
	PTempIndex TempIndex;

private:
    TBase(const TStr& _FPath, const int64& IndexCacheSize);
    TBase(const TStr& _FPath, const TFAccess& _FAccess, const int64& IndexCacheSize);
public:
	~TBase();
private:
    // serialization
    void SaveStreamAggrBaseV(TSOut& SOut);
    void LoadStreamAggrBaseV(TSIn& SIn);
    
	// searching
	PRecSet Invert(const PRecSet& RecSet, const TIndex::PQmGixMerger& Merger);
	TPair<TBool, PRecSet> Search(const TQueryItem& QueryItem, const TIndex::PQmGixMerger& Merger);

public:
	static PBase New(const TStr& FPath, const int64& IndexCacheSize) { 
		return new TBase(FPath, IndexCacheSize); }
	static PBase Load(const TStr& FPath, const TFAccess& FAccess, const int64& IndexCacheSize) {
		return new TBase(FPath, FAccess, IndexCacheSize); }

	// check if base already exists
	static bool Exists(const TStr& FPath);

	// initializing base
	bool IsInit() const { return InitP; }
	void Init();
	void Init(const TStr& TempFPath) { PutTempFPath(TempFPath); Init(); }

	// base disk location and access type
	const TStr& GetFPath() const { return FPath; }
	bool IsRdOnly() const { return FAccess == faRdOnly; }
	const TFAccess& GetFAccess() const { return FAccess; }

	// index
    TWPt<TIndexVoc> GetIndexVoc() const { return IndexVoc; }
	TWPt<TIndex> GetIndex() const;

	// stores
    void AddStore(const PStore& NewStore);
    int GetStores() const { return StoreH.Len(); }
    bool IsStoreN(const uint& StoreN) const { return StoreN < (uint)StoreH.Len(); }
    const TWPt<TStore> GetStoreByStoreN(const int& StoreN) const;
    bool IsStoreId(const uint& StoreId) const { return !StoreV[StoreId].Empty(); }
    const TWPt<TStore> GetStoreByStoreId(const uint& StoreId) const;
    bool IsStoreNm(const TStr& StoreNm) const { return StoreH.IsKey(StoreNm); }
	const TWPt<TStore> GetStoreByStoreNm(const TStr& StoreNm) const;
	/// Helper function for returning JSon definition of store
    PJsonVal GetStoreJson(const TWPt<TStore>& Store);

	// stream aggregates
	const PStreamAggrBase& GetStreamAggrBase(const uint& StoreId) const;
	const PStreamAggrBase& GetStreamAggrBase() const;
	bool IsStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const;
	bool IsStreamAggr(const TStr& StreamAggrNm) const;
	const PStreamAggr& GetStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const;
	const PStreamAggr& GetStreamAggr(const TStr& StoreNm, const TStr& StreamAggrNm) const;
	const PStreamAggr& GetStreamAggr(const TStr& StreamAggrNm) const;
	void AddStreamAggr(const uint& StoreId, const PStreamAggr& StreamAggr);
	void AddStreamAggr(const TUIntV& StoreIdV, const PStreamAggr& StreamAggr);
	void AddStreamAggr(const TStr& StoreNm, const PStreamAggr& StreamAggr);
	void AddStreamAggr(const TStrV& StoreNmV, const PStreamAggr& StreamAggr);
	void AddStreamAggr(const PStreamAggr& StreamAggr);
    // aggregate records
	void Aggr(PRecSet& RecSet, const TQueryAggrV& QueryAggrV);

	// operators
	void AddOp(const POp& NewOp);
	int GetOps() const { return OpH.Len(); }
	int GetFirstOpId() const { return OpH.FFirstKeyId(); }
	bool GetNextOpId(int& OpId) const { return OpH.FNextKeyId(OpId); }
	const POp& GetOp(const int& OpId) const { return OpH[OpId]; }
	bool IsOp(const TStr& OpNm) const { return OpH.IsKey(OpNm); }
	const POp& GetOp(const TStr& OpNm) const { return OpH.GetDat(OpNm); }
	// execute operator
	void Operator(const TRecSetV& InRecSetV, const PJsonVal& ParamVal, TRecSetV& OutRecSetV);	

	// create new word vocabulary and returns its id
	int NewIndexWordVoc(const TIndexKeyType& Type, const TStr& WordVocNm = TStr());
	// creates index key, without linking it to a filed, returns the id of created key
	int NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const TIndexKeyType& Type = oiktValue,
		const TIndexKeySortType& SortType = oikstUndef);
    // creates index key, without linking it to a filed using specified vocabulary,
	// returns the id of created key
	int NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& WordVocId, 
		const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    // create index key for a specified (store, field) pair, returns the id of created key
	int NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId,
		const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId, 
		const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    // create index key for a specified (store, field) pair using specified vocabulary,
	// returns the id of created key
	int NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId, const int& WordVocId, 
		const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId, 
		const int& WordVocId, const TIndexKeyType& Type = oiktValue, 
		const TIndexKeySortType& SortType = oikstUndef);

    /// Add new record to a give store
	uint64 AddRec(const TWPt<TStore>& Store, const PJsonVal& RecVal);
    /// Add new record to a give store
	uint64 AddRec(const TStr& StoreNm, const PJsonVal& RecVal);
    /// Add new record to a give store
	uint64 AddRec(const uint& StoreId, const PJsonVal& RecVal);
    
    // searching records (default search interface)
	PRecSet Search(const PQuery& Query);
	PRecSet Search(const TQueryItem& QueryItem);
	PRecSet Search(const TStr& QueryStr);
	PRecSet Search(const PJsonVal& QueryVal);
    
    /// Execute garbage collection on all stores
    void GarbageCollect();    

    // is temporary folder defined
	bool IsTempFPath() const { return TempFPathP; }
	// get temporary folder
	TStr GetTempFPath() const { return TempFPath; }
	// set temporary folder
	void PutTempFPath(const TStr& _TempFPath) { TempFPathP = true; TempFPath = _TempFPath; }

    // temporary index (useful at batch processing)
  	bool IsTempIndex() const { return !TempIndex.Empty(); }
	void InitTempIndex(const uint64& IndexCacheSize);
	void MergeTempIndex() { TempIndex->Merge(Index); TempIndex.Clr(); }
	bool IsTempIndexFull() const { return TempIndex->IsIndexFull(); }
	void NewTempIndex() const { TempIndex->NewIndex(IndexVoc); }
	void CheckTempIndexSize() { if (IsTempIndexFull()) { NewTempIndex(); } }

    // JSON dump and load
	bool SaveJSonDump(const TStr& DumpDir);
	bool RestoreJSonDump(const TStr& DumpDir);
    
    // statistics
    void PrintStores(const TStr& FNm, const bool& FullP = false);
	void PrintIndexVoc(const TStr& FNm);
	void PrintIndex(const TStr& FNm, const bool& SortP);
};

} // namespace

#endif
