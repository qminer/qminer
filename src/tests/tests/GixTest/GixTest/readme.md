# Project notes

## Task 1

### Problem

I/O traffic and memory consumption for huge data vectors (popular words like *Obama*) in `TGix` (inverted index) 
that need to be swapped when single item (new document) is added into them.

### Proposed solution

Change the way `TGix` is handling data vectors - split single vector into several vectors,
so that only the last one needs to be updated.

### Details

- Change `TGixItemSet` to use primary vector (the existing one up to some (configurable) length) plus child vectors. 
   The primary vector is called work-buffer and contains largest entries when merged. 
- These vectors would be serialized into BLOB separately - Blob remains in the `TGix` class.
- They are not cached in separate cache - `TGix` will provide methods for storing and retrieving them, `TGixItemSet` will load them if needed.
- When items are added into itemset, we check if the new value preserves "is merged" flag.
- When items are deleted, we add those deletes into work buffer. 
- When itemset is merged, we first process all deletes and then merge the remaining items.
- When itemset is saved, we only save child vectors with dirty bit set.

What we will achieve
- When we are adding new items to itemset, if itemset is not yet in memory, the last vector (the work-buffer) will be loaded into memory.
- When we need whole itemset data, we load all vectors into memory and perform in-memory concatenation.

## Github stuff

Establish a link to the official repository

	git remote add upstream https://github.com/qminer/qminer.git

To get all the latest stuff from the official repository

	git pull upstream master