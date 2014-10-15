# Project notes

## Task 1

**Problem:** 

I/O traffic and memory consumption for huge data vectors (popular words like *Obama*) in `TGix` (inverted index) 
that need to be swapped when single item (new document) is added into them.

**Proposed solution:**

Change the way `TGix` is handling data vectors - split single vector into several vectors,
so that only the last one needs to be updated.

### Details

In class `TGix` there are two members that we need to expand:

	THash<TKey, TBlobPt> KeyIdH
	mutable TCache<TBlobPt, PGixItemSet> ItemSetCache

## Github stuff

Establish a link to the official repository

	git remote add upstream https://github.com/qminer/qminer.git

To get all the latest stuff from the official repository

	git pull upstream master