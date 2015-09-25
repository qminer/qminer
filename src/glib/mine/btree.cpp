namespace BtreeTest
{

// This test performs a mixture of additions, deletions, and range queries
// and constantly checks the validity of the tree and correctness of results.
void Test(int nKeys)
{
	if (false && nKeys != 44) return;
	printf("nKeys = %d...    \r", nKeys);
	typedef TStr TKey;
	typedef TStr TDat;
	typedef TInt TNodeId;
	typedef TBtreeNodeMemStore_Paranoid<TKey, TNodeId, TNodeId> TInternalStore;
	typedef TBtreeNodeMemStore_Paranoid<TKey, TDat, TNodeId> TLeafStore;
	typedef TVec<TKey> TKeyV;
	typedef TKeyDat<TKey, TDat> TKd;
	typedef TVec<TKd> TKdV;
	typedef TBtreeOps<TKey, TDat, TCmp<TKey>, TNodeId, TInternalStore, TLeafStore> TMyBtreeOps;
	const int internalCapacity = 3, leafCapacity = 5;
	TMyBtreeOps bTree(new TInternalStore(), new TLeafStore(), internalCapacity, leafCapacity, true, true);
#define internalStore (*(bTree.internalStore))
#define leafStore (*(bTree.leafStore))
	bTree.Validate(); internalStore.IAssertNoCheckouts(); leafStore.IAssertNoCheckouts();
	if (false) bTree.Dump();
	// Prepare a list of (key, dat) pairs that we'll be working with.
	//int nKeys = 100;
	TRnd rnd(123 + nKeys); 
	TKdV keyDats; for (int i = 0; i < nKeys; i++) {
		char buf[50]; sprintf(buf, "key%03d", i); TStr key = buf;
		sprintf(buf, "dat%d-%03d", rnd.GetUniDevInt(nKeys), i); TStr dat = buf;
		keyDats.Add(TKd(key, dat)); }
	TIntV keysInTree, keysOutside; // contain indices into 'keyDats'
	for (int i = 0; i < nKeys; i++) keysOutside.Add(i);
	// The test run will consist of several phases.  In each phase, we randomly select
	// a goal size of the tree and then keep adding/deleting keys until the goal size is reached.
	// The first phase will have a goal size of at least 90% * nKeys; the last phase will have a goal 
	// size of 0 to delete everything.
	const int nPhases = 10; int opNo = 0;
	for (int phaseNo = 0; phaseNo < nPhases; phaseNo++)
	{
		int goalSize;
		if (phaseNo == 0) goalSize = rnd.GetUniDevInt((nKeys * 2) / 3, nKeys);
		else if (phaseNo == nPhases - 1) goalSize = 0;
		else goalSize = rnd.GetUniDevInt(nKeys / 10, (9 * nKeys) / 10);
		//
		while (keysInTree.Len() != goalSize)
		{
			// We will be mixing additions and deletions in each phase, but the operation
			// that moves us closer to the goal size has a higher probability.
			int add = rnd.GetUniDevInt(3);
			if (add >= 2) add = (keysInTree.Len() > goalSize ? 0 : 1);
			if (add && keysOutside.Empty()) add = 0;
			else if (! add && keysInTree.Empty()) add = 1;
			//
			if (add)
			{
				int idx = rnd.GetUniDevInt(keysOutside.Len());
				int keyNo = keysOutside[idx]; 
				keysOutside[idx] = keysOutside.Last(); keysOutside.DelLast();
				bTree.Add(keyDats[keyNo].Key, keyDats[keyNo].Dat);
				keysInTree.Add(keyNo);
			}
			else
			{
				int idx = rnd.GetUniDevInt(keysInTree.Len());
				int keyNo = keysInTree[idx]; keysInTree[idx] = keysInTree.Last(); keysInTree.DelLast();
				TStr dat;
				bool found = bTree.Del(keyDats[keyNo].Key, dat);
				IAssert(found);
				IAssert(keyDats[keyNo].Dat == dat);
				keysOutside.Add(keyNo);
			}
			opNo++;
			if (0) printf("%c", add ? '#' : '-');
			if (false) bTree.Dump();
			// Perform a few ranged queries.
			TKey minKey = keyDats[rnd.GetUniDevInt(nKeys)].Key, maxKey = keyDats[rnd.GetUniDevInt(nKeys)].Key;
			bool includeMin = (rnd.GetUniDevInt(3) > 0), includeMax = (rnd.GetUniDevInt(3) > 0);
			TKdV results, trueResults;
			for (int i = 0; i < keysInTree.Len(); i++) {
				const TKd &kd = keyDats[keysInTree[i]];
				if ((minKey < kd.Key || (includeMin && minKey == kd.Key)) &&
					(maxKey > kd.Key || (includeMax && maxKey == kd.Key)))
					trueResults.Add(kd); }
			trueResults.Sort();
			bTree.RangeQuery_(minKey, maxKey, includeMin, includeMax, TMyBtreeOps::TKeyDatSink(results));
			IAssert(results.Len() == trueResults.Len());
			for (int i = 0; i < results.Len(); i++) {
				IAssert(results[i].Key == trueResults[i].Key);
				IAssert(results[i].Dat == trueResults[i].Dat); }
			if (includeMin && includeMax)
			{
				TKeyV results2; bTree.RangeQuery(minKey, maxKey, results2);
				IAssert(results2.Len() == trueResults.Len());
				for (int i = 0; i < results2.Len(); i++) 
					IAssert(results2[i] == trueResults[i].Key);
			}
			if (keysInTree.Len() > 0)
			{
				int keyNo = keysInTree[rnd.GetUniDevInt(keysInTree.Len())];
				const TKey &key = keyDats[keyNo].Key;
				bool found = bTree.IsKey(key); IAssert(found);
				TStr dat; found = bTree.IsKeyGetDat(key, dat); 
				IAssert(found); IAssert(dat == keyDats[keyNo].Dat);
			}
			if (keysOutside.Len() > 0)
			{
				int keyNo = keysOutside[rnd.GetUniDevInt(keysOutside.Len())];
				const TKey &key = keyDats[keyNo].Key;
				bool found = bTree.IsKey(key); IAssert(! found);
				TStr dat, dat2; char buf[50]; sprintf(buf, "%d %d", rnd.GetUniDevInt(), rnd.GetUniDevInt()); dat = buf; dat2 = dat;
				found = bTree.IsKeyGetDat(key, dat); 
				IAssert(! found); IAssert(dat2 == dat);
			}
		}
		bTree.Validate(); internalStore.IAssertNoCheckouts(); leafStore.IAssertNoCheckouts();
		if (0) printf("(%d)", goalSize);
		// Test serialization and deserialization.
		{
			PSOut SOut = TMOut::New();
			bTree.Save(*SOut);
			bTree.~TMyBtreeOps();
			memset(&bTree, 0x69, sizeof(bTree));
			//
			TMOut *mOut = (TMOut *) SOut();
			char *buf = mOut->GetBfAddr();
			int bufLen = mOut->Len();
			char *buf2 = new char[bufLen]; memcpy(buf2, buf, bufLen);
			memset(buf, 0x69, bufLen);
			mOut->Clr(); mOut = 0; SOut = 0;
			printf(" (%d bytes) ", bufLen);
			//
			TMIn mIn(buf2, bufLen, true);
			new (&bTree) TMyBtreeOps(mIn);
		}

	}
	printf("nKeys = %d; %d add/del ops in %d phases.\n", nKeys, opNo, nPhases);

#undef internalStore 
#undef leafStore 

/*
	for (int i = 0; i < keyOrder.Len(); i++) 
	{
		char buf[50]; sprintf(buf, "key%03d", keyOrder[i]); TStr key = buf;
		sprintf(buf, "dat%d-%03d", rnd.GetUniDevInt(keyOrder.Len()), keyOrder[i]); TStr dat = buf;
		bTree.Add(key, dat);
		if (false) bTree.Dump();
		bTree.Validate(); internalStore.IAssertNoCheckouts(); leafStore.IAssertNoCheckouts();
	}
	if (false) bTree.Dump();
	keyOrder.Shuffle(rnd);
	for (int i = 0; i < keyOrder.Len(); i++)
	{
		char buf[50]; sprintf(buf, "key%03d", keyOrder[i]); TStr key = buf; TStr dat;
		bool found = bTree.Del(key, dat);
		IAssert(found);
		// ToDO: check that 'dat' is correct.
		if (false) bTree.Dump();
		bTree.Validate(); internalStore.IAssertNoCheckouts(); leafStore.IAssertNoCheckouts();
	}
	*/
}

// Speed test, measuring the speed of mixed additions and deletions in a large tree.
// (Note: the average number of keys in the tree during this experiment will be nKeys/2.)
void Test2(int nKeys, int internalCapacity, int leafCapacity)
{
	typedef TInt TKey;
	typedef TVec<TKey> TKeyV;
	typedef TInt TDat;
	typedef TInt TNodeId;
	typedef TKeyDat<TKey, TDat> TKd;
	typedef TVec<TKd> TKdV;
	typedef TBtreeNodeMemStore<TKey, TNodeId, TNodeId> TInternalStore;
	typedef TBtreeNodeMemStore<TKey, TDat, TNodeId> TLeafStore;
	typedef TBtreeOps<TKey, TDat, TCmp<TKey>, TNodeId, TInternalStore, TLeafStore> TMyBtreeOps;
	//const int internalCapacity = 20, leafCapacity = 20;

	TMyBtreeOps bTree(new TInternalStore(), new TLeafStore(), internalCapacity, leafCapacity, true, true);
	
	printf("nKeys = %d, internalCapacity = %d, leafCapacity = %d\n", nKeys, internalCapacity, leafCapacity);
	TKdV keysInTree, keysOutside;
	const int MaxKey = 1000000000; TRnd rnd(123 + nKeys);
	for (int i = 0; i < nKeys; i++) {
		int key = rnd.GetUniDevInt(MaxKey);
		int dat = rnd.GetUniDevInt(MaxKey);
		dat = key;
		keysOutside.Add(TKd(key, dat)); }
	keysOutside.Sort();
	for (int i = 1; i < keysOutside.Len(); i++)
		if (keysOutside[i].Key <= keysOutside[i - 1].Key)
			keysOutside[i].Key = keysOutside[i - 1].Key + 1; 
	//
	keysInTree.Reserve(nKeys);
	keysOutside.Shuffle(rnd);
	TTimer tm; tm.Start();
	while (keysOutside.Len() > keysInTree.Len()) {
		TKd kd = keysOutside.Last(); keysOutside.DelLast();
		keysInTree.Add(kd);
		bTree.Add(kd.Key, kd.Dat); }
	tm.Stop();
	printf("Adding %d keys into an empty tree: %.3f s (%.3f keys/s)\n", keysInTree.Len(), tm.Sec(), keysInTree.Len() / tm.Sec());
	bTree.Validate();
	//
	TTimer tmAdd, tmDel;
	int nAdds = 0, nDels = 0;
	for (int iOp = 0; /*iOp <= 3450331*/ tmAdd.Sec() <= 30; iOp++)
	{
		if (false) if (iOp == 3450331 || iOp % 10000 == 0) {
			printf("Op #%d     \r", iOp);
			bTree.Validate(); }
		bool add = (rnd.GetUniDevInt(2) == 0);
		if (keysOutside.Empty()) add = false; else if (keysInTree.Empty()) add = true;
		if (add)
		{
			int idx = rnd.GetUniDevInt(keysOutside.Len());
			TKd kd = keysOutside[idx]; keysOutside[idx] = keysOutside.Last(); keysOutside.DelLast();
			keysInTree.Add(kd);
			tmAdd.Start();
			bTree.Add(kd.Key, kd.Dat);
			tmAdd.Stop(); nAdds++;
		}
		else
		{
			int idx = rnd.GetUniDevInt(keysInTree.Len());
			TKd kd = keysInTree[idx]; keysInTree[idx] = keysInTree.Last(); keysInTree.DelLast();
			keysOutside.Add(kd);
			//if (iOp == 3450331) printf("!");
			tmDel.Start();
			TDat dat; bool found = bTree.Del(kd.Key, dat);
			tmDel.Stop(); nDels++;
			if (! (found && dat == kd.Dat)) 
				printf("Error at nKeys = %d, iOp = %d: key = %d, dat = %d, kd.Dat = %d\n",
					nKeys, iOp, int(kd.Key), int(dat), int(kd.Dat)); 
			IAssert(found); IAssert(dat == kd.Dat);
		}
	}
	printf("Mixed additions/deletions:\n");
	printf("%d additions in %.3f s (%.3f keys/s)\n", nAdds, tmAdd.Sec(), nAdds / tmAdd.Sec());
	printf("%d deletions in %.3f s (%.3f keys/s)\n", nDels, tmDel.Sec(), nDels / tmDel.Sec());
	bTree.Validate();
}

int BtreeTestMain(int argc, char** argv)
{
	if (false)
	{
		for (int nKeys = 1; nKeys <= 1000; nKeys++) 
			if (nKeys <= 100 || nKeys % 100 == 0)
				BtreeTest::Test(nKeys);
	}
	if (true)
	{
		//BtreeTest::Test2(2000000);
		BtreeTest::Test2(
			(argc > 1) ? atoi(argv[1]) : 2000000,
			(argc > 2) ? atoi(argv[2]) : 20,
			(argc > 3) ? atoi(argv[3]) : 20);
	/*
	D:\users\janez\dev\IndexStructures\IndexStructures\x64\release>indexstructures.exe 2000000 10 10
	Hello World!
	nKeys = 2000000, internalCapacity = 10, leafCapacity = 10
	Adding 1000000 keys into an empty tree: 0.969 s (1032258.065 keys/s)
	!Mixed additions/deletions:
	25152818 additions in 30.016 s (837990.813 keys/s)
	25148061 deletions in 33.188 s (757757.017 keys/s)

	D:\users\janez\dev\IndexStructures\IndexStructures\x64\release>indexstructures.exe 2000000 20 20
	Hello World!
	nKeys = 2000000, internalCapacity = 20, leafCapacity = 20
	Adding 1000000 keys into an empty tree: 0.875 s (1142857.143 keys/s)
	!Mixed additions/deletions:
	27001817 additions in 30.016 s (899592.029 keys/s)
	26992740 deletions in 31.906 s (846001.645 keys/s)

	D:\users\janez\dev\IndexStructures\IndexStructures\x64\release>indexstructures.exe 2000000 30 30
	Hello World!
	nKeys = 2000000, internalCapacity = 30, leafCapacity = 30
	Adding 1000000 keys into an empty tree: 0.828 s (1207547.170 keys/s)
	!Mixed additions/deletions:
	28008193 additions in 30.016 s (933120.433 keys/s)
	27997289 deletions in 32.719 s (855695.557 keys/s)
	*/
	}
	return 0;
}

/*
>> (*) Save/Load
>> (*) Add(id, val)
>> (*) Del(id, val)
>> (*) pri (a) range search
>> (*) pri (b) in (c) nearest neighbor
*/

} // namespace BtreeTest

