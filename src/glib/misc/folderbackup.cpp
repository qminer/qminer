/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "folderbackup.h"

//
// TBackupLogInfo
//
TBackupLogInfo::TBackupLogInfo(const PJsonVal& Json)
{
    FolderName = Json->GetObjStr("folderName");
    SecsNeeded = Json->GetObjInt("secsNeeded");
    LogInfo = Json->GetObjStr("logInfo", "");
    LogDetails = Json->GetObjStr("logDetails", "");
}

TBackupLogInfo::TBackupLogInfo(const TStr& _FolderName, const TFlt _SecsNeeded, const TStr& _LogInfo, const TStr& _LogDetails):
    FolderName(_FolderName), SecsNeeded(_SecsNeeded), LogInfo(_LogInfo), LogDetails(_LogDetails)
{
}

PJsonVal TBackupLogInfo::GetJson() const
{
    PJsonVal RetJson = TJsonVal::NewObj("folderName", FolderName);
    RetJson->AddToObj("secsNeeded", SecsNeeded);
    RetJson->AddToObj("logInfo", LogInfo);
    RetJson->AddToObj("logDetails", LogDetails);
    return RetJson;
}

//
// TBackupProfile
// 
TBackupProfile::TBackupProfile(const PJsonVal& SettingsJson, const TStr& Destination_, const TStr& ProfileName_)
{
    Destination = Destination_;
    if (Destination.Len() > 0 && (Destination.LastCh() != '\\' || Destination.LastCh() != '/'))
        Destination += "/";
    ProfileName = ProfileName_;
    if (!TDir::Exists(Destination))
        TDir::GenDir(Destination);
    
    VersionsToKeep = SettingsJson->GetObjInt("versionsToKeep", 1);
    PJsonVal FoldersJson = SettingsJson->GetObjKey("folders");
    EAssertR(FoldersJson->IsArr(), "Expected to get an array of folders");
    for (int N = 0; N < FoldersJson->GetArrVals(); N++) {
        PJsonVal FolderJson = FoldersJson->GetArrVal(N);
        TBackupFolderInfo FolderInfo;
        FolderInfo.Folder = FolderJson->GetObjStr("folder");
        if (FolderJson->IsObjKey("extensions"))
            FolderJson->GetObjStrV("extensions", FolderInfo.Extensions);
        if (FolderInfo.Extensions.IsIn("*"))
            FolderInfo.Extensions.Clr();
        FolderInfo.IncludeSubfolders = FolderJson->GetObjBool("includeSubfolders");
        if (FolderJson->IsObjKey("skipIfContaining"))
            FolderJson->GetObjStrV("skipIfContaining", FolderInfo.SkipIfContainingV);
        FolderV.Add(FolderInfo);
    }
    
    // load logs of the previous backups
    ProfileLogFile = Destination + ProfileName + "/backupInfo.json";
    if (TFile::Exists(ProfileLogFile)) {
        PJsonVal LogJson = TJsonVal::GetValFromStr(TStr::LoadTxt(ProfileLogFile));
        if (LogJson->IsArr()) {
            for (int N = 0; N < LogJson->GetArrVals(); N++) {
                PJsonVal Log = LogJson->GetArrVal(N);
                LogV.Add(TBackupLogInfo(Log));
            }
        }
    }
}

TStr TBackupProfile::GetFolderNameForCurrentTime() const
{
    TTm Now = TTm::GetCurLocTm();
    return Now.GetYMDDashStr() + " " + TStr::Fmt("%02d-%02d-%02d", Now.GetHour(), Now.GetMin(), Now.GetSec());
}

// create the actual backup of a given profile
// additionally also check if we already have too many backups and remove the oldest one
TBackupLogInfo TBackupProfile::CreateBackup(const bool& ReportP)
{
    try {
        // create folder for the current backup
        if (!TDir::Exists(Destination + ProfileName))
            TDir::GenDir(Destination + ProfileName);
        const TStr DateFolderName = GetFolderNameForCurrentTime();
        const TStr FullDateFolder = Destination + ProfileName + "/" + DateFolderName + "/";
        if (!TDir::Exists(FullDateFolder))
            TDir::GenDir(FullDateFolder);

        TTmStopWatch StopWatch(true);

        TStr ErrMsgs, ErrDetails;
        for (int N = 0; N < FolderV.Len(); N++) {
            TStr ErrMsg, ErrDetail;
            // copy files
            CopyFolder(FullDateFolder, FolderV[N].Folder, FolderV[N].Extensions, FolderV[N].SkipIfContainingV, FolderV[N].IncludeSubfolders, ReportP, ErrMsg, ErrDetail);
            if (ErrMsg != "")
                ErrMsgs += (ErrMsgs.Len() > 0) ? "\n" + ErrMsg : ErrMsg;
            if (ErrDetails != "") {
                ErrDetails = (ErrDetails.Len() > 0) ? "\n" + ErrDetail : ErrDetail;
            }
        }
        const double Sec = StopWatch.GetSec();
        TStr OutText = ErrMsgs.Len() > 0 ? ErrMsgs : "Backup finished successfully.";
        
        // add a new log
        TBackupLogInfo BackupLogInfo(DateFolderName, Sec, OutText, ErrDetails);
        LogV.Add(BackupLogInfo);

        // if we have too many backups of the profile, delete the oldest one
        while (LogV.Len() > 1 && LogV.Len() > GetVersionsToKeep()) {
            const TStr FolderToDelete = Destination + ProfileName + "/" + LogV[0].GetFolderName();
            if (TDir::Exists(FolderToDelete))
                TDir::DelNonEmptyDir(FolderToDelete);
            LogV.Del(0);    // remove the first log item
        }
        
        // save the logs
        SaveLogs();

        return BackupLogInfo;
    }
    catch (PExcept E) {
        return TBackupLogInfo("", 0, E->GetMsgStr(), E->GetLocStr());
    }
    catch (...) {
        return TBackupLogInfo("", 0, "Unrecognized exception occured.", "");
    }
}

