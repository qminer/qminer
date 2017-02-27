/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// String-Utilities
class TStrUtil {
public:
	static TStr GetStr(const TIntV& IntV, const TStr& DelimiterStr = ",");
	static TStr GetStr(const TUInt64V& UInt64V, const TStr& DelimiterStr = ",");
	static TStr GetStr(const TVec<TIntV>& IntIntV, const TStr& DelimiterStr = ",");
	//Explicit 64 bit indexing
	static TStr GetStr(const TVec<TNum<int64>, int64>& IntV, const TStr& DelimiterStr);
	static TStr GetStr(const TStrIntPrV& StrIntPrV, const TStr& FieldDelimiterStr = ":",
		const TStr& DelimiterStr = ",");
	static TStr GetStr(const TUInt64IntPrV& UInt64IntPrV, const TStr& FieldDelimiterStr = ":",
			const TStr& DelimiterStr = ",");
	static TStr GetStr(const TIntFltPrV& IntFltPrV, const TStr& FieldDelimiterStr = ":",
			const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");
	static TStr GetStr(const TFltV& FltV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TIntFltKdV& IntFltKdV, const TStr& FieldDelimiterStr = ":", 
		const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TStrV& StrV, const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TStrH& StrH, const TStr& FieldDelimiterStr = ":", const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TFltVV& FltVV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");
	static TStr GetStr(const TVec<TIntFltKdV>& FltSpVV) { throw TExcept::New("Not implemented!"); }
	static TStr GetStr(const TIntIntFltTrV& IntIntFltTrV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");
	static TStr GetStr(const TUInt64UInt64IntTrV& UInt64UInt64IntTrV, const TStr& DelimiterStr = ",");
	static TStr GetStr(const TIntSet& IntSet, const TStr& DelimiterStr = ",");

    // format number 1234567 as "1,234,567"
    static TStr GetStr(const int& Val, const TStr& ThousandDelimiterStr = ",");

	template <class TKey, class TDat>
	static TStr GetStr(const THash<TKey, TDat>& KeyDatH);
};

template <class TKey, class TDat>
TStr TStrUtil::GetStr(const THash<TKey, TDat>& KeyDatH) {
	TChA ChA = "";

	int KeyId = KeyDatH.FFirstKeyId();
	while (KeyDatH.FNextKeyId(KeyId)) {
		const TKey& Key = KeyDatH.GetKey(KeyId);
		const TDat& Val = KeyDatH[KeyId];

		ChA += "(" + Key.GetStr() + "," + Val.GetStr() + ")";
	}

	return ChA;
}
