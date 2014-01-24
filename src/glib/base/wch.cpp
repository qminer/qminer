/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institut d.o.o.
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
// Wide-Char
const TWCh TWCh::Mn(0, 0);
const TWCh TWCh::Mx(0xFF, 0xFF);
const int TWCh::Vals=0x10000;

const TWCh TWCh::StartWCh(0xFE, 0xFF);
const TWCh TWCh::TabWCh(TCh::TabCh);
const TWCh TWCh::LfWCh(TCh::LfCh);
const TWCh TWCh::CrWCh(TCh::CrCh);
const TWCh TWCh::SpaceWCh(' ');

/////////////////////////////////////////////////
// Wide-Char-Array
void TWChA::AddCStr(const char* CStr){
  int CStrLen=int(strlen(CStr));
  for (int ChN=0; ChN<CStrLen; ChN++){
    WChV.Add(TWCh(CStr[ChN]));}
}

void TWChA::PutCStr(const char* CStr){
  int CStrLen=int(strlen(CStr));
  WChV.Gen(CStrLen, 0);
  for (int ChN=0; ChN<CStrLen; ChN++){
    WChV.Add(TWCh(CStr[ChN]));}
}

TStr TWChA::GetStr() const {
  TChA ChA;
  for (int WChN=0; WChN<WChV.Len(); WChN++){
    ChA+=WChV[WChN].GetCh();}
  return ChA;
}

void TWChA::LoadTxt(const PSIn& SIn, TWChA& WChA){
  WChA.Clr();
  TWCh WCh=TWCh::LoadTxt(SIn);
  IAssert(WCh==TWCh::StartWCh);
  while (!SIn->Eof()){
    WChA+=TWCh::LoadTxt(SIn);}
}

void TWChA::SaveTxt(const PSOut& SOut) const {
  TWCh::StartWCh.SaveTxt(SOut);
  for (int WChN=0; WChN<WChV.Len(); WChN++){
    WChV[WChN].SaveTxt(SOut);}
}

TWChA TWChA::EmptyWChA;

