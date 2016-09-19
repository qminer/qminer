/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef QMINER_CORE_H
#define QMINER_CORE_H

#include <base.h>
#include <mine.h>

namespace TQm {

///////////////////////////////
// Forward Declarations
class TBase; typedef TPt<TBase> PBase;
class TStore; typedef TPt<TStore> PStore;
class TRec;
class TRecSet; typedef TPt<TRecSet> PRecSet;
class TIndexVoc; typedef TPt<TIndexVoc> PIndexVoc;
class TIndex; typedef TPt<TIndex> PIndex;
class TAggr; typedef TPt<TAggr> PAggr;
class TStreamAggr; typedef TPt<TStreamAggr> PStreamAggr;
class TRecFilter; typedef TPt<TRecFilter> PRecFilter;
class TFtrExt; typedef TPt<TFtrExt> PFtrExt;
class TFtrSpace; typedef TPt<TFtrSpace> PFtrSpace;

///////////////////////////////
/// QMiner Environment.
class TEnv {
private:
    static bool InitP;
public:
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
    /// @param FPath        Specify logger output (standard output `std'; No output `null')
    /// @param TimestampP   Show timestamp with each status
    static void InitLogger(const int& _Verbosity = 1, const TStr& FPath = TStr(), const bool& TimestampP = false);
    
    /// Set return code
    static void SetReturnCode(const int& _ReturnCode) { ReturnCode = _ReturnCode; }
};

void ErrorLog(const TStr& MsgStr);
void InfoLog(const TStr& MsgStr);
void DebugLog(const TStr& MsgStr);

///////////////////////////////
/// QMiner Exception
class TQmExcept : public TExcept {
private:
    TQmExcept(const TStr& MsgStr, const TStr& LocStr): TExcept(MsgStr, LocStr) { }

    // private to catch old calls to this function
    static PExcept Throw(const TStr& MsgStr, const TStr& LocStr = TStr()) { return NULL; }
public:
    /// Create new QMiner exception
    static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr());
};

#define QmAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : throw TQm::TQmExcept::New(TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

