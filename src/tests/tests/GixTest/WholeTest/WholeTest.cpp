#define GIX_DEBUG
#define GIX_TEST

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <base.h>
#include <mine.h>
#include <qminer_core.h>
#include <qminer_gs.h>

#include <iostream>
#include <windows.h>

////////////////////////////////////////////////////////////////////////
// typedefs

//typedef  TGixDefMerger < TIntUInt64Pr, TUInt64 > TMyMerger;
//typedef TPt<TGixMerger<TIntUInt64Pr, TUInt64> > PGixMerger;

typedef TIntUInt64Pr TMyKey;
typedef TKeyDat<TUInt64, TInt> TMyItem;

// forward defs

class TMyGixDefMerger;

////////////////////////////////////////////////////////////////////////
// more typedefs
typedef  TGixItemSet < TMyKey, TMyItem, TMyGixDefMerger > TMyItemSet;
typedef  TPt < TMyItemSet > PMyItemSet;

typedef TGix<TMyKey, TMyItem, TMyGixDefMerger> TMyGix;
typedef TPt<TGix<TMyKey, TMyItem, TMyGixDefMerger> > PMyGix;


///////////////////////////////////////////////////////////////////////
// for nice outputs

HANDLE  hConsole;

void WarnNotifyI(TStr& const s) {
	SetConsoleTextAttribute(hConsole, 11);
	WarnNotify(s);
	SetConsoleTextAttribute(hConsole, 15);
}
void WarnNotifyW(TStr& const s) {
	SetConsoleTextAttribute(hConsole, 14);
	WarnNotify(s);
	SetConsoleTextAttribute(hConsole, 15);
}

#define TAssert(Cond, MsgCStr) \
  ((Cond) ? static_cast<void>(0) : WarnNotifyW( TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) +": "+ MsgCStr))


////////////////////////////////////////////////////////////////////////

class XTest {
public:

	void Test_Simple_1() {
		WarnNotifyI(TStr("Starting..."));
		{

			TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
			TStr def_dir = "..\\..\\..\\..\\..\\examples\\movies";
			
			//TStr def_file = def_dir + "\\movies.def";
			TStr def_file = def_dir + "\\movies_small.def";
			
			//TStr data_file = def_dir + "\\sandbox\\movies\\movies.json";
			TStr data_file = "..\\..\\..\\..\\..\\test\\nodejs\\sandbox\\movies\\movies_data.txt";

			// init unicode
			TUnicodeDef::Load(unicode_file);

			// create new base from definition
			PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
			TPt<TQm::TBase> Base = TQm::TStorage::NewBase("data\\", SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024);

			// load movies data
			{
				{
					TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("Movies");
					{
						PSIn fin = TFIn::New(data_file);
						TStr s;
						while (fin->GetNextLn(s)) {
							PJsonVal json = TJsonVal::GetValFromStr(s);
							store->AddRec(json);
						}
					}
				}
			}
			// do some querying
			
			/*auto res = Base->Search("{ \"$from\": \"Movies\", \"$or\": [ { \"Genres\": \"Action\" }, { \"Plot\": \"America\" } ] }");
			printf("Records: %d\n", res->GetRecs());*/

			auto res = Base->Search("{ \"$join\": { \"$name\": \"Actor\", \"$query\" : { \"$from\": \"Movies\", \"Genres\" : \"Horror\", \"$or\" : [{ \"Title\": \"lost\" }, { \"Plot\": \"lost\" }]}}}");
			printf("Records: %d\n", res->GetRecs());			
		}
		WarnNotifyI(TStr("Finished."));
	}

	void PerformTests() {
		Test_Simple_1();
	}

};

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	TQm::TEnv::Init();

	XTest test;
	test.PerformTests();

	return 0;
}

