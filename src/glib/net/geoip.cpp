/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Organization-Geographical-IP
int TGeoIpBs::GetOrgId(const TStr& IpNumStr){
  uint IpNum;
  if (TUInt::IsIpStr(IpNumStr, IpNum)) {    
    return GetOrgId(IpNum);
  } else {
    return -1;
  }
}

int TGeoIpBs::GetOrgId(const uint& IpNum){
  // get location-id from ip-number
  int OrgId=-1;
  int IpNumN; OrgMnIpNumV.SearchBin(IpNum+1, IpNumN);
  if (IpNumN>0){
    uint MnIpNum=OrgMnIpNumV[IpNumN-1];
    uint MxIpNum=MnIpNumToOrgDescH.GetDat(MnIpNum).MxIpNum;
    if ((MnIpNum<=IpNum)&&(IpNum<=MxIpNum)){
      OrgId=MnIpNumToOrgDescH.GetKeyId(MnIpNum);
    }
  }
  return OrgId;
}

void TGeoIpBs::WrOrgInfo(const PGeoIpBs& GeoIpBs, const TStr& IpNumStr){
  int OrgId=GeoIpBs->GetOrgId(IpNumStr);
  if (OrgId==-1){
    printf("Wrong IP\n");
  } else {
    printf("Org: %s\n", GeoIpBs->GetOrgNm(OrgId).CStr());
    printf("ISP: %s\n", GeoIpBs->GetIspNm(OrgId).CStr());
    printf("Country: %s\n", GeoIpBs->GetCountryNm(OrgId).CStr());
    printf("Region: %s\n", GeoIpBs->GetRegionNm(OrgId).CStr());
    printf("City: %s\n", GeoIpBs->GetCityNm(OrgId).CStr());
    printf("Postal: %s\n", GeoIpBs->GetPostalCode(OrgId).CStr());
	printf("Latitude: %f\n", GeoIpBs->GetLatitude(OrgId).Val);
	printf("Longitude: %f\n", GeoIpBs->GetLongitude(OrgId).Val);
    printf("Metro: %s\n", GeoIpBs->GetMetroCode(OrgId).CStr());
    printf("Area: %s\n", GeoIpBs->GetAreaCode(OrgId).CStr());
  }
}

PGeoIpBs TGeoIpBs::LoadCsv(const TStr& GeoIpFPath){
  PGeoIpBs GeoIpBs=TGeoIpBs::New();
  // filenames
  TStr GeoIpNrFPath=TStr::GetNrFPath(GeoIpFPath);
  TStr LocFNm=GeoIpNrFPath+"GeoIPCity-144-Location.csv";
  TStr OrgFNm=GeoIpNrFPath+"GeoIPCityISPOrg-144.csv";

  // location data
  {PSs Ss=TSs::LoadTxt(ssfCommaSep, LocFNm, TNotify::StdNotify, false);
  printf("Load %s ...\n", LocFNm.CStr());
  for (int Y=2; Y<Ss->GetYLen(); Y++){
    if (Y%1000==0){printf("%d\r", Y);}
    int LocId=Ss->At(0, Y).GetInt();
    TGeoIpLocDesc& LocDesc=GeoIpBs->LocIdToLocDescH.AddDat(LocId);
    LocDesc.CountryNm=Ss->At(1, Y);
    LocDesc.RegionNm=Ss->At(2, Y);
    LocDesc.CityNm=Ss->At(3, Y);
    LocDesc.PostalCode=Ss->At(4, Y);
    LocDesc.Latitude=Ss->At(5, Y).GetFlt();
    LocDesc.Longitude=Ss->At(6, Y).GetFlt();
    LocDesc.MetroCode=Ss->At(7, Y);
    LocDesc.AreaCode=Ss->At(8, Y);
  }
  printf("\nDone.\n");}

  // organization data
  {PSs Ss=TSs::LoadTxt(ssfCommaSep, OrgFNm, TNotify::StdNotify, false);
  printf("Load %s ...\n", OrgFNm.CStr());
  for (int Y=2; Y<Ss->GetYLen(); Y++){
    if (Y%1000==0){printf("%d\r", Y);}
    TUInt MnIpNum=Ss->At(0, Y).GetUInt();
    TGeoIpOrgDesc& OrgDesc=GeoIpBs->MnIpNumToOrgDescH.AddDat(MnIpNum);
    OrgDesc.MxIpNum=Ss->At(1, Y).GetUInt();
    OrgDesc.LocId=Ss->At(2, Y).GetInt();
    TStr IspNm=Ss->At(3, Y);
    OrgDesc.IspNmId=GeoIpBs->StrH.AddKey(IspNm);
    TStr OrgNm=Ss->At(4, Y);
    OrgDesc.OrgNmId=GeoIpBs->StrH.AddKey(OrgNm);
    GeoIpBs->OrgMnIpNumV.Add(MnIpNum);
  }
  printf("\nDone.\n");}
  printf("Sorting IPs... ");
  GeoIpBs->OrgMnIpNumV.Sort();
  printf("Done.\n");

  // return geoip base
  return GeoIpBs;
}

PGeoIpBs TGeoIpBs::LoadBin(const TStr& FNm){
    // load and return if exists
    if (TFile::Exists(FNm)) {
        TFIn SIn(FNm); return Load(SIn);
    } 
    // otherwise assume we have CSV and we need to parse it first
    printf("Cannot fine %s, loading from raw files\n", FNm.CStr());
    TStr FPath = FNm.GetFPath() + "/GeoIP/";
    PGeoIpBs GeoIpBs = LoadCsv(FPath);
    GeoIpBs->SaveBin(FNm);
    return GeoIpBs;
}