#define QmAssertR(Cond, MsgStr) \
  ((Cond) ? static_cast<void>(0) : throw TQm::TQmExcept::New(MsgStr, TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

///////////////////////////////
/// QMiner Valid Name Enforcer.
class TNmValidator {
private:
    /// Valid non-alpha-numeric first character of store/field/join/key name
    static TChA ValidFirstCh;
    /// Valid non-alpha-numeric non-first character of store/field/join/key name
    static TChA ValidCh;
    /// is set to true, all field names will be valid
    TBool StrictNmP;

    /// Check if given char is valid and safe begining for JS variable
    static bool IsValidJsFirstCharacter(const TStr& NmStr);
    /// Check if given char is valid and safe for JS variable
    static bool IsValidJsCharacter(const char& Ch);
public:
    /// When StrictNmP = false, then all names go, else only one that play well in JavaScript
    TNmValidator(const bool& _StrictNmP): StrictNmP(_StrictNmP) {}
    /// Load from stream
    TNmValidator(TSIn& SIn): StrictNmP(SIn) {}

    /// Save to stream
    void Save(TSOut& SOut) const;

    /// Validate if given string is a valid store/field/join/key name.
    /// Valid names begin with alphabetical or ValidFirstCh and continues with
    /// any alphanumerical or ValidCh character (e.g. '_ThisIsValid_123_Name').
    void AssertValidNm(const TStr& NmStr) const;
    /// when set to false, all field names will be allowed
    void SetStrictNmP(const bool& _StrictNmP);
    /// check if we enforce strict names
    bool IsStrictNmP() const { return StrictNmP; }
};

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
    TJoinDesc(const TWPt<TBase>& Base, const TStr& _JoinNm, const uint& _JoinStoreId,
        const uint& StoreId, const TWPt<TIndexVoc>& IndexVoc, const bool& IsSmall);
    /// Create a field based join (1-1)
    TJoinDesc(const TWPt<TBase>& Base, const TStr& _JoinNm, const uint& _JoinStoreId, const int& _JoinRecFieldId,
        const int& _JoinFqFieldId);

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
    oftByte     = 17,///< Unsigned 8-bit integer
    oftInt      = 0, ///< 32-bit integer
    oftInt16    = 15,///< 16-bit integer
    oftInt64    = 16,///< 64-bit integer
    oftIntV     = 9, ///< Vector of integers
    oftUInt     = 13,///< Unsigned 32bit integer
    oftUInt16   = 14,///< Unsigned 16bit integer
    oftUInt64   = 8, ///< Unsigned 64bit integer
    oftStr      = 1, ///< String
    oftStrV     = 2, ///< Vector of strings
    oftBool     = 4, ///< Boolean
    oftFlt      = 5, ///< Double precision number
    oftSFlt     = 18,///< Single precision number
    oftFltPr    = 6, ///< Pair of double precision numbers, useful for storing geographic coordinates
    oftFltV     = 10,///< Vector of double precision numbers
    oftTm       = 7, ///< Date and time
    oftNumSpV   = 11,///< Sparse vector -- vector of (integer,double) pairs 
    oftBowSpV   = 12, ///< Bag-of-words sparse vector
    oftTMem     = 19, ///< Memory buffer
    oftJson     = 20  ///< JSON field
} TFieldType;

///////////////////////////////
/// Field Description
class TFieldDesc {
private:
    typedef enum {
        ofdfNull = (1 << 0),
        ofdfInternal = (1 << 1),
        ofdfPrimary = (1 << 2),
        ofdfCodebook = (1 << 3)
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
    TFieldDesc(const TWPt<TBase>& Base, const TStr& _FieldNm, TFieldType _FieldType, const bool& Primary,
        const bool& NullP, const bool& InternalP, const bool& CodebookP);
    
    TFieldDesc(TSIn& SIn);
    void Save(TSOut& SOut) const;

    // basic properties
    void PutFieldId(const int& _FieldId) { FieldId = _FieldId; }
    int GetFieldId() const { return FieldId; };
    const TStr& GetFieldNm() const { return FieldNm; }

    // field data-type
    TFieldType GetFieldType() const { return FieldType; }
    TStr GetFieldTypeStr() const;
    bool IsByte() const { return FieldType == oftByte; }
    bool IsInt() const { return FieldType == oftInt; }
    bool IsInt16() const { return FieldType == oftInt16; }
    bool IsInt64() const { return FieldType == oftInt64; }
    bool IsIntV() const { return FieldType == oftIntV; }
    bool IsUInt() const { return FieldType == oftUInt; }
    bool IsUInt16() const { return FieldType == oftUInt16; }
    bool IsUInt64() const { return FieldType == oftUInt64; }
    bool IsStr() const { return FieldType == oftStr; }
    bool IsStrV() const { return FieldType == oftStrV; }
    bool IsBool() const { return FieldType == oftBool; }
    bool IsFlt() const { return FieldType == oftFlt; }
    bool IsSFlt() const { return FieldType == oftSFlt; }
    bool IsFltPr() const { return FieldType == oftFltPr; }
    bool IsFltV() const { return FieldType == oftFltV; }
    bool IsTm() const { return FieldType == oftTm; }
    bool IsNumSpV() const { return FieldType == oftNumSpV; }
    bool IsBowSpV() const { return FieldType == oftBowSpV; }
    bool IsTMem() const { return FieldType == oftTMem; }
    bool IsJson() const { return FieldType == oftJson; }

    // flags
    bool IsNullable() const { return ((Flags & ofdfNull) != 0); }
    bool IsInternal() const { return ((Flags & ofdfInternal) != 0); }
    bool IsPrimary() const { return ((Flags & ofdfPrimary) != 0); }
    bool IsCodebook() const { return ((Flags & ofdfCodebook) != 0); }

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
/// Store Hash Key Iterator.
/// Useful for stores using THash to store records
/// Example: TStrH TestH; PStoreIter TestIter = TStoreIterHashKey<TStrH>::New(TestH);
template <class THash>
class TStoreIterHashKey : public TStoreIter {
private:
    /// Reference to hash table with KeyIds corresponding to Record IDs
    const THash& Hash;
    /// Current key ID (== record ID)
    int KeyId;

    TStoreIterHashKey(const THash& _Hash) : Hash(_Hash), KeyId(Hash.FFirstKeyId()) { }
public:
    // Create new iterator from a given hash table
    static PStoreIter New(const THash& Hash) { return new TStoreIterHashKey<THash>(Hash); }

    bool Next() { return Hash.FNextKeyId(KeyId); }
    uint64 GetRecId() const { return Hash.GetKey(KeyId); }
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
    /// Store type
    TStr StoreType;
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
    /// Derived classes can set store type
    void SetStoreType(const TStr& Type) { StoreType = Type; }
public:    
    /// Access to base
    const TWPt<TBase>& GetBase() const { return Base; }
    /// Access to store type flag
    const TStr GetStoreType() const { return StoreType; }
protected:
    /// Access to index
    const TWPt<TIndex>& GetIndex() const { return Index; }
    
    /// Register new field to the store
    int AddFieldDesc(const TFieldDesc& FieldDesc);
    /// Default error when accessing wrong field-type combination
    PExcept FieldError(const int& FieldId, const TStr& TypeStr) const;

public:
    /// Should be called after record RecId added; executes OnAdd event in all registered triggers
    void OnAdd(const uint64& RecId);
    /// Should be called after record Rec added; executes OnAdd event in all registered triggers
    void OnAdd(const TRec& Rec);
    /// Should be called after record RecId updated; executes OnUpdate event in all registered triggers
    void OnUpdate(const uint64& RecId);
    /// Should be called after record Rec updated; executes OnUpdate event in all registered triggers
    void OnUpdate(const TRec& Rec);
    /// Should be called before record RecId deleted; executes OnDelete event in all registered triggers
    void OnDelete(const uint64& RecId);
    /// Should be called before record Rec deleted; executes OnDelete event in all registered triggers
    void OnDelete(const TRec& Rec);

protected:
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
    /// Get full join description for the join with the given name
    const TJoinDesc& GetJoinDesc(const TStr& JoinNm) const { return JoinDescV[GetJoinId(JoinNm)]; }
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
    /// @param SampleSize   Number of records to be sampled out
    virtual PRecSet GetRndRecs(const uint64& SampleSize);
    /// Checks if no records in the store
    bool Empty() const { return (GetRecs() == uint64(0)); }

    /// Gets the first record in the store (order defined by store implementation)
    virtual uint64 GetFirstRecId() const { throw TQmExcept::New("GetFirstRecId not implemented"); }
    /// Gets the last record in the store (order defined by store implementation)
    virtual uint64 GetLastRecId() const { throw TQmExcept::New("GetLastRecId not implemented"); };
    /// Gets forward moving iterator (order defined by store implementation)
    virtual PStoreIter ForwardIter() const { throw TQmExcept::New("ForwardIter not implemented"); };
    /// Gets backward moving iterator (order defined by store implementation)
    virtual PStoreIter BackwardIter() const { throw TQmExcept::New("BackwardIter not implemented"); };
    
    /// Does the store implement GetAllRecs?
    virtual bool HasGetAllRecs() const { return false; }
    /// Is the forward iterator implemented?
    virtual bool HasForwardIter() const { return false; }
    /// Is the backward iterator implemented?
    virtual bool HasBackwardIter() const { return false; }
    /// Is the first record  id getter implemented?
    virtual bool HasFirstRecId() const { return false; }
    /// Is the last record id getter implemented?
    virtual bool HasLastRecId() const { return false; }

    /// Add new record provided as JSon
    virtual uint64 AddRec(const PJsonVal& RecVal, const bool& TriggerEvents=true) = 0;
    /// Update existing record with updates in provided JSon
    virtual void UpdateRec(const uint64& RecId, const PJsonVal& RecVal) = 0;
    
    /// Add join
    void AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq = 1);
    void AddJoin(const TStr& JoinNm, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq = 1);
    /// Delete join
    void DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq = TInt::Mx);
    void DelJoin(const TStr& JoinNm, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq = TInt::Mx);
    /// Delete joins
    void DelJoins(const int& JoinId, const uint64& RecId);
    void DelJoins(const TStr& JoinNm, const uint64& RecId);
    
    /// Signal to purge any old stuff, e.g. records that fall out of time window when store has one
    virtual void GarbageCollect() { }
    /// Deletes all records
    virtual void DeleteAllRecs() = 0;
    /// Delete the first DelRecs records (the records that were inserted first)
    virtual void DeleteFirstRecs(const int& DelRecs) = 0;
    /// Delete specific records
    virtual void DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK = true) = 0;
    
    /// Check if the value of given field for a given record is NULL
    virtual bool IsFieldNull(const uint64& RecId, const int& FieldId) const { return false; }
    /// Get field value using field id   
    virtual int GetFieldInt(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual int16 GetFieldInt16(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual int64 GetFieldInt64(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual uchar GetFieldByte(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const = 0;
    /// Get field value using field id   
    virtual uint GetFieldUInt(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual uint16 GetFieldUInt16(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual TStr GetFieldStr(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const = 0;
    /// Get field value using field id   
    virtual bool GetFieldBool(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual double GetFieldFlt(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual float GetFieldSFlt(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const = 0;
    /// Get field value using field id   
    virtual void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const = 0;
    /// Get field value using field id   
    virtual uint64 GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const = 0;
    /// Get field value using field id   
    virtual void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const = 0;
    /// Get field value using field id   
    virtual void GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const = 0;
    /// Get field value using field id   
    virtual void GetFieldTMem(const uint64& RecId, const int& FieldId, TMem& Mem) const = 0;
    /// Get field value using field id   
    virtual PJsonVal GetFieldJsonVal(const uint64& RecId, const int& FieldId) const = 0;

    /// Get field value using field id safely   
    uint64 GetFieldUInt64Safe(const uint64& RecId, const int& FieldId) const;
    /// Get field value using field id safely   
    int64 GetFieldInt64Safe(const uint64& RecId, const int& FieldId) const;

    /// Check if the value of given field for a given record is NULL
    bool IsFieldNmNull(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    uchar GetFieldNmByte(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    int GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    void GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const;
    /// Get field value using field name   
    uint64 GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    TStr GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    void GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const;
    /// Get field value using field name   
    bool GetFieldNmBool(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    double GetFieldNmFlt(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    TFltPr GetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    void GetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, TFltV& FltV) const;
    /// Get field value using field name   
    void GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const;
    /// Get field value using field name   
    uint64 GetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm) const;
    /// Get field value using field name   
    void GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const;
    /// Get field value using field name   
    void GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const;
    /// Get field value using field name   
    void GetFieldNmTMem(const uint64& RecId, const TStr& FieldNm, TMem& mem) const;
    /// Get field value using field name   
    PJsonVal GetFieldNmJsonVal(const uint64& RecId, const TStr& FieldNm) const;

    /// Set the value of given field to NULL
    virtual void SetFieldNull(const uint64& RecId, const int& FieldId) = 0;
    /// Set field value using field id   
    virtual void SetFieldByte(const uint64& RecId, const int& FieldId, const uchar& Byte) = 0;
    /// Set field value using field id   
    virtual void SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int) = 0;
    /// Set field value using field id   
    virtual void SetFieldInt16(const uint64& RecId, const int& FieldId, const int16& Int16) = 0;
    /// Set field value using field id   
    virtual void SetFieldInt64(const uint64& RecId, const int& FieldId, const int64& Int64) = 0;
    /// Set field value using field id   
    virtual void SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV) = 0;
    /// Set field value using field id   
    virtual void SetFieldUInt(const uint64& RecId, const int& FieldId, const uint& UInt) = 0;
    /// Set field value using field id   
    virtual void SetFieldUInt16(const uint64& RecId, const int& FieldId, const uint16& UInt16) = 0;
    /// Set field value using field id   
    virtual void SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64) = 0;
    /// Set field value using field id   
    virtual void SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str) = 0;
    /// Set field value using field id   
    virtual void SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV) = 0;
    /// Set field value using field id   
    virtual void SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool) = 0;
    /// Set field value using field id   
    virtual void SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt) = 0;
    /// Set field value using field id   
    virtual void SetFieldSFlt(const uint64& RecId, const int& FieldId, const float& Flt) = 0;
    /// Set field value using field id   
    virtual void SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr) = 0;
    /// Set field value using field id   
    virtual void SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV) = 0;
    /// Set field value using field id   
    virtual void SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm) = 0;
    /// Set field value using field id   
    virtual void SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs) = 0;
    /// Set field value using field id   
    virtual void SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV) = 0;
    /// Set field value using field id   
    virtual void SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV) = 0;
    /// Set field value using field id   
    virtual void SetFieldTMem(const uint64& RecId, const int& FieldId, const TMem& Mem) = 0;
    /// Set field value using field id   
    virtual void SetFieldJsonVal(const uint64& RecId, const int& FieldId, const PJsonVal& Json) = 0;

    /// Set field value using field id   
    void SetFieldUInt64Safe(const uint64& RecId, const int& FieldId, const uint64& UInt64);
    /// Set field value using field id   
    void SetFieldInt64Safe(const uint64& RecId, const int& FieldId, const int64& Int64);

    /// Set the value of given field to NULL
    void SetFieldNmNull(const uint64& RecId, const TStr& FieldNm);
    /// Set field value using field name   
    void SetFieldNmByte(const uint64& RecId, const TStr& FieldNm, const uchar& Byte);
    /// Set field value using field name   
    void SetFieldNmInt(const uint64& RecId, const TStr& FieldNm, const int& Int);
    /// Set field value using field name   
    void SetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, const TIntV& IntV);
    /// Set field value using field name   
    void SetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm, const uint64& UInt64);
    /// Set field value using field name   
    void SetFieldNmStr(const uint64& RecId, const TStr& FieldNm, const TStr& Str);
    /// Set field value using field name   
    void SetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, const TStrV& StrV);
    /// Set field value using field name   
    void SetFieldNmBool(const uint64& RecId, const TStr& FieldNm, const bool& Bool);
    /// Set field value using field name   
    void SetFieldNmFlt(const uint64& RecId, const TStr& FieldNm, const double& Flt);
    /// Set field value using field name   
    void SetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm, const TFltPr& FltPr);
    /// Set field value using field name   
    void SetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, const TFltV& FltV);
    /// Set field value using field name   
    void SetFieldNmTm(const uint64& RecId, const TStr& FieldNm, const TTm& Tm);
    /// Set field value using field name   
    void SetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm, const uint64& TmMSecs);
    /// Set field value using field name   
    void SetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, const TIntFltKdV& SpV);
    /// Set field value using field name   
    void SetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, const PBowSpV& SpV);
    /// Set field value using field name   
    void SetFieldNmTMem(const uint64& RecId, const TStr& FieldNm, const TMem& Mem);   
    /// Set field value using field name   
    void SetFieldNmJsonVal(const uint64& RecId, const TStr& FieldNm, const PJsonVal& Json);

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
    virtual PJsonVal GetStoreJson(const TWPt<TBase>& Base) const;
    /// Parse out record id from record JSon serialization
    uint64 GetRecId(const PJsonVal& RecVal) const;

    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const uint64& RecId, const int& JoinId);
    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const uint64& RecId, const TStr& JoinNm) { return GetFieldJoinRecId(RecId, GetJoinId(JoinNm)); }
    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const uint64& RecId, const TJoinDesc& JoinDesc);
    /// Returns frequency of given field join
    int GetFieldJoinFq(const uint64& RecId, const int& JoinId);
    /// Returns frequency of given field join
    int GetFieldJoinFq(const uint64& RecId, const TStr& JoinNm) { return GetFieldJoinFq(RecId, GetJoinId(JoinNm)); }
    /// Returns frequency of given field join
    int GetFieldJoinFq(const uint64& RecId, const TJoinDesc& JoinDesc);
    
    /// Prints record set with all the field values, useful for debugging
    void PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, TSOut& SOut) const;
    /// Prints record set with all the field values, useful for debugging
    void PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, const TStr& FNm) const;
    /// Prints all records with all the field values, useful for debugging
    void PrintAll(const TWPt<TBase>& Base, TSOut& SOut, const bool& IncludingJoins = false);
    /// Prints all records with all the field values, useful for debugging
    void PrintAll(const TWPt<TBase>& Base, const TStr& FNm, const bool& IncludingJoins = false);
   /// Prints registered fields and joins, useful for debugging
    void PrintTypes(const TWPt<TBase>& Base, TSOut& SOut) const;
    /// Prints registered fields and joins, useful for debugging
    void PrintTypes(const TWPt<TBase>& Base, const TStr& FNm) const;
    /// Prints record set with all the field values, useful for debugging
    void PrintRecSetAsJson(const TWPt<TBase>& Base, const PRecSet& RecSet, TSOut& SOut);
    /// Prints record set with all the field values, useful for debugging
    void PrintRecSetAsJson(const TWPt<TBase>& Base, const PRecSet& RecSet, const TStr& FNm);
    /// Prints all records with all the field values, useful for debugging
    void PrintAllAsJson(const TWPt<TBase>& Base, TSOut& SOut);
    /// Prints all records with all the field values, useful for debugging
    void PrintAllAsJson(const TWPt<TBase>& Base, const TStr& FNm);

    /// Get codebook mappings for given string field
    virtual int GetCodebookId(const int& FieldId, const TStr& Str) const { throw TQmExcept::New("Not implemented"); }

    /// Save part of the data, given time-window
    virtual int PartialFlush(int WndInMsec = 500) { throw TQmExcept::New("Not implemented"); }
    /// Retrieve performance statistics for this store
    virtual PJsonVal GetStats() = 0;
};

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
    /// Frequency of record, default 1, can be set when this record is accessed from field join
    TInt Fq;
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
    TRec(const TWPt<TStore>& _Store, const uint64& _RecId, const int& _Fq = 1): Store(_Store), 
        ByRefP(true), RecId(_RecId), Fq(_Fq), RecValOut(RecVal) { }
    /// Constructor from JSon
    TRec(const TWPt<TStore>& _Store, const PJsonVal& JsonVal);    
    /// Copy-constructor
    TRec(const TRec& Rec);
    /// Assignment operator
    TRec& operator=(const TRec& Rec);
    
    /// Load record from stream, requires Base to initialize store pointer
    TRec(const TWPt<TBase>& Base, TSIn& SIn);
    /// Save record to stream
    void Save(TSOut& SOut) const;

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
    /// Get record id (only valid when by reference)
    int GetRecFq() const { return Fq; }

    /// Checks if field value is null
    bool IsFieldNull(const int& FieldId) const;
    /// Field value retrieval
    uchar GetFieldByte(const int& FieldId) const;
    /// Field value retrieval
    int GetFieldInt(const int& FieldId) const;
    /// Field value retrieval
    int16 GetFieldInt16(const int& FieldId) const;
    /// Field value retrieval
    int64 GetFieldInt64(const int& FieldId) const;
    /// Field value retrieval
    void GetFieldIntV(const int& FieldId, TIntV& IntV) const;
    /// Field value retrieval
    uint GetFieldUInt(const int& FieldId) const;
    /// Field value retrieval
    uint16 GetFieldUInt16(const int& FieldId) const;
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
    float GetFieldSFlt(const int& FieldId) const;
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
    /// Field value retrieval
    void GetFieldTMem(const int& FieldId, TMem& Mem) const;
    /// Field value retrieval
    PJsonVal GetFieldJsonVal(const int& FieldId) const;

    /// Field value retrieval - SAFE
    uint16 GetFieldUInt16Safe(const int& FieldId) const;
    /// Field value retrieval - SAFE
    int16 GetFieldInt16Safe(const int& FieldId) const;
    /// Field value retrieval - SAFE
    uint GetFieldUIntSafe(const int& FieldId) const;
    /// Field value retrieval - SAFE
    int GetFieldIntSafe(const int& FieldId) const;
    /// Field value retrieval - SAFE
    uint64 GetFieldUInt64Safe(const int& FieldId) const;

    /// Get field value as JSon object using field id
    PJsonVal GetFieldJson(const int& FieldId) const;
    /// Get field value as human-readable text using field id
    TStr GetFieldText(const int& FieldId) const;
    
    /// Set field value to NULL
    void SetFieldNull(const int& FieldId);
    /// Set field value
    void SetFieldByte(const int& FieldId, const uchar& Int);
    /// Set field value
    void SetFieldInt(const int& FieldId, const int& Int);
    /// Set field value
    void SetFieldInt16(const int& FieldId, const int16& Int);
    /// Set field value
    void SetFieldInt64(const int& FieldId, const int64& Int);
    /// Set field value
    void SetFieldIntV(const int& FieldId, const TIntV& IntV);
    /// Set field value
    void SetFieldUInt(const int& FieldId, const uint& UInt);
    /// Set field value
    void SetFieldUInt16(const int& FieldId, const uint16& UInt16);
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
    void SetFieldSFlt(const int& FieldId, const float& Flt);
    /// Set field value
    void SetFieldFltV(const int& FieldId, const TFltV& FltV);
    /// Set field value
    void SetFieldFltPr(const int& FieldId, const TFltPr& FltPr);
    /// Set field value
    void SetFieldTm(const int& FieldId, const TTm& Tm);
    /// Set field value
    void SetFieldTmMSecs(const int& FieldId, const uint64& TmMSecs);
    /// Set field value
    void SetFieldNumSpV(const int& FieldId, const TIntFltKdV& NumSpV);
    /// Set field value
    void SetFieldBowSpV(const int& FieldId, const PBowSpV& BowSpV);
    /// Set field value
    void SetFieldTMem(const int& FieldId, const TMem& Mem);
    /// Set field value
    void SetFieldJsonVal(const int& FieldId, const PJsonVal& Json);
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

    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const int& JoinId) const;
    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const TStr& JoinNm) const { return GetFieldJoinRecId(Store->GetJoinId(JoinNm));  }
    /// Returns record-id of given field join
    uint64 GetFieldJoinRecId(const TJoinDesc& JoinDesc) const;
    /// Returns frequency of given field join
    int GetFieldJoinFq(const int& JoinId) const;
    /// Returns frequency of given field join
    int GetFieldJoinFq(const TStr& JoinNm) const { return GetFieldJoinFq(Store->GetJoinId(JoinNm)); }
    /// Returns frequency of given field join
    int GetFieldJoinFq(const TJoinDesc& JoinDesc) const;

    /// Get record as JSon object
    PJsonVal GetJson(const TWPt<TBase>& Base, const bool& FieldsP = true, 
        const bool& StoreInfoP = true, const bool& JoinRecsP = false, 
        const bool& JoinRecFieldsP = false, const bool& RecInfoP = true) const;
};

