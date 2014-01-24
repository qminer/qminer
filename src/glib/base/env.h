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
// Environment
class TEnv{
private:
  TStrV ArgV;
  TStr HdStr;
  int MnArgs;
  bool SilentP;
  bool LineP;
  PNotify Notify;
public:
  TEnv(): ArgV(), HdStr(), MnArgs(1), SilentP(false), LineP(false), Notify(){}
  TEnv(const TEnv&);
  TEnv(const int& _Args, char** _ArgV, const PNotify& _Notify=NULL);
  TEnv(const TStr& _ArgStr, const PNotify& _Notify=NULL);

  TEnv& operator=(const TEnv& Env){
    ArgV=Env.ArgV; HdStr=Env.HdStr; MnArgs=Env.MnArgs;    
    SilentP=Env.SilentP; Notify=Env.Notify; return *this;}

  // basic arguments & command line
  int GetArgs() const {return ArgV.Len();}
  TStr GetArg(const int& ArgN) const {
    IAssert((0<=ArgN)&&(ArgN<GetArgs())); return ArgV[ArgN];}
  const TStrV& GetArgV() const { return ArgV; }
  TStr GetExeFNm() const;
  TStr GetExeFPath() const {return GetExeFNm().GetFPath();}
  TStr GetCmLn(const int& FromArgN=1) const;
  bool IsSilent() const {return SilentP;}
  void SetNoLine() {LineP=false;}
  void SetSilent() {SilentP=true;}

  // arguments prefix & postfix
  int GetPrefixArgN(const TStr& PrefixStr) const;
  bool IsArgPrefix(const TStr& PrefixStr) const {
    return GetPrefixArgN(PrefixStr)!=-1;}
  TStr GetArgPostfix(const TStr& PrefixStr) const;

  // top level arguments handling
  void PrepArgs(
   const TStr& _HdStr=TStr(), const int& _MnArgs=1, const bool& SilentP=false);
  TStr GetHdStr() const {return HdStr;}
  int GetMnArgs() const {return MnArgs;}
  bool IsEndOfRun() const;

  // arguments value retrieval
  bool IsArgStr(const TStr& ArgStr);
  bool IsArgStr(const TStr& ArgStr, const TStr& DNm);
  TStr GetIfArgPrefixStr(const TStr& PrefixStr, const TStr& DfVal="", const TStr& DNm="") const;
  TStrV GetIfArgPrefixStrV(const TStr& PrefixStr, TStrV& DfValV, const TStr& DNm="") const;
  TStrV GetIfArgPrefixStrV(const TStr& PrefixStr, const TStr& DNm="") const {
    TStrV DfValV; return GetIfArgPrefixStrV(PrefixStr, DfValV, DNm);}
  TStrSet GetIfArgPrefixStrSet(const TStr& PrefixStr, TStrV& DfValV, const TStr& DNm="") const {
    return TStrSet(GetIfArgPrefixStrV(PrefixStr, DfValV, DNm));}
  TStrSet GetIfArgPrefixStrSet(const TStr& PrefixStr, const TStr& DNm="") const {
    return TStrSet(GetIfArgPrefixStrV(PrefixStr, DNm));}
  bool GetIfArgPrefixBool(const TStr& PrefixStr, const bool& DfVal=false, const TStr& DNm="") const;
  int GetIfArgPrefixInt(const TStr& PrefixStr, const int& DfVal=0, const TStr& DNm="") const;
  TIntV GetIfArgPrefixIntV(const TStr& PrefixStr, TIntV& DfValV, const TStr& DNm="") const;
  TIntV GetIfArgPrefixIntV(const TStr& PrefixStr, const TStr& DNm="") const {
    TIntV DfValV; return GetIfArgPrefixIntV(PrefixStr, DfValV, DNm);}
  double GetIfArgPrefixFlt(const TStr& PrefixStr, const double& DfVal=0, const TStr& DNm="") const;

  // environment variables
  static void PutVarVal(const TStr& VarNm, const TStr& VarVal);
  static TStr GetVarVal(const TStr& VarNm);
  void GetVarNmV(TStrV& VarNmV);
  void GetVarNmValV(TStrV& NmValV);
};

extern TEnv Env;

