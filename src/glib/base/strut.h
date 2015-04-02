/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/////////////////////////////////////////////////
// String-Utilities
class TStrUtil {
public:
	static TStr GetStr(const TIntV& IntV, const TStr& DelimiterStr = ",");
	static TStr GetStr(const TVec<TIntV>& IntIntV, const TStr& DelimiterStr = ",");
	static TStr GetStr(const TStrIntPrV& StrIntPrV, const TStr& FieldDelimiterStr = ":",
		const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TFltV& FltV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TIntFltKdV& IntFltKdV, const TStr& FieldDelimiterStr = ":", 
		const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");	
	static TStr GetStr(const TStrV& StrV, const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TStrH& StrH, const TStr& FieldDelimiterStr = ":", const TStr& DelimiterStr = ",");	
	static TStr GetStr(const TFltVV& FltVV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");
	static TStr GetStr(const TIntIntFltTrV& IntIntFltTrV, const TStr& DelimiterStr = ",", const TStr& FmtStr = "%g");

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
