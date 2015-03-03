/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef QMINER_SNAP_H
#define QMINER_SNAP_H

#include <string>
#include <qminer.h>


namespace TQm {
	namespace TAggrs {
		class TTwitterUser {
		public:
			TStr ID;
			TStr Text;
			THash<TStr, TPair<TInt, TUInt64>> UsersAddressed;

			TInt nNeg;
			TInt nPos;
			uint64 NumTweets;
			uint64 MinTime;
			uint64 MaxTime;
		public:
			THash<TStr, TPair<TInt, TUInt64>> & GetUsersAddressed() { return UsersAddressed; }
			TTwitterUser(){ MinTime = TUInt64::Mx; MaxTime = TUInt64::Mn; NumTweets = 0; nNeg = 0; nPos = 0;}
			TTwitterUser(const TStr& ID_) {
				ID = ID_;
				MinTime = TUInt64::Mx;
				MaxTime = TUInt64::Mn;
				NumTweets = 0;
				nNeg = 0; nPos = 0;
			}
			// removes all IDs from UsersAddressed (the users that the current user referenced) that are not keys in Users (not tweet authors)
			void Clean(const TStrH& UsersH) {
				TStrV Keys;
				UsersAddressed.GetKeyV(Keys);
				for (int KeyN = 0; KeyN < Keys.Len(); KeyN++) {
					if (!UsersH.IsKey(Keys[KeyN])){
						//printf("deleting %s\n", Keys[KeyN].CStr());
						UsersAddressed.DelKey(Keys[KeyN]);
					}
				}
			}

			void  Update(const TStr& TweetText, const TStrV& UsersInTweet, const uint64& TweetTime, const TInt& Label){
				for (int UserN = 0; UserN < UsersInTweet.Len(); UserN++) {
					if (UsersAddressed.IsKey(UsersInTweet[UserN])){
						//known user
						UsersAddressed.GetDat(UsersInTweet[UserN]).Val1 += 1;
						if (UsersAddressed.GetDat(UsersInTweet[UserN]).Val2 > TweetTime) {
							UsersAddressed.GetDat(UsersInTweet[UserN]).Val2 = TweetTime;
						}
					}
					else {
						//new user
						TPair<TInt, TUInt64> pair;
						pair.Val1 = 1;
						//second value = first time of interaction
						pair.Val2 = TweetTime;
						UsersAddressed.AddDat(UsersInTweet[UserN], pair);
					}
				}
				Text += " " + TweetText;
				if (MinTime > TweetTime) {
					MinTime = TweetTime;
				}
				if (MaxTime < TweetTime) {
					MaxTime = TweetTime;
				}
				NumTweets += 1;
				if (Label == -1) { nNeg++; }
				if (Label == 1) { nPos++; }
			}

		};

		class TTwitterUsers {
		private:
			// if fixed users are not empty we ignore all other users and do not clean any fixed users
			THashSet<TStr> FixedUsers;
			bool FixedUsersEmpty;
			THash<TStr, TTwitterUser> Users;
		public:
			TTwitterUsers() : FixedUsersEmpty(true) {}
			TTwitterUsers(const TStrV& UsersV) : FixedUsers(UsersV) { 
				FixedUsersEmpty = FixedUsers.Empty();
				int Len = UsersV.Len();
				for (int UserN = 0; UserN < Len; UserN++) {
					TTwitterUser User(UsersV[UserN]);
					Users.AddDat(UsersV[UserN], User);
				}
			}
			bool IdOK(const TStr& ID) {
				if (!FixedUsersEmpty) {
					// if we are in fixed users mode, check if ID is a key
					return FixedUsers.IsKey(ID);
				}
				// we are not in fixed users mode, ID is OK
				return true;
			}
			void Update(const TStr& ID, const TStr& TweetText, const TStrV& UsersInTweet, const uint64& TweetTime, const TInt& Label) {
				// check if we should ignore user
				if (!IdOK(ID)) { return; }
				if (Users.IsKey(ID)){
					//known user
					Users.GetDat(ID).Update(TweetText, UsersInTweet, TweetTime, Label);
				}
				else {
					//new user
					TTwitterUser User(ID);
					User.Update(TweetText, UsersInTweet, TweetTime, Label);
					Users.AddDat(ID, User);
				}
			}
			int GetLen() { return Users.Len(); }
			void CleanUsers() {
				//remove addressed users that are not keys in Users (not tweet authors)
				TStrV UserIDs; 	Users.GetKeyV(UserIDs);

				TStrH UserIDsH(UserIDs.Len());
				for (int UserN = 0; UserN < UserIDs.Len(); UserN++) {
					UserIDsH.AddKey(UserIDs[UserN]);
				}
				if (!FixedUsersEmpty) {// add FixedUsers to the list
					TStrV KeyV; FixedUsers.GetKeyV(KeyV); int Keys = KeyV.Len();
					for (int KeyN = 0; KeyN < Keys; KeyN++) { UserIDsH.AddKey(KeyV[KeyN]); }
				}
				for (int UserN = 0; UserN < UserIDs.Len(); UserN++)	{
					Users.GetDat(UserIDs[UserN]).Clean(UserIDsH);
					//if (!Users.IsKey(UserIDs[UserN])) printf("%s was deleted from UsersAddressed\n", UserIDs[UserN].CStr());
				}

				////now remove users that address no one
				if (!FixedUsersEmpty) {
					for (int UserN = 0; UserN < UserIDs.Len(); UserN++)	{
						if (Users.GetDat(UserIDs[UserN]).UsersAddressed.Len() == 0) {
							Users.DelKey(UserIDs[UserN]);
							//printf("%s was deleted from users (no users addressed)\n", UserIDs[UserN].CStr());
						}
					}
				}
			}

