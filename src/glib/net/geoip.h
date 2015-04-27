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
// Geographical-IP-Location-Descriptor
class TGeoIpLocDesc{
public:
  TStr CountryNm;
  TStr RegionNm;
  TStr CityNm;
  TStr PostalCode;
  TFlt Latitude;
  TFlt Longitude;
  TStr MetroCode;
  TStr AreaCode;
public:
  TGeoIpLocDesc(){}
  TGeoIpLocDesc(TSIn& SIn):
    CountryNm(SIn), RegionNm(SIn), CityNm(SIn), PostalCode(SIn), 
    Latitude(SIn), Longitude(SIn), MetroCode(SIn), AreaCode(SIn){}
  void Save(TSOut& SOut) const {
    CountryNm.Save(SOut); RegionNm.Save(SOut); CityNm.Save(SOut); 
    PostalCode.Save(SOut); Latitude.Save(SOut); Longitude.Save(SOut); 
    MetroCode.Save(SOut); AreaCode.Save(SOut);}

  TGeoIpLocDesc& operator=(const TGeoIpLocDesc& LocDesc){
    if (this!=&LocDesc){
      CountryNm=LocDesc.CountryNm; RegionNm=LocDesc.RegionNm; CityNm=LocDesc.CityNm;
      PostalCode=LocDesc.PostalCode; Latitude=LocDesc.Latitude; Longitude=LocDesc.Longitude;
      MetroCode=LocDesc.MetroCode; AreaCode=LocDesc.AreaCode;}
    return *this;}
};

/////////////////////////////////////////////////
// Geographical-IP-Organization-Descriptor
class TGeoIpOrgDesc{
public:
  TUInt MxIpNum;
  TInt IspNmId;
  TInt OrgNmId;
  TInt LocId;
public:
  TGeoIpOrgDesc(){}
  TGeoIpOrgDesc(TSIn& SIn):
    MxIpNum(SIn), IspNmId(SIn), OrgNmId(SIn), LocId(SIn){}
  void Save(TSOut& SOut) const {
    MxIpNum.Save(SOut); IspNmId.Save(SOut); OrgNmId.Save(SOut); LocId.Save(SOut);}

  TGeoIpOrgDesc& operator=(const TGeoIpOrgDesc& OrgDesc){
    if (this!=&OrgDesc){
      MxIpNum=OrgDesc.MxIpNum; IspNmId=OrgDesc.IspNmId; OrgNmId=OrgDesc.OrgNmId; LocId=OrgDesc.LocId;}
    return *this;}
};

/////////////////////////////////////////////////
// Geographical-IP
ClassTP(TGeoIpBs, PGeoIpBs)//{
private:
  TStrStrH CountrySNmToLNmH;
  THash<TInt, TGeoIpLocDesc> LocIdToLocDescH;
  TUIntV OrgMnIpNumV;
  THash<TUInt, TGeoIpOrgDesc> MnIpNumToOrgDescH;
  TStrHash<TInt> StrH;
  UndefCopyAssign(TGeoIpBs);
public:
  TGeoIpBs():
    CountrySNmToLNmH(), LocIdToLocDescH(), 
    OrgMnIpNumV(), MnIpNumToOrgDescH(), StrH(){}
  static PGeoIpBs New(){return new TGeoIpBs();}
  ~TGeoIpBs(){}
  TGeoIpBs(TSIn& SIn) {
    CountrySNmToLNmH.Load(SIn); LocIdToLocDescH.Load(SIn);
    OrgMnIpNumV.Load(SIn); MnIpNumToOrgDescH.Load(SIn);
    StrH.Load(SIn);
  }
  static PGeoIpBs Load(TSIn& SIn){return new TGeoIpBs(SIn);}
  void Save(TSOut& SOut){
    CountrySNmToLNmH.Save(SOut); LocIdToLocDescH.Save(SOut);
    OrgMnIpNumV.Save(SOut); MnIpNumToOrgDescH.Save(SOut); StrH.Save(SOut);}

  // organization search
  int GetOrgId(const TStr& IpNumStr);
  int GetOrgId(const uint& IpNum);

  // organization context
  TStr GetOrgNm(const int& OrgId) const {return StrH.GetKey(MnIpNumToOrgDescH[OrgId].OrgNmId);}
  TStr GetIspNm(const int& OrgId) const {return StrH.GetKey(MnIpNumToOrgDescH[OrgId].IspNmId);}
  TStr GetCountryNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).CountryNm;}
  TStr GetRegionNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).RegionNm;}
  TStr GetCityNm(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).CityNm;}
  TStr GetPostalCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).PostalCode;}
  TFlt GetLatitude(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).Latitude;}
  TFlt GetLongitude(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).Longitude;}
  TStr GetMetroCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).MetroCode;}
  TStr GetAreaCode(const int& OrgId) const {return LocIdToLocDescH.GetDat(MnIpNumToOrgDescH[OrgId].LocId).AreaCode;}

  static void WrOrgInfo(const PGeoIpBs& GeoIpBs, const TStr& IpNumStr);

  // files
  static PGeoIpBs LoadCsv(const TStr& GeoIpFPath);
  static PGeoIpBs LoadBin(const TStr& FNm);
  void SaveBin(const TStr& FNm){
    TFOut SOut(FNm); Save(SOut);}
};