#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"


TEST(Trie, Adding)
{
	TTrie<TCh, TUInt64> TrieH;

	TrieH.AddDat(TStr("testi"), 1);
	TrieH.AddDat(TStr("test"), 2);
	TrieH.AddDat(TStr("tes"), 3);
	TrieH.AddDat(TStr("te"), 4);
	TUInt64V DestV;
	TrieH.SearchByPrefix(TStr("tes"), DestV, 2, true);
	ASSERT_EQ(DestV.Len(), 2);
	TrieH.SearchByPrefix(TStr("tes"), DestV, -1, true);
	ASSERT_EQ(DestV.Len(), 3);

	// test overriding the old value for the same key
	TrieH.AddDat(TStr("testi"), 123);
	TrieH.SearchByPrefix(TStr("testi"), DestV, -1, true);
	ASSERT_EQ(DestV.Len(), 1);
	ASSERT_EQ(DestV[0], 123);

	TrieH.AddIfNew(TStr("testi"), 1234);
	TrieH.SearchByPrefix(TStr("testi"), DestV, -1, true);
	ASSERT_EQ(DestV.Len(), 1);
	// we should not get 1234 since the key already existed
	ASSERT_EQ(DestV[0], 123);

	TrieH.SearchByPrefix(TStr("invalid"), DestV, -1, true);
	ASSERT_EQ(DestV.Len(), 0);
}
