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

#include <qminer_snap.h>
#include <qminer_js.h>

namespace TQm {
	namespace TAggrs {
		TTwitterGraph::TTwitterGraph(const TWPt<TBase>& Base, const TStr& AggrNm,
			const PRecSet& RecSet, const TStr& SaveFNm, const TStrV& UsersV, const TStr& GraphName) : TAggr(Base, AggrNm), Users(UsersV){
			uint TextFieldId = RecSet->GetStore()->GetFieldId("Text");
			uint DateFieldId = RecSet->GetStore()->GetFieldId("Date");
			uint AuthorJoinId = RecSet->GetStore()->GetJoinId("author");
			uint UserFieldId = Base->GetStoreByStoreNm("Users")->GetFieldId("Username");
			uint SentimentFieldId = RecSet->GetStore()->GetFieldId("Sentiment");

			////printf("got fixed users:\n");
			//TTwitterUsers::PrintVec(UsersV);
			

			for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
				TRec Rec = RecSet->GetRec(RecN);
				// look up user in Users store and get the Username field
				TStr User = Rec.DoSingleJoin(Base, AuthorJoinId).GetFieldStr(UserFieldId);
				
				if (!Users.IdOK(User)) {
					//printf("ignoring %s\n", User.CStr());
					continue;
				}
				// get text
				TStr Text = Rec.GetFieldStr(TextFieldId);
				// ignore retweets
				if (TTwitterUsers::ContainsRT(Text)) {
					continue;
				}
				TStrV UsersInText;
				TTwitterUsers::GetUsers(Text, UsersInText);
				// get date
				TTm Tm;
				Rec.GetFieldTm(DateFieldId, Tm);
				uint64 Time = (uint64)TTm::GetDateTimeIntFromTm(Tm);
				// update
				TInt Label = Rec.GetFieldInt(SentimentFieldId);
				Users.Update(User, Text, UsersInText, Time, Label);
				//printf("user: %s, text: %s, date: %f, users in text:\n", User.CStr(), Text.CStr(), (double)Time);
				//TTwitterUsers::PrintVec(UsersInText);
			}
			TFOut FOut(SaveFNm);
			Users.DumpDOTGraph(GraphName, "twitter @ graph", "digraph", FOut, true);			
		}
		
		PAggr TTwitterGraph::New(const TWPt<TBase>& Base, const TStr& AggrNm,
			const PRecSet& RecSet, const PJsonVal& JsonVal) {
			//// parse
			TStr FilePath = JsonVal->GetObjStr("fName");
			TStr GraphName = JsonVal->GetObjStr("dotName", "graph");
			TStrV UsersV;
			if (JsonVal->IsObjKey("userVec")) {
				//get JSON array of users
				PJsonVal RecordSet = JsonVal->GetObjKey("userVec");
				if (RecordSet->IsArr()) {
					RecordSet->GetArrStrV(UsersV);					
				}				
			}
			return New(Base, AggrNm, RecSet, FilePath, UsersV, GraphName);
		}

		PJsonVal TTwitterGraph::SaveJson() const {
			//return array of Usernames
			TStrV UsersV;
			Users.GetUsersV(UsersV);
			PJsonVal Usernames = TJsonVal::NewArr(UsersV);
			return Usernames;
		};
	}
}