// copy files for a particular folder info
void TBackupProfile::CopyFolder(const TStr& BaseTargetFolder, const TStr& SourceFolder, const TStrV& Extensions, const TStrV& SkipIfContainingV, const bool& IncludeSubfolders, const bool& ReportP, TStr& ErrMsg, TStr& ErrDetails)
{
    try {
        // get the name of the source folder
        TStrV PathV; TDir::SplitPath(SourceFolder, PathV);
        EAssert(PathV.Len() > 0);

        // create the folder in the base target folder
        TStr TargetFolder = BaseTargetFolder + PathV[PathV.Len() - 1] + "/";
        if (!TDir::Exists(TargetFolder))
            TDir::GenDir(TargetFolder);

        // find files to be copied
        TStrV FileV;
        TFFile::GetFNmV(SourceFolder, Extensions, false, FileV);

        TStrV FolderV;
        // copy them
        for (int N = 0; N < FileV.Len(); N++) {
            // we found a file
            if (TFile::Exists(FileV[N])) {
                const TStr FileName = TDir::GetFileName(FileV[N]);
                // is this a file that we wish to ignore?
                bool ShouldCopy = true;
                for (int S = 0; S < SkipIfContainingV.Len(); S++) {
                    if (FileName.SearchStr(SkipIfContainingV[S]) >= 0)
                        ShouldCopy = false;
                }
                if (!ShouldCopy)
                    continue;
                const TStr TargetFNm = TargetFolder + FileName;
                if (ReportP)
                    TNotify::StdNotify->OnStatusFmt("Copying file: %s\r", FileName.CStr());
                TFile::Copy(FileV[N], TargetFNm);
            }
            // we found a folder
            else {
                FolderV.Add(FileV[N]);
            }
        }

        if (IncludeSubfolders) {
            for (int N = 0; N < FolderV.Len(); N++)
                CopyFolder(TargetFolder, FolderV[N], Extensions, SkipIfContainingV, IncludeSubfolders, ReportP, ErrMsg, ErrDetails);
        }
    }
    catch (PExcept E) {
        if (ErrMsg != "")
            ErrMsg += "\n";
        ErrMsg += "Exception while copying from " + SourceFolder + ": " + E->GetMsgStr();
        ErrDetails = E->GetLocStr();
    }
    catch (...) {
        if (ErrMsg != "")
            ErrMsg += "\n";
        ErrMsg += "Exception while copying from " + SourceFolder + ": " + "Unrecognized exception occured.";
        ErrDetails = "";
    }
}


void TBackupProfile::Restore(const TStr& BackupFolderName, const ERestoringMode& RestoringMode, const bool& ReportP) const
{
    for (int N = 0; N < LogV.Len(); N++) {
        // find the folder that matches the BackupFolderName
        if (LogV[N].GetFolderName() == BackupFolderName) {
            const TVec<TBackupFolderInfo> Folders = GetFolders();
            for (int N = 0; N < Folders.Len(); N++) {
                const TStr TargetFolder = Folders[N].Folder;
                TStrV PartV; TDir::SplitPath(TargetFolder, PartV);
                const TStr LastFolderNamePart = PartV[PartV.Len() - 1];

                // do we want to first remove any existing data in the target folder?
                if (RestoringMode == RemoveExistingFirst)
                    TDir::DelNonEmptyDir(TargetFolder);

                // copy data from backup to the destination folder
                const TStr SourceFolder = Destination + ProfileName + "/" + BackupFolderName + "/" + LastFolderNamePart;
                if (ReportP)
                    TNotify::StdNotify->OnStatusFmt("Copying folder: %s", SourceFolder.CStr());
                if (TDir::Exists(SourceFolder))
                    TDir::CopyDir(SourceFolder, TargetFolder, RestoringMode == OverwriteIfExisting);
                else
                    TNotify::StdNotify->OnStatusFmt("WARNING: Unable to create a restore of the folder %s. The folder does not exist.", SourceFolder.CStr());
            }
        }
    }
}