			void DumpDOTGraph(const TStr& GraphName, const TStr& Description, const TStr& GraphType, TSOut &SOut, const bool& WriteText) {
				THash<TStr, TInt> UserKeyToColIdx;
				int temp = Users.FFirstKeyId();
				int idx = 0;
				while (Users.FNextKeyId(temp)) {
					UserKeyToColIdx.AddDat(Users.GetKey(temp), idx);
					idx++;
				}


				//graphType = {graph | digraph}
				TStr GType = GraphType.GetLc().GetTrunc();
				if (!GType.EqI("graph") && !GType.EqI("digraph")){
					printf("Graph type must be: {graph|digraph}\n");
					return;
				}
				SOut.PutStrLn(GType + " " + GraphName + " {");
				SOut.PutStrLn("\tdesc=\"" + Description + "\";");
				int KeyId;
				//// NODES
				KeyId = Users.FFirstKeyId();
				while (Users.FNextKeyId(KeyId)) {
					TStr NodeLine = "\t";

					TStr NodeKey = Users.GetKey(KeyId); ////NodeLine += "n" + NodeId + " ";								
					TStr NodeName = Users.GetDat(NodeKey).ID;
					TStr NodeIdx = UserKeyToColIdx.GetDat(NodeName).GetStr();
					NodeLine += NodeIdx + " ";

					// Attributes
					NodeLine += "[";
					NodeLine += "username=\"" + NodeName + "\", ";
					NodeLine += (Users.GetDat(NodeKey).nPos > Users.GetDat(NodeKey).nNeg) ? "sentiment = 1, " : "sentiment = -1, ";
					NodeLine += "numTweets=" + TUInt64::GetStr(Users.GetDat(NodeKey).NumTweets) + ", ";
					NodeLine += "firstTweetTime=" + TUInt64::GetStr(Users.GetDat(NodeKey).MinTime) + ", ";
					NodeLine += "lastTweetTime=" + TUInt64::GetStr(Users.GetDat(NodeKey).MaxTime);


					if (WriteText){
						NodeLine += ", text=\"" + encode(Users.GetDat(NodeKey).Text) + "\"";
					}

					NodeLine += "];";
					SOut.PutStrLn(NodeLine);
				}
				////// EDGES
				KeyId = Users.FFirstKeyId();
				while (Users.FNextKeyId(KeyId)) {
					TStr SourceNodeId = Users.GetKey(KeyId);
					THash<TStr, TPair<TInt, TUInt64>> UsersAddressed = Users.GetDat(SourceNodeId).UsersAddressed;
					int KeyId2 = UsersAddressed.FFirstKeyId();
					while (UsersAddressed.FNextKeyId(KeyId2)) {
						TStr EdgeLine = "\t";
						TStr TargetNodeId = UsersAddressed.GetKey(KeyId2);

						if (!UserKeyToColIdx.IsKey(SourceNodeId) || !UserKeyToColIdx.IsKey(TargetNodeId)){
							//one of the users doesn't have any tweets
							continue;
						}

						EdgeLine += UserKeyToColIdx.GetDat(SourceNodeId).GetStr() + " " + ((GType.EqI("graph")) ? ("--") : ("->")) + " " + UserKeyToColIdx.GetDat(TargetNodeId).GetStr() + " ";
						EdgeLine += "[";
						EdgeLine += "count=" + UsersAddressed.GetDat(TargetNodeId).Val1.GetStr() + ", ";
						EdgeLine += "firstInteraction=" + UsersAddressed.GetDat(TargetNodeId).Val2.GetStr();
						EdgeLine += "];";
						SOut.PutStrLn(EdgeLine);
					}
				}
				SOut.PutStrLn("}");
				SOut.Flush();
			}
			
