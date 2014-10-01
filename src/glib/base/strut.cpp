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
