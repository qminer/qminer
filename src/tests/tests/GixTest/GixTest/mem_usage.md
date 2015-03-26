This file is used for manual inspection of memory usage
==========================================================

it looks like TCache is not reporting correctly. 
Hash table seems ok. 
There were no memory leaks detected using Deleaker.

Maybe the problem lies with TBlobBs - it contains some vectors that are not trivial??

TGixItemSet

	*CRef
	*int TotalCnt
	*TKey ItemSetKey
	*TBool MergedP
	*TBool Dirty
	*TGixMerger *Merger
	*TGix<TKey, TItem, TGixMerger> *Gix
	*TVec<TItem> ItemV - it only reports usage inside memory-buffer, which in case of TGix is all there is
	*TVec<int> ItemVDel - same as above
	*TVec<TGixItemSetChildInfo> Children - same as above
	*TVec<TVec<TItem>> ChildrenData - is reported properly, in two steps

TGix

	*CRef
	*TFAccess Access;
	*TStr GixFNm;
	*TStr GixBlobFNm;
	*THash<TKey, TBlobPt> KeyIdH;

	*TGixMerger Merger;

	*TCache<TBlobPt, PGixItemSet> ItemSetCache;
	*PBlobBs ItemSetBlobBs;

	*int64 CacheResetThreshold;
	*int64 NewCacheSizeInc;
	*bool CacheFullP;
	
	*int SplitLen;
	*int SplitLenMin;
	*int SplitLenMax;
	*TGixStats Stats;

TCache

	*int64 MxMemUsed;
	*int64 CurMemUsed;
	THash<TKey, TKeyLNDatPr, THashFunc> KeyDatH;
	TKeyL TimeKeyL;


-----------------------

300.000
50.556kb -> 22.597kb (diff 28.000)
h 10.3
c 10.4 -> 0

300.000
52.977kb -> 25.264kb (diff 27.700)
h 10.3
c 10.4 -> 0


400.000
51.560kb -> 31.072kb (diff 20.500)
h 10.3 -> 0
c 10.4

500.000
51.560kb -> 31.072kb (diff 20.500)
h 17.5 -> 0
c 10.4


600.000
70.471kb -> 31.072kb (diff 39.500)
h 20.7 -> 0
c 10.4
70.467kb -> 42.549kb (diff 27.900)
h 20.7
c 10.4 -> 0


800.000
72.040kb -> 31.072kb (diff 41.030)
h 28.7 -> 0
c 10.4
73.465kb -> 45.940kb (diff 27.500)
h 28.7
c 10.4 -> 0



1.300.000
109.867kb -> 31.072kb (diff 78.800)
h 44.7 -> 0
c 10.4
109.862kb -> 82.341kb (diff 27.500)
h 44.7
c 10.4 -> 0