			template <class T>
			static void PrintVec(const TVec<T>& Vec) {
				for (int ElN = 0; ElN < Vec.Len(); ElN++) {
					printf("%s ", Vec[ElN].GetStr().CStr());
				}
				printf("\n");

			}

			static TStr encode(TStr InStr)
			{
				std::string data(InStr.CStr());
				std::string::size_type pos = 0;
				for (;;)
				{
					pos = data.find_first_of("\"&<>", pos);
					if (pos == std::string::npos) break;
					std::string replacement;
					switch ((data)[pos])
					{
					case '\"': replacement = "&quot;"; break;
					case '&':  replacement = "&amp;";  break;
					case '<':  replacement = "&lt;";   break;
					case '>':  replacement = "&gt;";   break;
					default:;
					}
					data.replace(pos, 1, replacement);
					pos += replacement.size();
				};
				return TStr(data.c_str());
			}
			static bool ContainsRT(const TStr& Text) {
				if (Text.Len() < 4) return false;
				TStr Sub = Text.GetSubStr(0, 3);
				if (Sub.EqI("rt @")) return true;
				return false;
			}

			static bool TerminatingChar(const char a) {
				return !(TCh::IsAlNum(a) || a == '_');
			}

			static void GetUsers(const TStr& Text, TStrV& Users){
				// search string for @ and terminate usernames
				Users.Clr();
				TIntV StartIndices;
				int TempIdx = 0;
				int TextLen = Text.Len();
				while (TempIdx < TextLen) {
					TempIdx = Text.SearchCh('@', TempIdx);
					if (TempIdx == -1) break;
					StartIndices.Add(TempIdx);
					TempIdx++;
				}
				// for each start index look for a terminating character (not alpha numeric or _)
				for (int WordN = 0; WordN < StartIndices.Len(); WordN++) {
					for (int ChN = StartIndices[WordN] + 1; ChN < TextLen; ChN++) {
						if (TerminatingChar(Text[ChN]) || ChN == TextLen - 1) {
							if (StartIndices[WordN] + 1 <= ChN - 1) {
								TStr User = Text.GetSubStr(StartIndices[WordN] + 1, ChN - 1);
								if (User.Len() > 0 && User.Len() <= 15) {
									Users.Add(User);
								}
								break;
							}
						}
						
					}
				}
			}		

			void GetUsersV(TStrV& UsersV) const {
				UsersV.Clr();
				Users.GetKeyV(UsersV);
				return;
			}

		};

		///////////////////////////////
		// QMiner-Aggregator-Twitter@Graph
		// works with twitter stores
		class TTwitterGraph : public TAggr {
		private:
			TTwitterUsers Users;
			TTwitterGraph(const TWPt<TBase>& Base, const TStr& AggrNm,
				const PRecSet& RecSet, const TStr& SaveFNm, const TStrV& UsersV, const TStr& GraphName);
		public:
			static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
				const PRecSet& RecSet, const TStr& SaveFNm, const TStrV& UsersV, const TStr& GraphName) {
				return new TTwitterGraph(Base, AggrNm, RecSet, SaveFNm, UsersV, GraphName);
			}

			static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
				const PRecSet& RecSet, const PJsonVal& JsonVal);

			PJsonVal SaveJson() const;

			// aggregator type name 
			static TStr GetType() { return "twitterGraph"; }
		};
	}
}


#endif
