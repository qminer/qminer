/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _FOLDERBACKUP
#define _FOLDERBACKUP

#include <base.h>

/*
Functionality in this file provides one the possibility to create backups of a set of folders.
A typical use case would include creating regular backups of a given folder with user's data. 

One would start by creating a json file such as the one provided at the bottom of the file. In the file you 
can specify one or more backup profiles. A backup profile is a unit for which the backup is generated and stored in a single location.
In the given example, the "er" is the name of the profile and it will backup two folders (...\IndexArticles and ...\IndexGeo).
When TFolderBackup::CreateBackup() or TFolderBackup::CreateBackup("er") will be called, a new folder will be created in 
E:\\data\\NewsMiner\\_AutoBackups such as "2014-11-05 15-17-13". Inside this folder it will copy the folders IndexArticles and IndexGeo.
The "versionsToKeep" parameter for a profile determines how many backups to keep. If set to 3 it will always keep the latest 3 backup folders.
When the backup will be called for the 4th (and 5th, ...) time, the oldest folder will also be removed afterwards.

Bug reports can be sent to gleban@gmail.com
*/


// information about past backup operations
class TBackupLogInfo
{
private:
    TStr LogInfo;       // info about the success or failure of the backing up
    TStr LogDetails;    // empty or contains a stack trace of the error
    TFlt SecsNeeded;    // how long did we need to make the backup (in seconds)
    TStr FolderName;    // name of the folder in which we have stored the backup data

public:
    TBackupLogInfo() {}
    TBackupLogInfo(const PJsonVal& Json);
    TBackupLogInfo(const TStr& FolderName, const TFlt SecsNeeded, const TStr& LogInfo, const TStr& LogDetails);
    PJsonVal GetJson() const;
    TStr GetFolderName() const { return FolderName; }
};

// info about the folders to backup
class TBackupFolderInfo
{
public:
    TStr Folder;                // folder to backup
    TStrV Extensions;           // list of extensions to backup. empty = backup all. specify only extensions (bin and not *.bin)
    TBool IncludeSubfolders;    // should subfolders also be backedup
    TStrV SkipIfContainingV;    // if a part of the full filename will match any text in this list, the file will not be backedup. Case sensitive!
};

class TBackupProfile
{
private:
    TStr Destination;
    TStr ProfileName;
    TStr ProfileLogFile;
    TInt VersionsToKeep;
    TVec<TBackupFolderInfo> FolderV;
    TVec<TBackupLogInfo> LogV;

    TStr GetFolderNameForCurrentTime() const;
    void CopyFolder(const TStr& BaseTargetFolder, const TStr& SourceFolder, const TStrV& Extensions, const TStrV& SkipIfContainingV, const bool& IncludeSubfolders, const bool& ReportP, TStr& ErrMsg, TStr& ErrDetails);
    
    void SaveLogs() const;

public:

    enum ERestoringMode { RemoveExistingFirst, SkipIfExisting, OverwriteIfExisting };

    TBackupProfile() {}
    TBackupProfile(const PJsonVal& SettingsJson, const TStr& DestinationDirNm, const TStr& ProfileName);
    TBackupLogInfo CreateBackup(const bool& ReportP);
    void Restore(const TStr& BackupFolderName, const ERestoringMode& RestoringMode, const bool& ReportP) const;

    int GetVersionsToKeep() const { return VersionsToKeep; }
    TStr GetDestination() const { return Destination; }
    const TVec<TBackupFolderInfo> GetFolders() const { return FolderV; }
    const TVec<TBackupLogInfo> GetLogs() const { return LogV; }
};


// main class for doing data backup
// it uses a settings file containing one or more profiles. each profile has a set of folders to backup
// calling CreateBackup creates a new backup and possibly removes old backup folder (depending on the value of "versionsToKeep")
// it also stores the backup information in a "backupLogFile" file
class TFolderBackup
{
private:
    TStr DestinationDirNm;
    bool ReportP;

    THash<TStr, TBackupProfile> ProfileH;
    
    void ParseSettings(const PJsonVal& SettingsJson);

public:
    

    TFolderBackup(const TStr& SettingsFNm, const bool& ReportP = true);
    TFolderBackup(const PJsonVal& SettingsJson, const bool& ReportP = true);

    // backup a particular profile
    TBackupLogInfo CreateBackup(const TStr& ProfileName);
    // backup all profiles
    void CreateBackup(TVec<TBackupLogInfo>& BackupLogInfo);
    
    void GetBackupFolders(const TStr& ProfileName, TStrV& FolderNmV) const;
    int GetBackupCount(const TStr& ProfileName) const;
    void Restore(const TStr& ProfileName, const TStr& BackupFolderName, const TBackupProfile::ERestoringMode& RestoringMode = TBackupProfile::RemoveExistingFirst) const;
    bool RestoreLatest(const TStr& ProfileName, const TBackupProfile::ERestoringMode& RestoringMode = TBackupProfile::RemoveExistingFirst) const;
    bool IsProfileName(const TStr& ProfileName) const { return ProfileH.IsKey(ProfileName); }
};

/*
example of a json file containing the settings to be used when initializing the TFolderBackup instance:

{
    "destination": "./_AutoBackups",
    "profiles" : {
        "Articles1" : {
            "versionsToKeep": 3,
            "folders": [
                { 
                    "folder": "./IndexArticles", 
                    "extensions": ["*"],
                    "skipIfContaining" : ["ArticlesWebRqLog.bin"],
                    "includeSubfolders": true
                }
            ]
        },
        "ClusteringEng" : {
            "versionsToKeep": 3,
            "folders": [
                { 
                    "folder": "./IndexClustersEng", 
                    "extensions": [".bin"],
                    "skipIfContaining" : ["IncomingWebRqLog"],
                    "includeSubfolders": true
                }
            ]
        },
        "ClusteringDefault" : {
            "versionsToKeep": 3,
            "folders": [
                { 
                    "folder": "./IndexClustersDefault", 
                    "extensions": [".bin"],
                    "skipIfContaining" : ["IncomingWebRqLog"],
                    "includeSubfolders": true
                }
            ]
        }
    }
}
*/

#endif
