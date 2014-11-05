# Project notes

## Task 1

### Problem

I/O traffic and memory consumption for huge data vectors (popular words like *Obama*) in `TGix` (inverted index) 
that need to be swapped when single item (new document) is added into them.

### Proposed solution

Change the way `TGix` is handling data vectors - split single vector into several vectors,
so that only the last one needs to be updated.

### Details

- Change `TGixItemSet` to use primary vector (the existing one up to some predefined/configurable? length) plus extension vectors.
- These vectors would be serialized into BLOB separately - Blob remains in the `TGix` class.
- They are not cached in separate - `TGix` will provide methods for storing and retrieving them, `TGixItemSet` will load them if needed.

What we will achieve
- When we are adding new items to itemset, if itemset is not yet in memory, only the first and the last vector will be loaded into memory.
- When we need whole itemset data, we load all vectors into memory and perform in-memory concatenation.

## Github stuff

Establish a link to the official repository

	git remote add upstream https://github.com/qminer/qminer.git

To get all the latest stuff from the official repository

	git pull upstream master