// save the logs of the backups to the log file
void TBackupProfile::SaveLogs() const
{
    TJsonValV ArrV;
    for (int N = 0; N < LogV.Len(); N++)
        ArrV.Add(LogV[N].GetJson());
    PJsonVal JsonArr = TJsonVal::NewArr(ArrV);
    TStr JsonStr = JsonArr->SaveStr();
    JsonStr.SaveTxt(ProfileLogFile);
}


//
// TFolderBackup
//
TFolderBackup::TFolderBackup(const TStr& SettingsFNm, const bool& _ReportP)
{
    ReportP = _ReportP;
    if (!TFile::Exists(SettingsFNm)) {
        TNotify::StdNotify->OnStatusFmt("Unable to load settings file name for TBackupData. File %s is missing.", SettingsFNm.CStr());
        return;
    }
    TStr JsonData = TStr::LoadTxt(SettingsFNm);
    PJsonVal SettingsJson = TJsonVal::GetValFromStr(JsonData);
    ParseSettings(SettingsJson);
}

TFolderBackup::TFolderBackup(const PJsonVal& SettingsJson, const bool& _ReportP)
{
    ReportP = _ReportP;
    ParseSettings(SettingsJson);
}

void TFolderBackup::ParseSettings(const PJsonVal& SettingsJson)
{
    // name of the file that holds all backups
    DestinationDirNm = SettingsJson->GetObjStr("destination");
    
    // load profiles for which we wish to make backups
    PJsonVal ProfilesJson = SettingsJson->GetObjKey("profiles");
    if (!ProfilesJson->IsObj()) {
        TNotify::StdNotify->OnStatus("'profiles' object is not an object");
        return;
    }
    for (int N = 0; N < ProfilesJson->GetObjKeys(); N++) {
        const TStr ProfileName = ProfilesJson->GetObjKey(N);
        ProfileH.AddDat(ProfileName, TBackupProfile(ProfilesJson->GetObjKey(ProfileName), DestinationDirNm, ProfileName));
    }
}

// backup a particular profile
TBackupLogInfo TFolderBackup::CreateBackup(const TStr& ProfileName)
{
    if (ProfileH.IsKey(ProfileName)) {
        // execute the backup
        TBackupProfile Profile = ProfileH.GetDat(ProfileName);
        TBackupLogInfo Info = Profile.CreateBackup(ReportP);
        return Info;
    }
    else {
        TNotify::StdNotify->OnStatusFmt("Unable to find profile with name %s", ProfileName.CStr());
        return TBackupLogInfo("", 0, TStr::Fmt("Unable to find profile with the name %s", ProfileName.CStr()), "");
    }
}

// backup all profiles
void TFolderBackup::CreateBackup(TVec<TBackupLogInfo>& BackupLogInfo)
{
    for (int KeyId = ProfileH.FFirstKeyId(); ProfileH.FNextKeyId(KeyId);) {
        const TStr ProfileName = ProfileH.GetKey(KeyId);
        TBackupLogInfo Info = CreateBackup(ProfileName);
        BackupLogInfo.Add(Info);
    }
}

// return the list of folders containing backups for a given profile name
// folders are sorted from the oldest to the newest
void TFolderBackup::GetBackupFolders(const TStr& ProfileName, TStrV& FolderNmV) const
{
    FolderNmV.Clr();
    if (ProfileH.IsKey(ProfileName)) {
        TBackupProfile Profile = ProfileH.GetDat(ProfileName);
        const TVec<TBackupLogInfo>& LogV = Profile.GetLogs();
        for (int N = 0; N < LogV.Len(); N++)
            FolderNmV.Add(LogV[N].GetFolderName());
    }
}

int TFolderBackup::GetBackupCount(const TStr& ProfileName) const
{
    if (ProfileH.IsKey(ProfileName)) {
        TBackupProfile Profile = ProfileH.GetDat(ProfileName);
        return Profile.GetLogs().Len();
    }
    return 0;
}

// restore a previous backup
// backup folder name is the timestamp of the backed up folder (YYYY-MM-DD HH-MM-SS)
// you can get the folder by calling the GetBackupFolders() method
void TFolderBackup::Restore(const TStr& ProfileName, const TStr& BackupFolderName, const TBackupProfile::ERestoringMode& RestoringMode) const
{
    if (ProfileH.IsKey(ProfileName))
        ProfileH.GetDat(ProfileName).Restore(BackupFolderName, RestoringMode, ReportP);
}

bool TFolderBackup::RestoreLatest(const TStr& ProfileName, const TBackupProfile::ERestoringMode& RestoringMode) const
{
    if (ProfileH.IsKey(ProfileName)) {
        TStrV FolderV; GetBackupFolders(ProfileName, FolderV);
        if (FolderV.Len() > 0) {
            ProfileH.GetDat(ProfileName).Restore(FolderV[FolderV.Len()-1], RestoringMode, ReportP);
            return true;
        }
    }
    return false;
}
