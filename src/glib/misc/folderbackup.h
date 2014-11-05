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
	TStr LogInfo;		// info about the success or failure of the backing up
	TFlt SecsNeeded;	// how long did we need to make the backup (in seconds)
	TStr FolderName;	// name of the folder in which we have stored the backup data

public:
	TBackupLogInfo() {}
	TBackupLogInfo(const PJsonVal& Json);
	TBackupLogInfo(const TStr& FolderName, const TFlt SecsNeeded, const TStr& LogInfo);
	PJsonVal GetJson() const;
	TStr GetFolderName() const { return FolderName; }
};

// info about the folders to backup
class TBackupFolderInfo
{
public:
	TStr Folder;
	TStrV Extensions;
	TBool IncludeSubfolders;
};

class TBackupProfile
{
private:
	TStr Destination;
	TInt VersionsToKeep;
	TVec<TBackupFolderInfo> FolderV;

	TStr GetFolderNameForCurrentTime() const;
	void CopyFolder(const TStr& BaseTargetFolder, const TStr& SourceFolder, const TStrV& Extensions, const bool& IncludeSubfolders, TStr& ErrMsg);

public:
	TBackupProfile() {}
	TBackupProfile(const PJsonVal& SettingsJson);
	TBackupLogInfo CreateBackup();
	int GetVersionsToKeep() const { return VersionsToKeep; }
	TStr GetDestination() const { return Destination; }
};


// main class for doing data backup
// it uses a settings file containing one or more profiles. each profile has a set of folders to backup
// calling CreateBackup creates a new backup and possibly removes old backup folder (depending on the value of "versionsToKeep")
// it also stores the backup information in a "backupLogFile" file
class TFolderBackup
{
private:
	TStr ProfileLogFile;

	THash<TStr, TBackupProfile> ProfileH;
	THash<TStr, TVec<TBackupLogInfo> > ProfileToLogVH;

	void ParseSettings(const PJsonVal& SettingsJson);
	void SaveLogs() const;

public:
	TFolderBackup(const TStr& SettingsFNm);
	TFolderBackup(const PJsonVal& SettingsJson);

	// backup a particular profile
	TBackupLogInfo CreateBackup(const TStr& ProfileName);
	// backup all profiles
	void CreateBackup(TVec<TBackupLogInfo>& BackupLogInfo);

};

/*
example of a json file containing the settings to be used when initializing the TFolderBackup instance:

{
	"backupLogFile": "E:\\data\\NewsMiner\\_AutoBackups\\backups.json",
	"profiles" : {
		"er" : {
			"destination": "E:\\data\\NewsMiner\\_AutoBackups",
				"versionsToKeep" : 3,
				"folders" : [
			{
				"folder": "E:\\data\\NewsMiner\\IndexArticles",
					"extensions" : ["*"],
					"includeSubfolders" : true
			},
			{
				"folder": "E:\\data\\NewsMiner\\IndexGeo",
				"includeSubfolders" : true
			}
				]
		}
	}
}
*/

#endif