///////////////////////////////
/// Record Comparator by Frequency. If same, sort by ID
class TRecCmpByFq {
private:
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFq(const bool& _Asc) : Asc(_Asc) {}
    
    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
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
        const bool& _Asc) : Store(_Store), FieldId(_FieldId), Asc(_Asc) {}
    
    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
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
        const bool& _Asc) : Store(_Store), FieldId(_FieldId), Asc(_Asc) {}
    
    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
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
        const bool& _Asc) : Store(_Store), FieldId(_FieldId), Asc(_Asc) {}

    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
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
        const bool& _Asc) : Store(_Store), FieldId(_FieldId), Asc(_Asc) {}
    
    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
};

///////////////////////////////
/// Record Comparator by Byte Field. 
class TRecCmpByFieldByte {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    /// Field according to which we are sorting
    TInt FieldId;
    /// Sort direction
    TBool Asc;
public:
    TRecCmpByFieldByte(const TWPt<TStore>& _Store, const int& _FieldId,
        const bool& _Asc) : Store(_Store), FieldId(_FieldId), Asc(_Asc) {}

    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const;
};

///////////////////////////////
/// Record filter
class TRecFilter {
private:
    // smart-pointer
    TCRef CRef;
    friend class TPt<TRecFilter>;

private:
    /// New constructor delegate
    typedef PRecFilter(*TNewF)(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    /// Filter New constructor router
    static TFunRouter<PRecFilter, TNewF> NewRouter;

public:
    /// Register default record filters
    static void Init();
    /// Register new record filter
    template <class TObj> static void Register() {
        NewRouter.Register(TObj::GetType(), TObj::New); }

private:
    /// QMiner Base pointer
    TWPt<TBase> Base;

protected:
    // Create new empty record filter
    TRecFilter(const TWPt<TBase>& _Base): Base(_Base) { }

    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }

public:
    /// Create trivial record filter
    static PRecFilter New(const TWPt<TBase>& Base);
    /// Create new record filter.
    static PRecFilter New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TRecFilter(Base); }
    /// Destructor
    virtual ~TRecFilter() { }

    /// Calls the filter, default keeps all records
    virtual bool Filter(const TRec& Rec) const { return true; }

    /// Filter type name
    static TStr GetType() { return "trivial"; }
    /// Filter type name 
    virtual TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter by subsampling.
/// Given a parameter Skip, is a record filter that tracks the number of potential calls to OnAdd
/// but only alows processing on every (Skip+1)-th record. Example: Skip = 1 will result in processing
/// only half of records.
class TRecFilterSubsampler : public TRecFilter {
private:
    /// Counter of updates
    mutable TInt NumUpdates;
    /// Number of samples skipped for each call to OnAdd
    TInt Skip;
    
public:
    /// Constructor
    TRecFilterSubsampler(const TWPt<TBase>& _Base, const int& Skip = 0);
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Process every (Skip+1)-th record
    bool Filter(const TRec& Rec) const;

    /// Filter type name 
    static TStr GetType() { return "subsampling"; }
    /// Filter type name 
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter to existing records. 
class TRecFilterByExists : public TRecFilter {
private:
    /// Store from which we are sorting the records 
    TWPt<TStore> Store;
    
public:
    /// Constructor
    TRecFilterByExists(const TWPt<TBase>& Base, const TWPt<TStore>& _Store);
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    
    /// Filter function
    bool Filter(const TRec& Rec) const;

    /// Filter type name 
    static TStr GetType() { return "recordExists"; }
    /// Filter type name 
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter by record Id. 
class TRecFilterByRecId : public TRecFilter {
private:
    /// Types of filtering over record ids
    typedef enum { rfRange, rfSet } TRecFilterByRecIdType;
    /// Record filter type
    TRecFilterByRecIdType FilterType;
    
    /// Minimal id
    TUInt64 MinRecId;
    /// Maximal id
    TUInt64 MaxRecId;

    /// Set of record ids to filter 
    TUInt64Set RecIdSet;
    /// Keep records that are in the set or outside set
    TBool InP;

public:
    /// Range constructor
    TRecFilterByRecId(const TWPt<TBase>& _Base, const uint64& _MinRecId, const uint64& _MaxRecId);
    /// Set constructor
    TRecFilterByRecId(const TWPt<TBase>& _Base, const TUInt64Set& _RecIdSet, const bool _InP);
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    
    /// Filter function
    bool Filter(const TRec& Rec) const;

    /// Filter type name 
    static TStr GetType() { return "recordId"; }
    /// Filter type name 
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter by record fq. 
class TRecFilterByRecFq : public TRecFilter {
private:
    /// Minimal fq
    TInt MinFq;
    /// Maximal fq
    TInt MaxFq;
    
public:
    /// Constructor
    TRecFilterByRecFq(const TWPt<TBase>& _Base, const int& _MinFq, const int& _MaxFq);
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Filter function
    bool Filter(const TRec& Rec) const;
    
