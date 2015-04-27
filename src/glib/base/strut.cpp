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

////////////////////////////////////////////////
// String-Utilities
TStr TStrUtil::GetStr(const TIntV& IntV, const TStr& DelimiterStr) {
  TChA ResChA;
  for (int IntN = 0; IntN < IntV.Len(); IntN++) {
	if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
    ResChA+=IntV[IntN].GetStr();
  }
  return ResChA;
}

TStr TStrUtil::GetStr(const TVec<TIntV>& IntIntV, const TStr& DelimiterStr) {
	TChA ResChA;
	for (int RowN = 0; RowN < IntIntV.Len(); RowN++) {
		for (int ColN = 0; ColN < IntIntV[RowN].Len(); ColN++) {
			ResChA += IntIntV[RowN][ColN].GetStr();
			if (ColN < IntIntV[RowN].Len() - 1) { ResChA += DelimiterStr; }
		}
		ResChA += "\n";
	}
	return ResChA;
}

TStr TStrUtil::GetStr(const TStrIntPrV& StrIntPrV, 
 const TStr& FieldDelimiterStr, const TStr& DelimiterStr) {
  TChA ResChA;
  for (int EltN = 0; EltN < StrIntPrV.Len(); EltN++) {
	if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
    ResChA+=StrIntPrV[EltN].Val1;
	ResChA+=FieldDelimiterStr;
    ResChA+=StrIntPrV[EltN].Val2.GetStr();
  }
  return ResChA;
}

TStr TStrUtil::GetStr(const TFltV& FltV, const TStr& DelimiterStr, const TStr& FmtStr) {
  TChA ResChA;
  for (int FltN = 0; FltN < FltV.Len(); FltN++) {
	if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
    ResChA+=TFlt::GetStr(FltV[FltN], FmtStr);
  }
  return ResChA;
}

TStr TStrUtil::GetStr(const TFltVV& FltVV, const TStr& DelimiterStr, const TStr& FmtStr) {
	TChA ResChA;

	for (int i = 0; i < FltVV.GetXDim(); i++) {
		for (int j = 0; j < FltVV.GetYDim(); j++) {
			ResChA += TFlt::GetStr(FltVV(i,j), FmtStr);
			if (j < FltVV.GetYDim() - 1) { ResChA += DelimiterStr; }
		}
		if (i < FltVV.GetXDim() - 1) { ResChA += '\n'; }
	}

	return ResChA;
}

TStr TStrUtil::GetStr(const TIntIntFltTrV& IntIntFltTrV, const TStr& DelimiterStr, const TStr& FmtStr) {
	TChA ResChA;

	for (int i = 0; i < IntIntFltTrV.Len(); i++) {
		const TIntIntFltTr& Val = IntIntFltTrV[i];

		ResChA += "(";
		ResChA += Val.Val1.GetStr() + ", " + Val.Val2.GetStr() + ", " + TFlt::GetStr(Val.Val3, FmtStr) + ")";
		if (i < IntIntFltTrV.Len()-1) {
			ResChA += DelimiterStr;
		}
	}

	return ResChA;
}

TStr TStrUtil::GetStr(const TIntFltKdV& IntFltKdV, const TStr& FieldDelimiterStr, 
 const TStr& DelimiterStr, const TStr& FmtStr) {
  TChA ResChA;
  for (int EltN = 0; EltN < IntFltKdV.Len(); EltN++) {
	if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
    ResChA+=IntFltKdV[EltN].Key.GetStr();
	ResChA+=FieldDelimiterStr;
    ResChA+=TFlt::GetStr(IntFltKdV[EltN].Dat, FmtStr);
  }
  return ResChA;
}

TStr TStrUtil::GetStr(const TStrV& StrV, const TStr& DelimiterStr) {
  return TStr::GetStr(StrV, DelimiterStr);
}

TStr TStrUtil::GetStr(const TStrH& StrH, const TStr& FieldDelimiterStr, const TStr& DelimiterStr) {
  if (StrH.Empty()) {return TStr();}
  TChA ResChA;
  int KeyId = StrH.FFirstKeyId();
  while (StrH.FNextKeyId(KeyId)) {
	if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
    ResChA+=StrH.GetKey(KeyId);
	ResChA+=FieldDelimiterStr;
	ResChA+=StrH[KeyId].GetStr();
  }
  return ResChA;
}
