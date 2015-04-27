/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