    /// Filter type name 
    static TStr GetType() { return "recordFq"; }
    /// Filter type name 
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter by field.
/// Does not implement any logic, but contains JSON constructor which looks at the field
/// and given parameters and returns appropriate specialization 
class TRecFilterByField : public TRecFilter {
private:
    /// Types of filtering over record ids
    typedef enum { rfUndef, rfValue, rfRange, rfSet } TRecFilterByFieldType;
  
protected:
    /// Field according to which we are filtering
    TInt FieldId;
    /// If true (default), null fields are filtered out (Filter returns false)
    TBool FilterNullP;
    /// Internal constructor
    TRecFilterByField(const TWPt<TBase>& _Base, const int& _FieldId, const bool& _FilterNullP);
    
public:
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    
    /// Filter type name 
    static TStr GetType() { return "field"; }
    /// Filter type name 
    TStr Type() const { return GetType(); }    
};

///////////////////////////////
/// Record filter by bool field. 
class TRecFilterByFieldBool : public TRecFilterByField {
private:
    /// Value
    TBool Val;
    
public:
    /// Constructor
    TRecFilterByFieldBool(const TWPt<TBase>& _Base, const int& _FieldId, const bool& _Val, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record Filter by Integer Field. 
class TRecFilterByFieldInt : public TRecFilterByField {
private:
    /// Minimal value
    TInt MinVal;
    /// Maximal value
    TInt MaxVal;

public:
    /// Constructor
    TRecFilterByFieldInt(const TWPt<TBase>& _Base, const int& _FieldId, const int& _MinVal, const int& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record Filter by Integer Field. 
class TRecFilterByFieldInt16 : public TRecFilterByField {
private:
    /// Minimal value
    TInt16 MinVal;
    /// Maximal value
    TInt16 MaxVal;

public:
    /// Constructor
    TRecFilterByFieldInt16(const TWPt<TBase>& _Base, const int& _FieldId, const int16& _MinVal, const int16& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record Filter by Integer Field. 
class TRecFilterByFieldInt64 : public TRecFilterByField {
private:    
    /// Minimal value
    TInt64 MinVal;
    /// Maximal value
    TInt64 MaxVal;

public:
    /// Constructor
    TRecFilterByFieldInt64(const TWPt<TBase>& _Base, const int& _FieldId, const int64& _MinVal, const int64& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record Filter by Integer Field. 
class TRecFilterByFieldByte : public TRecFilterByField {
private:
    /// Minimal value
    TUCh MinVal;
    /// Maximal value
    TUCh MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldByte(const TWPt<TBase>& _Base, const int& _FieldId, const uchar& _MinVal, const uchar& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by integer field. 
class TRecFilterByFieldUInt : public TRecFilterByField {
private:
    /// Minimal value
    TUInt MinVal;
    /// Maximal value
    TUInt MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldUInt(const TWPt<TBase>& _Base, const int& _FieldId, const uint& _MinVal, const uint& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by integer field. 
class TRecFilterByFieldUInt16 : public TRecFilterByField {
private:
    /// Minimal value
    TUInt16 MinVal;
    /// Maximal value
    TUInt16 MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldUInt16(const TWPt<TBase>& _Base, const int& _FieldId, const uint16& _MinVal, const uint16& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by UInt64 field. 
class TRecFilterByFieldUInt64 : public TRecFilterByField {
private:
    /// Minimal value
    TUInt64 MinVal;
    /// Maximal value
    TUInt64 MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldUInt64(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal, const uint64& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by UInt64 or smaller integer field. 
class TRecFilterByFieldIntSafe : public TRecFilterByField {
private:
    /// Minimal value
    TUInt64 MinVal;
    /// Maximal value
    TUInt64 MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldIntSafe(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal, const uint64& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by numeric field. 
class TRecFilterByFieldFlt : public TRecFilterByField {
private:
    /// Minimal value
    TFlt MinVal;
    /// Maximal value
    TFlt MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldFlt(const TWPt<TBase>& _Base, const int& _FieldId, const double& _MinVal, const double& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by numeric field. 
class TRecFilterByFieldSFlt : public TRecFilterByField {
private:
    /// Minimal value
    TSFlt MinVal;
    /// Maximal value
    TSFlt MaxVal;
    
public:
    /// Constructor
    TRecFilterByFieldSFlt(const TWPt<TBase>& _Base, const int& _FieldId, const float& _MinVal, const float& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by string field. 
class TRecFilterByFieldStr : public TRecFilterByField {
private:
    /// String value
    const TStr StrVal;
    
public:
    /// Constructor
    TRecFilterByFieldStr(const TWPt<TBase>& _Base, const int& _FieldId, const TStr& _StrVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by string field range.
class TRecFilterByFieldStrRange : public TRecFilterByField {
private:
    /// String value - min
    const TStr StrValMin;
    /// String value - max
    const TStr StrValMax;
    
public:
    /// Constructor
    TRecFilterByFieldStrRange(const TWPt<TBase>& _Base, const int& _FieldId, const TStr& _StrVal, const TStr& _StrValMax, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by string field set. 
class TRecFilterByFieldStrSet : public TRecFilterByField {
private:
    /// String values
    TStrSet StrSet;
    
public:
    /// Constructor
    TRecFilterByFieldStrSet(const TWPt<TBase>& _Base, const int& _FieldId, const TStrSet& _StrSet, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by a set of strings in a set where the string field is using a codebook.
class TRecFilterByFieldStrSetUsingCodebook : public TRecFilterByField {
private:
    /// String values
    TIntSet IntSet;

public:
    /// Constructor
    TRecFilterByFieldStrSetUsingCodebook(const TWPt<TBase>& _Base, const int& _FieldId, const PStore& _Store, const TStrSet& _StrSet, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by time field. 
class TRecFilterByFieldTm : public TRecFilterByField {
private:
    /// Minimal value
    TUInt64 MinVal;
    /// Maximal value
    TUInt64 MaxVal;
    
public:
    /// Constructor from miliseconds
    TRecFilterByFieldTm(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal, const uint64& _MaxVal, const bool& _FilterNullP = true);
    /// Constructor from TTm time object
    TRecFilterByFieldTm(const TWPt<TBase>& _Base, const int& _FieldId, const TTm& _MinVal, const TTm& _MaxVal, const bool& _FilterNullP = true);
    /// Filter function
    bool Filter(const TRec& Rec) const;
};

///////////////////////////////
/// Record filter by index-join. 
class TRecFilterByIndexJoin : public TRecFilter {
private:
    /// Index object to use for index-joins
    TWPt<TIndex> Index;
    /// Join key ID
    int JoinKeyId;
    /// Minimal value
    TUInt64 MinVal;
    /// Maximal value
    TUInt64 MaxVal;
    
public:
    /// Constructor
    TRecFilterByIndexJoin(const TWPt<TStore>& Store, const int& JoinId, const uint64& _MinVal, const uint64& _MaxVal);
    /// JSON constructor
    static PRecFilter New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    /// Filter function
    bool Filter(const TRec& Rec) const;
    /// Filter type name
    static TStr GetType() { return "indexJoin"; }
    /// Filter type name
    TStr Type() const { return GetType(); }

};

///////////////////////////////
/// Record slitter by time field. 
class TRecSplitterByFieldTm {
private:
    /// Store from which we are splitting the records
    TWPt<TStore> Store;    
    /// Field according to which we are sorting
    TInt FieldId;
    /// Maximal difference value
    TUInt64 DiffMSecs;

public:
    TRecSplitterByFieldTm(const TWPt<TStore>& _Store, const int& _FieldId, const uint64& _DiffMSecs):
        Store(_Store), FieldId(_FieldId), DiffMSecs(_DiffMSecs) {}

    bool operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
        if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
        if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
        const uint64 RecVal1 = Store->GetFieldTmMSecs(RecIdFq1.Key, FieldId);
        const uint64 RecVal2 = Store->GetFieldTmMSecs(RecIdFq2.Key, FieldId);
        return (RecVal2 - RecVal1) > DiffMSecs;
    }
};

///////////////////////////////////////////////
/// Field value reader.
/// Utility functions for extracting and casting basic types out of records.
class TFieldReader {
private:
    /// Store Id
    TUInt StoreId;
    /// Field Id
    TIntV FieldIdV;
    /// Field description
    TFieldDescV FieldDescV;

    /// Extract string fields out of date
    void ParseDate(const TTm& Tm, TStrV& StrV) const;

    /// Assert field can provide double values
    static bool IsFlt(const TFieldDesc& FieldDesc);
    /// Assert field can provide double values
    static bool IsFltV(const TFieldDesc& FieldDesc);
    /// Assert field can provide double values
    static bool IsNumSpV(const TFieldDesc& FieldDesc);
    /// Assert field can provide double values
    static bool IsStr(const TFieldDesc& FieldDesc);
    /// Assert field can provide double values
    static bool IsStrV(const TFieldDesc& FieldDesc);
    /// Assert field can provide double values
    static bool IsTmMSecs(const TFieldDesc& FieldDesc);

    /// Pointer to function which checks specific field for type
    typedef bool (*TIsFun)(const TFieldDesc& FieldDesc);
    /// Apply given function to all fields and return true of all tests pass
    bool IsAll(TIsFun IsFun) const;

public:
    TFieldReader() { }
    /// Create reader from single field
    TFieldReader(const uint& _StoreId, const int& FieldId, const TFieldDesc& FieldDesc):
      StoreId(_StoreId) { FieldIdV.Add(FieldId); FieldDescV.Add(FieldDesc); }
    /// Create reader from multiple fields
    TFieldReader(const uint& _StoreId, const TIntV& _FieldIdV, const TFieldDescV& _FieldDescV):
      StoreId(_StoreId), FieldIdV(_FieldIdV), FieldDescV(_FieldDescV) { }

    /// Assert field can provide double values
    bool IsFlt() const { return IsAll(IsFlt); }
    /// Assert field can provide double values
    bool IsFltV() const { return IsAll(IsFltV); }
    /// Assert field can provide double values
    bool IsNumSpV() const { return IsAll(IsNumSpV); }
    /// Assert field can provide double values
    bool IsStr() const { return IsAll(IsStr); }
    /// Assert field can provide double values
    bool IsStrV() const { return IsAll(IsStrV); }
    /// Assert field can provide double values
    bool IsTmMSecs() const { return IsAll(IsTmMSecs); }

    /// Get double from a given record
    double GetFlt(const TRec& Rec) const;
    /// Get string vector from a given record
    void GetFltV(const TRec& FtrRec, TFltV& FltV) const;
    /// Get string vector from a given record set
    void GetFltV(const PRecSet& FtrRecSet, TFltV& FltV) const;
    /// Get sparse vector from a given record
    void GetNumSpV(const TRec& FtrRec, TIntFltKdV& NumSpV) const;
    /// Get string from a given record
    TStr GetStr(const TRec& FtrRec) const;
    /// Get string vector from a given record
    void GetStrV(const TRec& FtrRec, TStrV& StrV) const;
    /// Get string vector from a given record set
    void GetStrV(const PRecSet& FtrRecSet, TStrV& StrV) const;
    /// Get miliseconds from a given record
    uint64 GetTmMSecs(const TRec& FtrRec) const;

    /// Generate all possibe values that can be extracted from date
    static TStrV GetDateRange();
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
    TBool FqP;
    /// Vector of pairs (record id, weight)
    TUInt64IntKdV RecIdFqV;
    /// Vector of computed aggregates
    TVec<PAggr> AggrV;

private:
    /// Samples records from result set
    /// @param SampleSize number of records to sample out
    /// @param FqSampleP true when records are ordered according to the weight
    /// @param SampleRecIdFqV reference to vector for storing sampled records
    void GetSampleRecIdV(const int& SampleSize, 
        const bool& FqSampleP, TUInt64IntKdV& SampleRecIdFqV) const;
    /// Removes records from this result set that are not part of the provided
    void LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV);

    TRecSet() { }
    TRecSet(const TWPt<TStore>& Store, const uint64& RecId, const int& Fq);
    TRecSet(const TWPt<TStore>& Store, const TUInt64V& RecIdV);
    TRecSet(const TWPt<TStore>& Store, const TIntV& RecIdV);
    TRecSet(const TWPt<TStore>& Store, const TUInt64IntKdV& _RecIdFqV, const bool& _FqP);
    TRecSet(const TWPt<TBase>& Base, TSIn& SIn);

    /// Special access for TIndex and TBase to create weighted record sets
    /// @param FqP true when RecIdFqV contains valid weights
    static PRecSet New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV, const bool& FqP);
    friend class TIndex;
    friend class TBase;
public:
    /// Create empty set for a given store
    static PRecSet New(const TWPt<TStore>& Store);
    /// Create record set with one record
    static PRecSet New(const TWPt<TStore>& Store, const uint64& RecId, const int& Fq = 1);
    /// Create record set with one record (must be by reference)
    static PRecSet New(const TWPt<TStore>& Store, const TRec& Rec);
    /// Create record set from a given vector of record ids
    static PRecSet New(const TWPt<TStore>& Store, const TUInt64V& RecIdV);
    /// Create record set from a given vector of record ids
    static PRecSet New(const TWPt<TStore>& Store, const TIntV& RecIdV);
    /// Create record set from given vector of (Record id, weight) pairs
    static PRecSet New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV);

    /// Load record set from input stream.
    static PRecSet Load(const TWPt<TBase>& Base, TSIn& SIn){ return new TRecSet(Base, SIn); }
    /// Save record set from input stream. Does not serialize aggregates.
    void Save(TSOut& SOut);

    /// True when record set contains valid record weights
    bool IsFq() const { return FqP; }
    /// True when no record
    bool Empty() const { return RecIdFqV.Empty(); }
    /// Get store of the record set
    const TWPt<TStore>& GetStore() const { return Store; }
    /// Get store id of the record set
    uint GetStoreId() const { return Store->GetStoreId(); }

    /// Number of records in the set
    int GetRecs() const { return RecIdFqV.Len(); }  // FIXME this method should return uint64
    /// Get RecN-th record as TRec by reference
    TRec GetRec(const int& RecN) const { return TRec(GetStore(), RecIdFqV[RecN].Key); }
    /// Get id of RecN-th record
    uint64 GetRecId(const int& RecN) const { return RecIdFqV[RecN].Key; }
    /// Get weight of RecN-th record
    int GetRecFq(const int& RecN) const { return FqP ? RecIdFqV[RecN].Dat.Val : 1; }
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
    /// Filter records to keep only the ones that match the boolean value
    void FilterByFieldBool(const int& FieldId, const bool& Val);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldInt(const int& FieldId, const int& MinVal, const int& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldInt16(const int& FieldId, const int16& MinVal, const int16& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldInt64(const int& FieldId, const int64& MinVal, const int64& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldByte(const int& FieldId, const uchar& MinVal, const uchar& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldUInt(const int& FieldId, const uint& MinVal, const uint& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldUInt16(const int& FieldId, const uint16& MinVal, const uint16& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldFlt(const int& FieldId, const double& MinVal, const double& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldSFlt(const int& FieldId, const float& MinVal, const float& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldUInt64(const int& FieldId, const uint64& MinVal, const uint64& MaxVal);
    /// Filter records to keep only the ones with values of a given field equal to `FldVal'
    void FilterByFieldStr(const int& FieldId, const TStr& FldVal);
    /// Filter records to keep only the ones with values of a given field between `FldValMin' and `FldValMax' (both inclusive)
    void FilterByFieldStr(const int& FieldId, const TStr& FldValMin, const TStr& FldValMax);
    /// Filter records to keep only the ones with values of a given field present in `ValSet'
    void FilterByFieldStr(const int& FieldId, const TStrSet& ValSet);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldTm(const int& FieldId, const uint64& MinVal, const uint64& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldTm(const int& FieldId, const TTm& MinVal, const TTm& MaxVal);
    /// Filter records to keep only the ones with values of a given field within given range
    void FilterByFieldSafe(const int& FieldId, const uint64& MinVal, const uint64& MaxVal);
    /// Filter records to keep only the ones with join-record within given range
    void FilterByIndexJoin(const TWPt<TBase>& Base, const int& JoinId, const uint64& MinVal, const uint64& MaxVal);
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
    /// @param IgnoreFqP Ignores record frequency when available during sampling
    PRecSet GetSampleRecSet(const int& SampleSize, const bool& IgnoreFqP = false) const;
    /// Get record set containing `Limit' records starting from `RecN=Offset'
    PRecSet GetLimit(const int& Limit, const int& Offset) const;

    /// Merge this record set with the provided one. Result is stored in a new record set.
    /// Merging does not assume any sort order. In the process, records in this record set 
    /// are sorted by ids.
    PRecSet GetMerge(const PRecSet& RecSet) const;
    static PRecSet GetMerge(const TVec<PRecSet>& RecSetV);

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
    /// @param IgnoreFqP Ignores record frequency when available during sampling
    PRecSet DoJoin(const TWPt<TBase>& Base, const int& JoinId, const int& SampleSize = -1, const bool& IgnoreFqP = false) const;
    /// Execute join with the given name
    /// @param SampleSize Sample size used to do the join. When set to -1, all the records are used.
    /// @param IgnoreFqP Ignores record frequency when available during sampling
    PRecSet DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm, const int& SampleSize = -1, const bool& IgnoreFqP = false) const;
    /// Execute given join sequence. Each join is given by pair (id, sample size).
    PRecSet DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const;
    /// Execute given join sequence.
    PRecSet DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const;

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
        const TRec Rec = GetRec(RecN);
        // check the filter
        if (Filter.Filter(Rec)) { NewRecIdFqV.Add(RecIdFq); }
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
            ResV.Add(TRecSet::New(Store, NewRecIdFqV));
            // and initialize a new one
            NewRecIdFqV.Clr(false);            
        }
        // add new record to the next record set
        NewRecIdFqV.Add(RecIdFqV[RecN]);
    }
    // add last record set to the result list
    ResV.Add(TRecSet::New(GetStore(), NewRecIdFqV));
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
    oiktLinear   = (1 << 5), ///< Index as linearly ordered value using b-tree
    oiktInternal = (1 << 3), ///< Index used internaly for joins, using inverted index
    oiktSmall    = (1 << 4)  ///< Index uses small inverted index storage type
} TIndexKeyType;

///////////////////////////////
/// Index Key Sort Type
typedef enum { 
    oikstUndef    = 0,
    // for GIX sorting
    oikstByStr    = 1, ///< Sort lexicograficly as string
    oikstById     = 2, ///< Sort by index word id
    oikstByFlt    = 3, ///< Sort as float
    // for Linear BTree sorting
    oikstAsByte   = 8, ///< Sort as byte
    oikstAsInt    = 4, ///< Sort as int
    oikstAsInt16  = 9, ///< Sort as int16
    oikstAsInt64  =10, ///< Sort as int64
    oikstAsUInt   =11, ///< Sort as uint
    oikstAsUInt16 =12, ///< Sort as uint16
    oikstAsUInt64 = 5, ///< Sort as uint64
    oikstAsTm     = 6, ///< Sort as date-time
    oikstAsFlt    = 7, ///< Sort as double
    oikstAsSFlt   = 13, ///< Sort as float
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
        WordVocId(-1), TypeFlags(oiktUndef), SortType(oikstUndef) {}
    /// Create internal key, used for index joins
    TIndexKey(const TWPt<TBase>& Base, const uint& _StoreId, const TStr& _KeyNm, const TStr& _JoinNm, const bool& IsSmall);
    /// Create new key using given word vocabulary
    TIndexKey(const TWPt<TBase>& Base, const uint& _StoreId, const TStr& _KeyNm, const int& _WordVocId,
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
    /// Checks key type is on linearly  ordered value using b-tree
    bool IsLinear() const { return ((TypeFlags & oiktLinear) != 0); }
    /// Checks key type is internal
    bool IsInternal() const { return ((TypeFlags & oiktInternal) != 0); }
    /// Get flag that instructs index to use small gix
    bool IsSmall() const { return (TypeFlags & oiktSmall) != 0; }

    /// Get key sort type
    TIndexKeySortType GetSortType() const { return SortType; }

    /// Checks if key is sortable
    bool IsGixSort() const { return IsSortByStr() || IsSortByFlt() || IsSortById(); }
    /// Checks if key is sortable lexicographically (GIX)
    bool IsSortByStr() const { return SortType == oikstByStr; }
    /// Checks if key is sortable as number (GIX)
    bool IsSortByFlt() const { return SortType == oikstByFlt; }
    /// Checks if key is sortable by word id in the vocabulary (GIX)
    bool IsSortById() const { return SortType == oikstById; }

    /// Check if key is sortable by bytes (BTree)
    bool IsSortAsByte() const { return SortType == oikstAsByte; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsInt() const { return SortType == oikstAsInt; }
    /// Check if key is sortable by int16 (BTree)
    bool IsSortAsInt16() const { return SortType == oikstAsInt16; }
    /// Check if key is sortable by int64 (BTree)
    bool IsSortAsInt64() const { return SortType == oikstAsInt64; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsUInt() const { return SortType == oikstAsUInt; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsUInt16() const { return SortType == oikstAsUInt16; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsUInt64() const { return SortType == oikstAsUInt64; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsTm() const { return SortType == oikstAsTm; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsFlt() const { return SortType == oikstAsFlt; }
    /// Check if key is sortable by integers (BTree)
    bool IsSortAsSFlt() const { return SortType == oikstAsSFlt; }

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
    int AddKey(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm, const int& WordVocId,
        const TIndexKeyType& Type, const TIndexKeySortType& SortType = oikstUndef);
    /// Create new internal key
    int AddInternalKey(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm, const TStr& JoinNm, const bool& IsSmall);
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
    oqitLeafGixSmall = 10,///< Leaf inverted index query - for small items
    oqitGeo          = 8, ///< Geoindex query
    oqitRangeByte    = 15,///< Range BTree byte query
    oqitRangeInt     = 11,///< Range BTree integer query
    oqitRangeInt16   = 16,///< Range BTree int16 query
    oqitRangeInt64   = 17,///< Range BTree int64 query
    oqitRangeUInt    = 18,///< Range BTree uint query
    oqitRangeUInt16  = 19,///< Range BTree uint16 query
    oqitRangeUInt64  = 12,///< Range BTree uint64 query
    oqitRangeSFlt    = 20,///< Range BTree float query
    oqitRangeFlt     = 13,///< Range BTree float query
    oqitRangeTm      = 14,///< Range BTree date-time query
    oqitAnd          = 2, ///< AND between two or more queries
    oqitOr           = 3, ///< OR between two or more queries
    oqitNot          = 4, ///< NOT on current matching records
    oqitJoin         = 5, ///< Execute join on given records
    oqitRecSet       = 6, ///< Pass on a records set
    oqitRec          = 7, ///< Pass on a record
    oqitStore        = 9  ///< Incude all records from a store
} TQueryItemType; 

///////////////////////////////
/// Index Query Comparison Operators for GIX queries.
/// Comparison operators that can be specified between a field and a value
typedef enum { 
    oqctUndef    = 0,
    oqctEqual    = 1, ///< Equals (==)
    oqctGreater  = 2, ///< Greater than (>)
    oqctLess     = 3, ///< Less than (<)
    oqctNotEqual = 4, ///< Not equal (!=)
    oqctWildChar = 5 ///< Wildchar string matching (* for zero or more chars, ? for exactly one char)
} TQueryCmpType;

////////////////////////////////
/// Flags which gix objects are used in certain query
typedef enum {
    qgutUnknown = 0,       ///< Value not known yet
    qgutNone = 1,          ///< No gix used
    qgutNormal = 2,        ///< Normal gix is used
    qgutSmall = 3,         ///< Small-gix is used
    qgutBoth = 4           ///< Both gixes are used
} TQueryGixUsedType;

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
    /// Geographic coordinates (for location query)
    TFltPr Loc;
    /// Radius of search space in meters (for location query)
    TFlt LocRadius;
    /// Number of nearest neighbors of search space (for location query)
    TInt LocLimit;

    /// Edge parameters for range integer query
    TIntPr RangeIntMnMx;
    /// Edge parameters for range int16 query
    TInt16Pr RangeInt16MnMx;
    /// Edge parameters for range int64 query
    TInt64Pr RangeInt64MnMx;
    /// Edge parameters for range byte query
    TUChPr RangeUChMnMx;
    /// Edge parameters for range uint query
    TUIntUIntPr RangeUIntMnMx;
    /// Edge parameters for range uint16 query
    TUInt16Pr RangeUInt16MnMx;
    /// Edge parameters for range uint64 query
    TUInt64Pr RangeUInt64MnMx;
    /// Edge parameters for range float query
    TFltPr RangeFltMnMx;
    /// Edge parameters for range float query
    TSFltPr RangeSFltMnMx;
    
    /// List of subordinate query items.
    /// Has exactly one element when NOT or JOIN node type
    TQueryItemV ItemV;
    /// Join ID (for join nodes)
    TInt JoinId;
    /// Join sampling size (for join nodes). Value -1 means everything.
    TInt SampleSize;
    /// Record set which this query node returns (for qiven record set query)
    PRecSet RecSet;
    /// Record which this query node returns (for given record query)
    TRec Rec;
    /// Store which this query node returns
    TUInt StoreId;
    // This flag indicates which Gix is used in this query its (and its children)
    TQueryGixUsedType GixFlag;
    // This method recalculates gix flag - called after query is created
    void SetGixFlag();
    
    /// Parse Value for leaf nodes (result stored in WordIdV)
    void ParseWordStr(const TStr& WordStr, const TWPt<TIndexVoc>& IndexVoc);

    /// Parse join query from json (can be one or an array of joins)
    TWPt<TStore> ParseJoins(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
    /// Parse single join from json
    TWPt<TStore> ParseJoin(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
    /// Parse store of query
    TWPt<TStore> ParseFrom(const TWPt<TBase>& Base, const PJsonVal& JsonVal);
    /// Parse date time values in queries
    uint64 ParseTm(const PJsonVal& JsonVal);
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
    bool IsLeafGixSmall() const { return (Type == oqitLeafGixSmall); }
    /// Check query type
    bool IsGeo() const { return (Type == oqitGeo); }
    /// Check query type
    bool IsRangeInt() const { return (Type == oqitRangeInt); }
    /// Check query type
    bool IsRangeInt16() const { return (Type == oqitRangeInt16); }
    /// Check query type
    bool IsRangeInt64() const { return (Type == oqitRangeInt64); }
    /// Check query type
    bool IsRangeByte() const { return (Type == oqitRangeByte); }
    /// Check query type
    bool IsRangeUInt() const { return (Type == oqitRangeUInt); }
    /// Check query type
    bool IsRangeUInt16() const { return (Type == oqitRangeUInt16); }
    /// Check query type
    bool IsRangeUInt64() const { return (Type == oqitRangeUInt64); }
    /// Check query type
    bool IsRangeTm() const { return (Type == oqitRangeTm); }
    /// Check query type
    bool IsRangeFlt() const { return (Type == oqitRangeFlt); }
    /// Check query type
    bool IsRangeSFlt() const { return (Type == oqitRangeSFlt); }
    /// Check query type
    bool IsRange() const { return (IsRangeInt() || IsRangeInt16() || IsRangeInt64() || IsRangeByte() || IsRangeUInt64() || IsRangeUInt() || IsRangeUInt16() || IsRangeTm() || IsRangeFlt() || IsRangeSFlt()); }
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

    /// Calculates Gix-usage flag
    TQueryGixUsedType GetGixFlag() const;
    /// Optimizes query tree by removing unneeded nodes
    void Optimize();

    /// Get result store id
    uint GetStoreId(const TWPt<TBase>& Base) const;
    /// Get result store id
    TWPt<TStore> GetStore(const TWPt<TBase>& Base) const;
    /// Check if there are no subordinate items or values
    bool Empty() const { return !IsItems() && !IsWordIds(); }
    /// Check if result is weighted (only or-items)
    bool IsFq() const;

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

    /// Get integer range
    TIntPr GetRangeIntMinMax() const { return RangeIntMnMx; }
    /// Get integer range
    TInt16Pr GetRangeInt16MinMax() const { return RangeInt16MnMx; }
    /// Get integer range
    TInt64Pr GetRangeInt64MinMax() const { return RangeInt64MnMx; }
    /// Get integer range
    TUChPr GetRangeByteMinMax() const { return RangeUChMnMx; }
    /// Get uint64 integer range
    TUIntUIntPr GetRangeUIntMinMax() const { return RangeUIntMnMx; }
    /// Get uint64 integer range
    TUInt16Pr GetRangeUInt16MinMax() const { return RangeUInt16MnMx; }
    /// Get uint64 integer range
    TUInt64Pr GetRangeUInt64MinMax() const { return RangeUInt64MnMx; }
    /// Get float range
    TFltPr GetRangeFltMinMax() const { return RangeFltMnMx; }
    /// Get float range
    TSFltPr GetRangeSFltMinMax() const { return RangeSFltMnMx; }

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

    /// Internal method that traverses through the query tree and removes unneeded nodes
    void Optimize();

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
    bool IsFq() const { return QueryItem.IsFq(); }
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
class TGeoIndex; typedef TPt<TGeoIndex> PGeoIndex;
class TGeoIndex {
private:
    // smart-pointer
    TCRef CRef;
    friend class TPt<TGeoIndex>;

    /// Location precision (1,000,000 ~~ one meter)
    TFlt Precision;
    /// Map from location to records
    //TODO: Switch to GIX, maybe
    THash<TIntPr, TUInt64V> LocRecIdH;
    /// Location index
    TSphereNn<TInt, double> SphereNn;

    TIntPr GetLocId(const TFltPr& Loc) const;
    void LocKeyIdToRecId(const TIntV& LocKeyIdV, const int& Limit, TUInt64V& AllRecIdV) const;
    /// DEBUG: counts all the indexed records
    int AllRecs() const;

public:
    /// Create new empty index
    TGeoIndex(const double& _Precision) : Precision(_Precision),
        SphereNn(TSphereNn<TInt, double>::EarthRadiusKm() * 1000.0) {}
    /// Create new empty index
    static PGeoIndex New(const double& Precision = 1000000.0) { return new TGeoIndex(Precision); }
    /// Load existing index from stream
    TGeoIndex(TSIn& SIn) : Precision(SIn), LocRecIdH(SIn), SphereNn(SIn) {}
    /// Load existing index from stream
    static PGeoIndex Load(TSIn& SIn) { return new TGeoIndex(SIn); }
    /// Save index to stream
    void Save(TSOut& SOut) { Precision.Save(SOut); LocRecIdH.Save(SOut); SphereNn.Save(SOut); }

    /// Add new record
    void AddKey(const TFltPr& Loc, const uint64& RecId);
    /// Delete record
    void DelKey(const TFltPr& Loc, const uint64& RecId);
    /// Range query (in meters)
    void SearchRange(const TFltPr& Loc, const double& Radius,
        const int& Limit, TUInt64V& RecIdV) const;
    /// Nearest neighbour query
    void SearchNn(const TFltPr& Loc, const int& Limit, TUInt64V& RecIdV) const;

    /// Tells if two locations identical based on Precision
    bool LocEquals(const TFltPr& Loc1, const TFltPr& Loc2) const;
};

///////////////////////////////
// B-Tree Index
template <class TVal>
class TBTreeIndex {
private:
    // smart-pointer
    TCRef CRef;
    friend class TPt<TBTreeIndex>;

    /// We store values as (val, rec) pairs, which are sorted lexigraphically.
    /// That ensures that values are sorted primarly by value, and for same value by record id
    typedef TPair<TVal, TUInt64> TTreeVal;
    /// Define store for internal nodes
    typedef TBtree::TBtreeNodeMemStore<TTreeVal, TInt, TInt> TInternalStore;
    /// Define store for external nodes
    typedef TBtree::TBtreeNodeMemStore<TTreeVal, TVoid, TInt> TLeafStore;
    /// Define btree with given stores and value type. Each leaf node has a vector of record ids
    typedef TBtree::TBtreeOps<TTreeVal, TVoid, TCmp<TTreeVal>, TInt, TInternalStore, TLeafStore> TBtreeOps;

    /// Internal store instance
    TPt<TInternalStore> InternalStore;
    /// Leaf store instance
    TPt<TLeafStore> LeafStore;
    /// BTree instance
    TBtreeOps BTree;

public:
    /// Create new empty index
    TBTreeIndex(): InternalStore(new TInternalStore), LeafStore(new TLeafStore),
        BTree(InternalStore, LeafStore, 8, 64, false, false) { }
    /// Create new empty index
    static TPt<TBTreeIndex> New() { return new TBTreeIndex; }
    /// Load existing index from stream
    TBTreeIndex(TSIn& SIn): InternalStore(SIn), LeafStore(SIn), BTree(SIn, InternalStore, LeafStore) {  }
    /// Load existing index from stream
    static TPt<TBTreeIndex> Load(TSIn& SIn) { return new TBTreeIndex(SIn); }
    /// Save index to stream
    void Save(TSOut& SOut) { InternalStore.Save(SOut); LeafStore.Save(SOut); BTree.Save(SOut); }

    /// Add new record
    void AddKey(const TVal& Val, const uint64& RecId);
    /// Delete record
    void DelKey(const TVal& Val, const uint64& RecId);
    /// Range query
    void SearchRange(const TPair<TVal, TVal>& RangeMinMax, TUInt64V& RecIdV) const;
};

template <class TVal>
void TBTreeIndex<TVal>::AddKey(const TVal& Val, const uint64& RecId) {
    BTree.Add(TTreeVal(Val, RecId));
}

template <class TVal>
void TBTreeIndex<TVal>::DelKey(const TVal& Val, const uint64& RecId) {
    BTree.Del(TTreeVal(Val, RecId));
}

template <class TVal>
void TBTreeIndex<TVal>::SearchRange(const TPair<TVal, TVal>& RangeMinMax, TUInt64V& RecIdV) const {

    TVec<TTreeVal> ResValRecIdV;
    // execute query
    BTree.RangeQuery(TTreeVal(RangeMinMax.Val1, 0), TTreeVal(RangeMinMax.Val2, TUInt64::Mx), ResValRecIdV);
    // parse out record ids
    RecIdV.Gen(ResValRecIdV.Len(), 0);
    for (int ResN = 0; ResN < ResValRecIdV.Len(); ResN++) {
        RecIdV.Add(ResValRecIdV[ResN].Val2);
    }
}

///////////////////////////////
/// Index
class TIndex {
private: 
    // smart-pointer
    TCRef CRef;
    friend class TPt<TIndex>;
public:
    // gix template definitions
    typedef TKeyWord TQmGixKey; // (int KeyId, uint64 WordId)
    typedef TKeyDat<TUInt64, TInt> TQmGixItem; // [RecId, Freq]
    typedef TKeyDat<TUInt, TSInt> TQmGixItemSmall; // [RecId, Freq]
    typedef TVec<TQmGixItem> TQmGixItemV;
    typedef TVec<TQmGixItemSmall> TQmGixItemSmallV;
    typedef TPt<TGixExpMerger<TQmGixKey, TQmGixItem> > PQmGixExpMerger;
    typedef TPt<TGixExpMerger<TQmGixKey, TQmGixItemSmall> > PQmGixExpMergerSmall;
    typedef TPt<TGixKeyStr<TQmGixKey> > PQmGixKeyStr;

    /// Merger which sums up the frequencies
    class TQmGixDefMerger : public TGixExpMerger<TQmGixKey, TQmGixItem> {
    public:
        static PGixExpMerger New() { return new TQmGixDefMerger(); }

        // overriden abstract methods
        void Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
        void Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
        void Minus(const TQmGixItemV& MainV, const TQmGixItemV& JoinV, TQmGixItemV& ResV) const;
        void Def(const TQmGixKey& Key, TQmGixItemV& MainV) const {}
        
        // methods needed for template
        void Merge(TQmGixItemV& ItemV, bool IsLocal) const;
        void Delete(const TQmGixItem& Item, TVec<TQmGixItem>& MainV) const { return MainV.DelAll(Item); }
        bool IsLt(const TQmGixItem& Item1, const TQmGixItem& Item2) const { return Item1 < Item2; }
        bool IsLtE(const TQmGixItem& Item1, const TQmGixItem& Item2) const { return Item1 <= Item2; }
    };

    /// Merger which sums up the frequencies
    /// For small index data
    class TQmGixDefMergerSmall : public TGixExpMerger<TQmGixKey, TQmGixItemSmall> {
    public:
        static PGixExpMerger New() { return new TQmGixDefMergerSmall(); }

        // overriden abstract methods
        void Union(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const;
        void Intrs(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const;
        void Minus(const TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV, TQmGixItemSmallV& ResV) const;
        void Def(const TQmGixKey& Key, TQmGixItemSmallV& MainV) const {}

        // methods needed for template
        void Merge(TQmGixItemSmallV& ItemV, bool IsLocal) const;
        void Delete(const TQmGixItemSmall& Item, TQmGixItemSmallV& MainV) const { return MainV.DelAll(Item); }
        bool IsLt(const TQmGixItemSmall& Item1, const TQmGixItemSmall& Item2) const { return Item1 < Item2; }
        bool IsLtE(const TQmGixItemSmall& Item1, const TQmGixItemSmall& Item2) const { return Item1 <= Item2; }
    };

    /// Merger which sums the frequencies but removes the duplicates (e.g. 3+1 = 1+1 = 2)
    class TQmGixRmDupMerger : public TQmGixDefMerger {
    public:
        static PGixExpMerger New() { return new TQmGixRmDupMerger(); }

        void Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
        void Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const;
    };

    /// Merger which sums the frequencies but removes the duplicates (e.g. 3+1 = 1+1 = 2)
    /// For small index data
    class TQmGixRmDupMergerSmall : public TQmGixDefMergerSmall {
    public:
        static PGixExpMerger New() { return new TQmGixRmDupMergerSmall(); }

        void Union(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const;
        void Intrs(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const;
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

    // more typedefs
    typedef TGixItemSet<TQmGixKey, TQmGixItem, TQmGixDefMerger> TQmGixItemSet;
    typedef TGixItemSet<TQmGixKey, TQmGixItemSmall, TQmGixDefMergerSmall> TQmGixItemSetSmall;
    typedef TPt<TQmGixItemSet> PQmGixItemSet;
    typedef TPt<TQmGixItemSetSmall> PQmGixItemSetSmall;
    typedef TGix<TQmGixKey, TQmGixItem, TQmGixDefMerger> TQmGix;
    typedef TGix<TQmGixKey, TQmGixItemSmall, TQmGixDefMergerSmall> TQmGixSmall;
    typedef TPt<TQmGix> PQmGix;
    typedef TPt<TQmGixSmall> PQmGixSmall;
    typedef TGixExpItem<TQmGixKey, TQmGixItem, TQmGixDefMerger> TQmGixExpItem;
    typedef TGixExpItem<TQmGixKey, TQmGixItemSmall, TQmGixDefMergerSmall> TQmGixExpItemSmall;
    typedef TPt<TQmGixExpItem> PQmGixExpItem;
    typedef TPt<TQmGixExpItemSmall> PQmGixExpItemSmall;

    // b-tree definitions
    typedef TPt<TBTreeIndex<TUCh>> PBTreeIndexUCh;
    typedef TPt<TBTreeIndex<TInt>> PBTreeIndexInt;
    typedef TPt<TBTreeIndex<TInt16>> PBTreeIndexInt16;
    typedef TPt<TBTreeIndex<TInt64>> PBTreeIndexInt64;
    typedef TPt<TBTreeIndex<TUInt>> PBTreeIndexUInt;
    typedef TPt<TBTreeIndex<TUInt16>> PBTreeIndexUInt16;
    typedef TPt<TBTreeIndex<TUInt64>> PBTreeIndexUInt64;
    typedef TPt<TBTreeIndex<TFlt>> PBTreeIndexFlt;
    typedef TPt<TBTreeIndex<TSFlt>> PBTreeIndexSFlt;

private:    
    /// Remember index location
    TStr IndexFPath;
    /// Remember access mode to the index
    TFAccess Access;
    /// Inverted index
    mutable PQmGix Gix;
    /// Inverted index - small
    mutable PQmGixSmall GixSmall;

    /// Location index (one for each key)
    THash<TInt, PGeoIndex> GeoIndexH;
    /// BTree index for bytes (one for each key)
    THash<TInt, PBTreeIndexUCh> BTreeIndexByteH;
    /// BTree index for integers (one for each key)
    THash<TInt, PBTreeIndexInt> BTreeIndexIntH;
    /// BTree index for int16 (one for each key)
    THash<TInt, PBTreeIndexInt16> BTreeIndexInt16H;
    /// BTree index for inte64 (one for each key)
    THash<TInt, PBTreeIndexInt64> BTreeIndexInt64H;
    /// BTree index uint (one for each key)
    THash<TInt, PBTreeIndexUInt> BTreeIndexUIntH;
    /// BTree index uint16 (one for each key)
    THash<TInt, PBTreeIndexUInt16> BTreeIndexUInt16H;
    /// BTree index uint64 (one for each key)
    THash<TInt, PBTreeIndexUInt64> BTreeIndexUInt64H;
    /// BTree index for floats (one for each key)
    THash<TInt, PBTreeIndexFlt> BTreeIndexFltH;
    /// BTree index for floats (one for each key)
    THash<TInt, PBTreeIndexSFlt> BTreeIndexSFltH;

    /// Index Vocabulary
    PIndexVoc IndexVoc;
    /// Inverted Index Default Merger
    PQmGixExpMerger DefMerger;
    /// Inverted Index Default Merger Small
    PQmGixExpMergerSmall DefMergerSmall;

    /// Converts query item tree to GIX query expression
    PQmGixExpItem ToExpItem(const TQueryItem& QueryItem) const;
    /// Converts query item tree to GIX-small query expression
    PQmGixExpItemSmall ToExpItemSmall(const TQueryItem& QueryItem) const;
    /// Executes GIX query expression against the index
    bool DoQuery(const PQmGixExpItem& ExpItem, const PQmGixExpMerger& Merger, 
        TQmGixItemV& RecIdFqV) const;
    /// Executes GIX-small query expression against the index
    bool DoQuerySmall(const PQmGixExpItemSmall& ExpItem, const PQmGixExpMergerSmall& Merger,
        TQmGixItemSmallV& RecIdFqV) const;
    /// Determines which Gix should be used for given KeyId
    bool UseGixSmall(const int& KeyId) const { return IndexVoc->GetKey(KeyId).IsSmall(); }
    /// Upgrades a vector of small items into a vector of big ones
    void Upgrade(const TQmGixItemSmallV& Src, TQmGixItemV& Dest) const;

    /// Constructor
    TIndex(const TStr& _IndexFPath, const TFAccess& _Access, const PIndexVoc& IndexVoc,
        const int64& CacheSize, const int64& CacheSizeSmall, const int& SplitLen);
public:
    /// Create (Access==faCreate) or open existing index
    static PIndex New(const TStr& IndexFPath, const TFAccess& Access, const PIndexVoc& IndexVoc,
        const int64& CacheSize, const int64& CacheSizeSmall, const int& SplitLen) {
            return new TIndex(IndexFPath, Access, IndexVoc, CacheSize, CacheSizeSmall, SplitLen);
    }
    /// Checks if there is an existing index at the given path
    static bool Exists(const TStr& IndexFPath) { return TFile::Exists(IndexFPath + "Index.Gix"); }
    
    /// Close the query
    ~TIndex();

    /// Get index location
    TStr GetIndexFPath() const { return IndexFPath; }
    /// Get index vocabulary
    TWPt<TIndexVoc> GetIndexVoc() const { return IndexVoc; }
    /// Get default index merger
    PQmGixExpMerger GetDefMerger() const { return DefMerger; }
    /// Get default index merger - small gix
    PQmGixExpMergerSmall GetDefMergerSmall() const { return DefMergerSmall; }

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
        const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = TInt::Mx);
    // Remove join from index
    void DeleteJoin(const TWPt<TStore>& Store, const TStr& JoinNm, 
        const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = TInt::Mx);
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

    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const uint& StoreId, const TStr& KeyNm, const int& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const uint& StoreId, const TStr& KeyNm, const uint64& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const uint& StoreId, const TStr& KeyNm, const double& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const uchar& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const int& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const int16& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const int64& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const uint& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const uint16& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const uint64& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const double& Val, const uint64& RecId);
    /// Add RecId to linear index under (Key, Val)
    void IndexLinear(const int& KeyId, const float& Val, const uint64& RecId);

    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uchar& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int16& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int64& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint16& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint64& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const double& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const uint& StoreId, const TStr& KeyNm, const float& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const uchar& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const int& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const int16& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const int64& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const uint& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const uint16& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const uint64& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const double& Val, const uint64& RecId);
    /// Delete RecId from linear index under (Key, Val)
    void DeleteLinear(const int& KeyId, const float& Val, const uint64& RecId);

    /// Check if index opened in read-only mode
    bool IsReadOnly() const { return Access == faRdOnly; }

    /// Do flat AND search, given the vector of inverted index queries
    void SearchAnd(const TIntUInt64PrV& KeyWordV, TQmGixItemV& StoreRecIdFqV) const;
    /// Do flat OR search, given the vector of inverted index queries
    void SearchOr(const TIntUInt64PrV& KeyWordV, TQmGixItemV& StoreRecIdFqV) const;
    /// Search with special Merger (does not handle joins)
    TPair<TBool, PRecSet> Search(const TWPt<TBase>& Base, const TQueryItem& QueryItem, const PQmGixExpMerger& Merger, const PQmGixExpMergerSmall& MergerSmall) const;
    /// Do geo-location range (in meters) search
    PRecSet SearchGeoRange(const TWPt<TBase>& Base, const int& KeyId,
        const TFltPr& Loc, const double& Radius, const int& Limit) const;
    /// Do geo-location nearest-neighbor search
    PRecSet SearchGeoNn(const TWPt<TBase>& Base, const int& KeyId,
        const TFltPr& Loc, const int& Limit) const;
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUChPr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TIntPr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TInt16Pr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TInt64Pr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUIntUIntPr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUInt16Pr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUInt64Pr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TFltPr& RangeMinMax);
    /// Do B-Tree linear search
    PRecSet SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TSFltPr& RangeMinMax);
    /// Get records ids and counts that are joined with given RecId (via given join key)
    void GetJoinRecIdFqV(const int& JoinKeyId, const uint64& RecId, TUInt64IntKdV& JoinRecIdFqV) const;
    /// Are there any existing joins from RecId using JoinKeyId
    bool HasJoin(const int& JoinKeyId, const uint64& RecId) const;

    /// Save debug statistics to a file
    void SaveTxt(const TWPt<TBase>& Base, const TStr& FNm);

    /// get blob stats
    TBlobBsStats GetBlobStats() const;
    /// get gix stats
    TGixStats GetGixStats(const bool& RefreshP = true) const;

    /// Get split length of inner Gix
    int GetSplitLen() const { return Gix->GetSplitLen(); }
    /// reset blob stats
    void ResetStats() { Gix->ResetStats(); GixSmall->ResetStats(); }

    /// perform partial flush of index contents
    int PartialFlush(const int& WndInMsec = 500);
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
    
public:
    /// Register default stream aggregates
    static void Init();
    /// Register new stream aggregate
    template <class TObj> static void Register() {
        NewRouter.Register(TObj::GetType(), TObj::New); }
    
protected:
    /// QMiner Base pointer
    TWPt<TBase> Base;
    /// Stream aggreagte name
    const TStr AggrNm;

protected:
    /// Create new stream aggregate from JSon parameters
    TStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm);
    /// Create new stream aggregate from JSon parameters
    TStreamAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }

    /// Parse stream aggregate from json
    TWPt<TStreamAggr> ParseAggr(const PJsonVal& ParamVal, const TStr& AggrKeyNm);
    
public:
    /// Create new stream aggregate based on provided JSon parameters
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    /// Virtual destructor!
    virtual ~TStreamAggr() { }

    /// Load stream aggregate state from stream
    virtual void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    virtual void SaveState(TSOut& SOut) const;

    virtual PJsonVal GetParam() const { return TJsonVal::NewObj(); }
    virtual void SetParam(const PJsonVal& JsonVal) {}

    /// Get aggregate name
    const TStr& GetAggrNm() const { return AggrNm; }
    /// Is the aggregate initialized. Used for aggregates, which require some time to get started.
    virtual bool IsInit() const { return true; }

    /// Reset the state of the aggregate
    virtual void Reset() = 0;

    /// Update state of the aggregate
    virtual void OnStep() { }
    /// Update state of the aggregate at time
    virtual void OnTime(const uint64& TmMsec) { OnStep(); }
    /// Add new record to the aggregate
    virtual void OnAddRec(const TRec& Rec) { OnStep(); }
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
    virtual TStr Type() const = 0;

protected:
    /// Cast stream aggregate to interface
    template <class IInterface>
    static TWPt<IInterface> Cast(const TWPt<TStreamAggr>& Aggr, const bool& CheckP = true) {
        TWPt<IInterface> CastAggr = dynamic_cast<IInterface*>(Aggr());
        QmAssertR(!CastAggr.Empty() || !CheckP, "[TStreamAggr] error casting " + Aggr->GetAggrNm());
        return CastAggr;
    }
};

///////////////////////////////
/// Stream aggregate output interfaces.
namespace TStreamAggrOut {
    class IInt {
    public:
        virtual ~IInt() {}
        virtual int GetInt() const = 0;
    };

    class IFlt {
    public:
        virtual ~IFlt() {}
        virtual double GetFlt() const = 0;
    };

    template <class TVal>
    class IVal {
    public:
        virtual ~IVal() {}
        virtual TVal GetVal() const = 0;
    };
        
    class ITm {
    public:
        virtual uint64 GetTmMSecs() const = 0;
    };
    
    /// vector of values
    template <class TVal>
    class IValVec {
    public:
        virtual int GetVals() const = 0;
        virtual void GetVal(const int& ElN, TVal& Val) const = 0;
        virtual void GetValV(TVec<TVal>& ValV) const = 0;
    };
    typedef IValVec<TFlt> IFltVec;
    typedef IValVec<TIntFltKdV> ISparseVVec;
    typedef IValVec<TIntFltKd> ISparseVec;

    /// vector of timestamps
    class ITmVec {
    public:
        virtual int GetTmLen() const = 0;
        virtual uint64 GetTm(const int& ElN) const = 0;
        virtual void GetTmV(TUInt64V& MSecsV) const = 0;
    };

    template <class TVal>
    class IValIO {
    public:
        // incomming
        virtual void GetInValV(TVec<TVal>& ValV) const = 0;
        // outgoing
        virtual void GetOutValV(TVec<TVal>& ValV) const = 0;
    };
    typedef IValIO<TFlt> IFltIO;

    class ITmIO {
    public:
        // incomming
        virtual void GetInTmMSecsV(TUInt64V& MSecsV) const = 0;
        // outgoing
        virtual void GetOutTmMSecsV(TUInt64V& MSecsV) const = 0;
    };
    
    class INmInt {
    public:
        // retrieving named values
        virtual bool IsNm(const TStr& Nm) const = 0;
        virtual double GetNmInt(const TStr& Nm) const = 0;
        virtual void GetNmIntV(TStrIntPrV& NmIntV) const = 0;
    };

    class INmFlt {
    public:
        // retrieving named values
        virtual bool IsNmFlt(const TStr& Nm) const = 0;
        virtual double GetNmFlt(const TStr& Nm) const = 0;
        virtual void GetNmFltV(TStrFltPrV& NmFltV) const = 0;
    };

    class IFtrSpace {
    public:
        // get feature space
        virtual PFtrSpace GetFtrSpace() const = 0;
    };
}
///////////////////////////////
/// Stream aggregator set.
/// Holds a set of stream aggregates and triggers them all on call.
/// Aggregates are triggered in the same order as they are added to the set.
class TStreamAggrSet : public TStreamAggr {
protected:
    /// List of aggregates triggered in step
    TVec<TWPt<TStreamAggr>> StreamAggrV;
    
    /// Create empty aggregate base
    TStreamAggrSet(const TWPt<TBase>& _Base, const TStr& _AggrNm);
    /// Create empty aggregate base from json
    TStreamAggrSet(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
public:
    /// Create empty aggregate base
    static PStreamAggr New(const TWPt<TBase>& Base);
    /// Create empty aggregate base
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm);
    /// Create empty aggregate base
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Is the aggregate set empty
    bool Empty() const;
    /// Number of aggregates in the set
    int Len() const;
    /// Add new aggregate to the name
    void AddStreamAggr(const PStreamAggr& StreamAggr);
    /// Get stream aggregate by name
    const TWPt<TStreamAggr>& GetStreamAggr(const int& StreamAggrN) const;
    /// Get list of all aggregates
    TStrV GetStreamAggrNmV() const;
    
    /// Reset all aggregates in the set
    void Reset();

    /// Update state of the aggregates
    void OnStep();
    /// Update state of the aggregates at time
    void OnTime(const uint64& TmMsec);
    /// Add new record to the aggregates
    void OnAddRec(const TRec& Rec);
    /// Recored already added to the aggregates is being updated
    void OnUpdateRec(const TRec& Rec);
    /// Recored already added to the aggregates is being deleted from the store 
    void OnDeleteRec(const TRec& Rec);

    /// Print latest statistics to logger
    void PrintStat() const;
    /// Serialization current status to JSon
    PJsonVal SaveJson(const int& Limit) const;

    // stream aggregator type name 
    static TStr GetType() { return "set"; }
    TStr Type() const { return GetType(); }    
};

///////////////////////////////
/// Store trigger that pushes updates to stream aggregates
class TStreamAggrTrigger : public TStoreTrigger {
private:
    /// Pointer to stream aggregate we forward records to
    TWPt<TStreamAggr> StreamAggr;

    /// Create new trigger for given stream aggregate
    TStreamAggrTrigger(const TWPt<TStreamAggr>& StreamAggr);
public:
    /// Create new trigger for given stream aggregate
    static PStoreTrigger New(const TWPt<TStreamAggr>& StreamAggr);

    /// new record added to the store, call stream aggregate OnAddRec
    void OnAdd(const TRec& Rec);
    /// record is updated in the store, call stream aggregate OnUpdateRec
    void OnUpdate(const TRec& Rec);
    /// record is deleted from the store, call stream aggregate OnDeleteRec
    void OnDelete(const TRec& Rec);
};

///////////////////////////////
// QMiner-Base
class TBase {
private: 
    /// Smart pointer reference counter
    TCRef CRef;
    /// We are friends with smart pointer so it can access referenc coutner
    friend class TPt<TBase>;
    
    /// True after the base is initialized
    TBool InitP;

    /// Location of base on the disk
    TStr FPath;
    /// Access type to currently opened base
    TFAccess FAccess;

    /// Index vocabilary
    PIndexVoc IndexVoc;
    /// Index structures (gix, linear, geo)
    PIndex Index;
    /// Shared stora storage layer
    PBlobBs StoreBlobBs;
    /// List of open stores
    TVec<PStore> StoreV;
    /// Map from store name to store
    THash<TStr, PStore> StoreH;

    /// List of all registered stream aggregates
    THash<TStr, PStreamAggr> StreamAggrH;
    /// Stream aggregate sets for each store
    TVec<TWPt<TStreamAggrSet>> StreamAggrSetV;
    
    /// Name validates used for validating field, join and key names
    TNmValidator NmValidator;

private:
    /// Invert given record set (replace with all the records from the store that are not in it)
    PRecSet Invert(const PRecSet& RecSet, const TIndex::PQmGixExpMerger& Merger);
    /// Execute search query. Returns results and a flag indicating if the results should be inverted.
    TPair<TBool, PRecSet> Search(const TQueryItem& QueryItem, const TIndex::PQmGixExpMerger& Merger,
        const TIndex::PQmGixExpMergerSmall& MergerSmall, const TQueryGixUsedType& ParentGixFlag);

    /// Get config name for base located on a given path
    static TStr GetConfFNm(const TStr& FPath) { return FPath + "Base.json"; }
    /// Load base config
    void LoadBaseConf(const TStr& FPath);
    /// Save base config
    void SaveBaseConf(const TStr& FPath) const;

    /// Create new base on the given folder
    TBase(const TStr& _FPath, const int64& IndexCacheSize, const int& SplitLen, const bool& StrictNmP);
    /// Open existing base from the given folder
    TBase(const TStr& _FPath, const TFAccess& _FAccess, const int64& IndexCacheSize, const int& SplitLen);

public:
    ~TBase();

    /// Create new base on the given folder
    static TWPt<TBase> New(const TStr& FPath, const int64& IndexCacheSize, const int& SplitLen, const bool& StrictNmP) {
        return new TBase(FPath, IndexCacheSize, SplitLen, StrictNmP);
    }
    /// Open existing base from the given folder
    static TWPt<TBase> Load(const TStr& FPath, const TFAccess& FAccess, const int64& IndexCacheSize, const int& SplitLen) {
        return new TBase(FPath, FAccess, IndexCacheSize, SplitLen);
    }

    /// Check if base already exists at a given folder
    static bool Exists(const TStr& FPath);

    /// Check if base is initialized
    bool IsInit() const { return InitP; }
    /// Initialize base
    void Init();

    /// Get folder where base is located
    const TStr& GetFPath() const { return FPath; }
    /// Check if base is open in read only mode
    bool IsRdOnly() const { return FAccess == faRdOnly; }
    /// Get mode in which the base is opened
    const TFAccess& GetFAccess() const { return FAccess; }

    /// Get index vocabulary
    TWPt<TIndexVoc> GetIndexVoc() const { return IndexVoc; }
    /// Get index
    TWPt<TIndex> GetIndex() const;

    /// Add new store
    void AddStore(const PStore& NewStore);
    /// Get number of stores
    int GetStores() const { return StoreH.Len(); }
    /// Check if store with given number exists
    bool IsStoreN(const uint& StoreN) const { return StoreN < (uint)StoreH.Len(); }
    /// Get store with the given number
    const TWPt<TStore> GetStoreByStoreN(const int& StoreN) const;
    /// Check if store with the given ID exists
    bool IsStoreId(const uint& StoreId) const { return !StoreV[StoreId].Empty(); }
    /// Get store with the given ID
    const TWPt<TStore> GetStoreByStoreId(const uint& StoreId) const;
    /// Check if store with the given name exists
    bool IsStoreNm(const TStr& StoreNm) const { return StoreH.IsKey(StoreNm); }
    /// Get store with the given name
    const TWPt<TStore> GetStoreByStoreNm(const TStr& StoreNm) const;
    /// Helper function for returning JSon definition of store
    PJsonVal GetStoreJson(const TWPt<TStore>& Store);
    /// Get store blob base
    const PBlobBs& GetStoreBlobBs() { return StoreBlobBs; }

    /// Check if base has stream aggregate with the given name
    bool IsStreamAggr(const TStr& StreamAggrNm) const;
    /// Register new stream aggregate to the base
    void AddStreamAggr(const PStreamAggr& StreamAggr);
    /// Get stream aggregate with the given name from base
    TWPt<TStreamAggr> GetStreamAggr(const TStr& StreamAggrNm) const;
    /// Get list of all stream aggregates
    TStrV GetStreamAggrNmV() const { TStrV NmV; StreamAggrH.GetKeyV(NmV); return NmV; }
    /// Get stream aggregate set for the given store
    TWPt<TStreamAggrSet> GetStreamAggrSet(const uint& StoreId) const;

    /// Aggregate given recordset and add aggregates to the record set
    void Aggr(PRecSet& RecSet, const TQueryAggrV& QueryAggrV);

    /// Create new word vocabulary and returns its id
    int NewIndexWordVoc(const TIndexKeyType& Type, const TStr& WordVocNm = TStr());
    /// Create index key, without linking it to a filed, returns the id of created key
    int NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const TIndexKeyType& Type = oiktValue,
        const TIndexKeySortType& SortType = oikstUndef);
    /// Create index key, without linking it to a field using specified vocabulary,
    /// returns the id of created key
    int NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& WordVocId,
        const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    /// Create index key for a specified (store, field) pair, returns the id of created key
    int NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId,
        const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    /// Create index key for a specified (store, field) pair, returns the id of created key.
    /// Uses provided custom key name.
    int NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId, 
        const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    // Create index key for a specified (store, field) pair using specified vocabulary,
    // returns the id of created key
    int NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId, const int& WordVocId, 
        const TIndexKeyType& Type = oiktValue, const TIndexKeySortType& SortType = oikstUndef);
    // Create index key for a specified (store, field) pair using specified vocabulary,
    // returns the id of created key. Uses provided custom key name.
    int NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId, 
        const int& WordVocId, const TIndexKeyType& Type = oiktValue, 
        const TIndexKeySortType& SortType = oikstUndef);

    /// Add new record to a give store
    uint64 AddRec(const TWPt<TStore>& Store, const PJsonVal& RecVal);
    /// Add new record to a give store
    uint64 AddRec(const TStr& StoreNm, const PJsonVal& RecVal);
    /// Add new record to a give store
    uint64 AddRec(const uint& StoreId, const PJsonVal& RecVal);
    
    /// Searching records (default search interface)
    PRecSet Search(const PQuery& Query);
    /// Searching records (default search interface)
    PRecSet Search(const TQueryItem& QueryItem);
    /// Searching records (default search interface)
    PRecSet Search(const TStr& QueryStr);
    /// Searching records (default search interface)
    PRecSet Search(const PJsonVal& QueryVal);
    
    /// Execute garbage collection on all stores
    void GarbageCollect();
    /// Perform partial flush of data
    int PartialFlush(int WndInMsec = 500);

    /// asserts if a field name is valid
    void AssertValidNm(const TStr& FldNm) const { NmValidator.AssertValidNm(FldNm); }
    /// when set to true, all field names except an empty string will be valid
    void SetStrictNmP(const bool& StrictNmP) { NmValidator.SetStrictNmP(StrictNmP); }    

    /// Dump complete base to json
    bool SaveJSonDump(const TStr& DumpDir);
    /// Restore complete base from json
    bool RestoreJSonDump(const TStr& DumpDir);
    
    /// Write store statistics to file
    void PrintStores(const TStr& FNm, const bool& FullP = false);
    /// Write index vocabulary statistics to file
    void PrintIndexVoc(const TStr& FNm);
    /// Write index statistics to file
    void PrintIndex(const TStr& FNm, const bool& SortP);
        
    /// Get gix-blob stats
    const TBlobBsStats GetGixBlobStats() { return Index->GetBlobStats(); }
    /// Get gix stats
    const TGixStats GetGixStats(bool do_refresh = true) { return Index->GetGixStats(do_refresh); }
    /// Reset gix-blob stats
    void ResetGixStats() { Index->ResetStats(); }
    /// Get performance statistics in JSON form
    PJsonVal GetStats();
};

////////////////////////////////////////////////////////////////////////////
// Some utility functions

/// Export TBlobBsStats object to JSON
PJsonVal BlobBsStatsToJson(const TBlobBsStats& stats);

/// Export TGixStats object to JSON
PJsonVal GixStatsToJson(const TGixStats& stats);

} // namespace

#endif
