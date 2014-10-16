# Project notes

## Task 1

### Problem

I/O traffic and memory consumption for huge data vectors (popular words like *Obama*) in `TGix` (inverted index) 
that need to be swapped when single item (new document) is added into them.

### Proposed solution

Change the way `TGix` is handling data vectors - split single vector into several vectors,
so that only the last one needs to be updated.

### Details

In class `TGix` there are two members that we need to expand:

	THash<TKey, TBlobPt> KeyIdH
	mutable TCache<TBlobPt, PGixItemSet> ItemSetCache

**Variant 1**

`KeyIdH` is used to map between keys and `BLOB` pointers. `ItemSetCache` is used for caching of
`BLOB` data - this object contains data vectors that reside in memory, so they need to be split.

But the split can be done inside `KeyIdH`, so instead of mapping from `TKey` to `TBlobPt` we 
will map  from `TKey` to `TVec<TBlobPt>`.

When adding new record we should check if currently last vector is full - if it reached some size threshold.
If yes, we need to create new item set and insert the new data into this new vector/item set.

**Variant 2**

But looking at the public methods of `TGix` class I am contemplating about moving this code into `TGixItemSet`.
The reason being that this class contains single vector and is also exposed to outside code.
What we need is to:

- Change `TGixItemSet` to contain vector of vectors and not just single one.
- Split `TGix` so that caching code goes into separate object. `TGixItemSet` instances should receive a reference to it.
- Also, the Blob-handling code should move into that class too.
- This means that almost all of the `TGix` code should be moved. Perhaps we should just pass a reference to `TGix` into `TGixItemSet`.

**Variant 3**

Another option would be to:

- Change `TGixItemSet` to use primary vector (the existing one) plus extension vectors.
- These vectors would be serialized into BLOB separately - Blob remains in the `TGix` class.
- They would also have their own cache. (where would this cache reside? special setting to limit cache size?)

## Github stuff

Establish a link to the official repository

	git remote add upstream https://github.com/qminer/qminer.git

To get all the latest stuff from the official repository

	git pull upstream master