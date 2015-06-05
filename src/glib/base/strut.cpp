/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
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
//Explicit 64 bit indexing
TStr TStrUtil::GetStr(const TVec<TNum<int64>, int64>& IntV, const TStr& DelimiterStr) {
	TChA ResChA;
	for (int64 IntN = 0; IntN < IntV.Len(); IntN++) {
		if (!ResChA.Empty()) { ResChA += DelimiterStr; }
		ResChA += IntV[IntN].GetStr();
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
