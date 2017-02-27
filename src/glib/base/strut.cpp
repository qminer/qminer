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

TStr TStrUtil::GetStr(const TUInt64V& UInt64V, const TStr& DelimiterStr) {
	TChA ResChA;
	for (int N = 0; N < UInt64V.Len(); N++) {
		if (!ResChA.Empty()) { ResChA += DelimiterStr; }
		ResChA += UInt64V[N].GetStr();
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

TStr TStrUtil::GetStr(const TUInt64IntPrV& UInt64IntPrV, const TStr& FieldDelimiterStr,
		const TStr& DelimiterStr) {
	TChA ResChA;
	for (int i = 0; i < UInt64IntPrV.Len(); i++) {
		if (!ResChA.Empty()) { ResChA += DelimiterStr; }
		ResChA += UInt64IntPrV[i].Val1.GetStr();
		ResChA += FieldDelimiterStr;
		ResChA += UInt64IntPrV[i].Val2.GetStr();
	}
	return ResChA;
}

TStr TStrUtil::GetStr(const TIntFltPrV& IntFltPrV, const TStr& FieldDelimiterStr,
		const TStr& DelimiterStr, const TStr& FmtStr) {
	TChA ResChA;
	for (int i = 0; i < IntFltPrV.Len(); i++) {
		if (!ResChA.Empty()) { ResChA += DelimiterStr; }
		ResChA += IntFltPrV[i].Val1.GetStr();
		ResChA += FieldDelimiterStr;
		ResChA += TFlt::GetStr(IntFltPrV[i].Val2, FmtStr);
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

TStr TStrUtil::GetStr(const TUInt64UInt64IntTrV& UInt64UInt64IntTrV,
		const TStr& DelimiterStr) {
	TChA ResChA;
	for (int ValN = 0; ValN < UInt64UInt64IntTrV.Len(); ValN++) {
		const TUInt64UInt64IntTr& Val = UInt64UInt64IntTrV[ValN];

		ResChA += "(";
		ResChA += Val.Val1.GetStr() + ", " + Val.Val2.GetStr() + ", " + Val.Val3.GetStr() + ")";
		if (ValN < UInt64UInt64IntTrV.Len()-1) {
			ResChA += DelimiterStr;
		}
	}
	return ResChA;
}

TStr TStrUtil::GetStr(const TIntSet& IntSet, const TStr& DelimiterStr) {
	TChA ResChA;

	int KeyId = IntSet.FFirstKeyId();
	while (IntSet.FNextKeyId(KeyId)) {
		if (!ResChA.Empty()) { ResChA+=DelimiterStr; }
		const TInt& Val = IntSet[KeyId];
		ResChA += Val.GetStr();
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


TStr TStrUtil::GetStr(const int& Val, const TStr& ThousandDelimiterStr)
{
    TStr StrVal = TInt(Val).GetStr();
    int Ind = 0;
    for (int N = StrVal.Len() - 1; N > 0; N--) {
        Ind++;
        if (Ind % 3 == 0) {
            StrVal.InsStr(N, ThousandDelimiterStr);
        }
    }
    return StrVal;
}