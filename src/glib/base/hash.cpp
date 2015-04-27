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
// Big-String-Pool
void TBigStrPool::Resize(TSize _MxBfL) {
  TSize newSize = MxBfL;
  while (newSize < _MxBfL) {
    if (newSize >= GrowBy && GrowBy > 0) newSize += GrowBy;
    else if (newSize > 0) newSize *= 2;
    else newSize = TInt::GetMn(GrowBy, 1024);
  }
  if (newSize > MxBfL) {
    Bf = (char *) realloc(Bf, newSize);
    IAssertR(Bf, TStr::Fmt("old Bf size: %u, new size: %u", MxBfL, newSize).CStr());
    MxBfL = newSize;
  }
  IAssert(MxBfL >= _MxBfL);
}

TBigStrPool::TBigStrPool(TSize MxBfLen, uint _GrowBy) : MxBfL(MxBfLen), BfL(0), GrowBy(_GrowBy), Bf(0) {
  //IAssert(MxBfL >= 0); IAssert(GrowBy >= 0);
  if (MxBfL > 0) { Bf = (char *) malloc(MxBfL);  IAssert(Bf); }
  AddStr(""); // add empty string
}

TBigStrPool::TBigStrPool(TSIn& SIn, bool LoadCompact) : MxBfL(0), BfL(0), GrowBy(0), Bf(0) {
  uint64 Tmp;
  SIn.Load(Tmp); IAssert(Tmp <= uint64(TSizeMx)); MxBfL=TSize(Tmp);
  SIn.Load(Tmp); IAssert(Tmp <= uint64(TSizeMx)); BfL=TSize(Tmp);
  SIn.Load(GrowBy);
  IAssert(MxBfL >= BfL);
  if (LoadCompact) MxBfL = BfL;
  if (MxBfL > 0) { Bf = (char *) malloc(MxBfL); IAssert(Bf); }
  if (BfL > 0) { SIn.LoadBf(Bf, BfL); }
  SIn.LoadCs();
  int NStr=0;  SIn.Load(NStr);
  IdOffV.Gen(NStr, 0);
  for (int i = 0; i < NStr; i++) {
    SIn.Load(Tmp);
    IAssert(Tmp <= uint64(TSizeMx));
    IdOffV.Add(TSize(Tmp));
  }
}

void TBigStrPool::Save(TSOut& SOut) const {
  SOut.Save(uint64(MxBfL));  SOut.Save(uint64(BfL));  SOut.Save(GrowBy);
  if (BfL > 0) { SOut.SaveBf(Bf, BfL); }
  SOut.SaveCs();
  SOut.Save(IdOffV.Len());
  for (int i = 0; i < IdOffV.Len(); i++) {
    SOut.Save(uint64(IdOffV[i])); 
  }
}

TBigStrPool& TBigStrPool::operator = (const TBigStrPool& Pool) {
  if (this != &Pool) {
    GrowBy = Pool.GrowBy;  MxBfL = Pool.MxBfL;  BfL = Pool.BfL;
    if (Bf) free(Bf); else IAssert(MxBfL == 0);
    Bf = (char *) malloc(MxBfL);  IAssert(Bf);  memcpy(Bf, Pool.Bf, BfL);
  }
  return *this;
}

// Adds Len characters to pool. To append a null terminated string Len must be equal to strlen(s) + 1
int TBigStrPool::AddStr(const char *Str, uint Len) {
  IAssertR(Len > 0, "String too short (lenght includes the null character)");  //J: if (! Len) return -1;
  Assert(Str);  Assert(Len > 0);
  if (Len == 1 && IdOffV.Len() > 0) { return 0; } // empty string
  if (BfL + Len > MxBfL) { Resize(BfL + Len); }
  memcpy(Bf + BfL, Str, Len);
  TSize Pos = BfL;  BfL += Len;  
  IdOffV.Add(Pos);
  return IdOffV.Len()-1;
}

int TBigStrPool::GetPrimHashCd(const char *CStr) {
  return TStrHashF_DJB::GetPrimHashCd(CStr);
}

int TBigStrPool::GetSecHashCd(const char *CStr) {
  return TStrHashF_DJB::GetSecHashCd(CStr);
}

/////////////////////////////////////////////////
// String-Hash-Functions

// Md5-Hash-Function
int TStrHashF_Md5::GetPrimHashCd(const char *p) {
  TMd5Sig sig = TStr(p); 
  return sig.GetPrimHashCd(); 
}

int TStrHashF_Md5::GetSecHashCd(const char *p) {
  TMd5Sig sig = TStr(p);
  return sig.GetSecHashCd(); 
}

int TStrHashF_Md5::GetPrimHashCd(const TStr& s) {
  TMd5Sig sig(s);
  return sig.GetPrimHashCd(); 
}

int TStrHashF_Md5::GetSecHashCd(const TStr& s) {
  TMd5Sig sig(s);
  return sig.GetSecHashCd();
}
