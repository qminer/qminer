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

#include <qminer_snap.h>

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

