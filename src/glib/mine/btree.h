/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ____BTREE_H_INCLUDED____
#define ____BTREE_H_INCLUDED____

namespace TBtree {

template<typename T> class TPrinter { };
template<> class TPrinter<TStr> {
    public: void operator()(FILE *f, const TStr& s) const { fprintf(f, "\"%s\"", s.CStr()); }};
template<> class TPrinter<TInt> {
    public: void operator()(FILE *f, const TInt& x) const { fprintf(f, "%d", int(x)); }};
template<> class TPrinter<TFlt> {
    public: void operator()(FILE *f, const TFlt& x) const { fprintf(f, "%g", double(x)); }};

//----------------------------------------------------------------------------
// TBtreeNode
//----------------------------------------------------------------------------

template <typename TKey_, typename TDat_, typename TNodeId_>
class TBtreeNode
{
public:
	typedef TKey_ TKey;
	typedef TDat_ TDat;
	typedef TNodeId_ TNodeId;
	typedef TKeyDat<TKey, TDat> TKd;
	typedef TVec<TKd> TKdV;
	TKdV v;
	TNodeId prev, next;

	TBtreeNode() : prev(-1), next(-1) { }
	TBtreeNode(const TBtreeNode &other) : v(other.v), prev(other.prev), next(other.next) { }
	TBtreeNode(TSIn& SIn) : v(SIn), prev(SIn), next(SIn) { }
	void Load(TSIn& SIn) { v.Load(SIn); prev.Load(SIn); next.Load(SIn); }
	void Save(TSOut& SOut) const { v.Save(SOut); prev.Save(SOut); next.Save(SOut); }
	void Clr() { v.Clr(); prev = -1; next = -1; }
	int Add(const TKey& key, const TDat& dat) {
		return v.Add(TKdV(key, dat)); }
};

//----------------------------------------------------------------------------
// IBtreeNodeStore
//----------------------------------------------------------------------------
//
// NOTE: This template just shows an example of what must be supported by the classes which will be
// given to TBtreeOps as the TLeafStore and TInternalStore template parameters.  In practice,
// you don't have to actually derive your store implementation from IBtreeStore or anything of that sort.
//
// For examples of concrete node store implementations, see TBtreeNodeMemStore
// and TBtreeNodeMemStore_Paranoid below.
//
// The store has to support the following operations:
// - AllocNode/FreeNode
// - CheckOutNode(nodeId): returns a pointer to a TNode instance containing the contents of the given node.
//   This TNode instance is owned and managed by the store, and will remain valid until the
//   node is checked in again.
// - CheckInNode(nodeId, TNode*, dirty): must be called by the user of the store when it is done
//   using this node.  The TNode* must be the same pointer that was returned by CheckInNode.
//   'dirty' must be set to 'true' if the node has been modified, otherwise it should be set to 'false'.
//   This is intended to help disk-based node stores decide whether they should save the node to disk or not.
//
// Promises made by TBtreeOps to the store:
// - any node that is checked out will eventually be checked in
// - if a node is already checked out, it won't be checked out again (without being checked in first)
// - when the node is checked in, the 'node' parameter will be the same pointer that was given by CheckOutNode as a return value
// - if 'dirty' is false, then the contents of the TNode pointed to by that pointer are the same as they were when CheckOutNode returned
// - TNode.v will at no time be made to contain more than 2 * capacity + 1 entries
// - CheckOutNode will receive a valid node id; FreeNode won't receive the ID of a node that is currently checked out
// - Clr won't be called if any node is checked out at the moment
// Promises made by the store to TBTreeOps:
// - the pointer returned by CheckOutNode will remain valid until the node is checked in again,
//   even if during this time (other) nodes are being allocated, freed, checked in/out from the same store.

template <typename TKey_, typename TDat_, typename TNodeId_>
class IBtreeNodeStore
{
private:
	TCRef CRef;
public:
	typedef TKey_ TKey;
	typedef TDat_ TDat;
	typedef TNodeId_ TNodeId;
	typedef TBtreeNode<TKey, TDat, TNodeId> TNode;

	static TPt<IBtreeNodeStore> Load(TSIn& SIn);
	void Save(TSOut &SOut) const;

	TNodeId AllocNode();
	void FreeNode(const TNodeId& nodeId);
	TNode *CheckOutNode(const TNodeId& nodeId);
	void CheckInNode(const TNodeId& nodeId, TNode *node, bool dirty);
	void Clr();
};

//----------------------------------------------------------------------------
// TBtreeOps
//----------------------------------------------------------------------------
//
// This class supports various operations on a B-tree and maintains various
// book-keeping information, but relies on two helper classes (TInternalStore,
// TLeafStore) to maintain the nodes themselves.
//
// Interface:
// - void         Clr()
// - TKeyLocation Add(TKey, TDat)  -- adds the key to the tree; duplicate keys are allowed, but might confuse some of the other functions
// - bool         Del(TKey)        -- if the key exists, deletes it and returns true; otherwise returns false; if multiple copies of the key exist, an arbitrary one is deleted
// - bool         Del(TKey, TDat&) -- same as above, but also returns the corresponding dat (if the key was found)
// - void         Dump(FILE *f)    -- generated a text dump of the tree
// - void         Validate()       -- IAsserts that the tree is structurally valid
// - int          RangeQuery_(minKey, maxKey, includeMin, includeMax, TSink&)  -- calls sink(key, dat) for each key from the range minKey <= key <= maxKey [or < if includeMin/includeMax is false]; returns # of calls made
// - int          RangeQuery(minKey, maxKey, TKeyV&) -- puts all keys in the range 'minKey <= key <= maxKey' into the destination vector
// - int          RangeQuery(minKey, maxKey, TKeyDatV&) -- puts (key, dat) for all keys in the range 'minKey <= key <= maxKey' into the destination vector
// - bool         IsKey(key)       -- returns true iff the given key is present in the tree
// - bool         IsKeyGetDat(key, TDat&) -- like IsKey(), but also returns the corresponding dat if the key is found
//
// A few sink classes for use with RangeQuery_ are also included> TKeySink, TKeyDatSink, TNullSink, TCountSink.
// Several of the above methods are actually just wrappers around RangeQuery_, using one of these sinks.

template <
	typename TKey_, typename TDat_, typename TComp_, typename TNodeId_,
    typename TInternalStore_, typename TLeafStore_>
class TBtreeOps
{
private:

	// Instantiating this template causes a compile-time error if T and U aren't the same class.
	template<typename T, typename U> class TSameClass { private: TSameClass(); };
	template<typename T> class TSameClass<T, T> { public: TSameClass() { } };

public:
	typedef TKey_ TKey;
	typedef TDat_ TDat;
	typedef TNodeId_ TNodeId;
    typedef TComp_ TComp;
	typedef TBtreeNode<TKey, TNodeId, TNodeId> TInternalNode; // should be the same as TInternalStore::TNode
	typedef TBtreeNode<TKey, TDat, TNodeId> TLeafNode; // should be the same as TLeafStore::TNode
	typedef TVec<TNodeId> TNodeIdV;
	typedef TInternalStore_ TInternalStore;
	typedef TLeafStore_ TLeafStore;
	typedef TPt<TInternalStore> PInternalStore;
	typedef TPt<TLeafStore> PLeafStore;
	typedef TVec<TKey> TKeyV;
	typedef TKeyDat<TKey, TDat> TKd;
	typedef TVec<TKd> TKdV;

    TComp cmp;
	TNodeId root; // root == first[0] == last[0]
	int nLevels; // when the tree is empty, nLevels = 1 as we have just the root node; otherwise, there will be at least one leaf, leading to nLevels >= 2
	TNodeIdV first, last; // first[h], last[h] = first/last node of level h; at h = 0, we have first = last = root; h = nLevels - 1 is the leaf level (unless nLevels == 1)
	int leafCapacity, internalCapacity; // the number of entries in each node must be at least 'capacity' and at most '2 * capacity - 1'; exceptionally, the root may have fewer entries, as may the leaf node if it's the only leaf node.
	PInternalStore internalStore;
	PLeafStore leafStore;
	// If ownXxxxStore is false, the tree won't save/load the store to streams during (de)serialization,
	// so it's up to the user to manage that, and to provide a store when loading the tree from the stream.
	bool ownInternalStore, ownLeafStore;

	TBtreeOps(
		const PInternalStore &InternalStore, const PLeafStore &LeafStore,
		int InternalCapacity, int LeafCapacity,
		bool OwnInternalStore, bool OwnLeafStore) :
        leafCapacity(LeafCapacity), internalCapacity(InternalCapacity),
		internalStore(InternalStore), leafStore(LeafStore),
		ownInternalStore(OwnInternalStore), ownLeafStore(OwnLeafStore)
	{
		Clr();
		// Make sure that the stores actually work with the correct type of nodes.
		// TBH if this isn't the case, there will probably be compile-time errors in other methods anyway.
		// TSameClass<TInternalStore::TNode, TInternalNode> dummy1;
		// TSameClass<TLeafStore::TNode, TLeafNode> dummy2;
	}

	// This constructor loads the tree from the stream, assuming that the stores were serialized
	// along with the tree (i.e. the serialized tree owned the stores).
	explicit TBtreeOps(TSIn& SIn) {
		Load(SIn);
		IAssert(ownInternalStore); IAssert(ownLeafStore); }

	// This constructor loads the tree from the stream, assuming that the serialized tree didn't own
	// the stores and the stores were therefore not serialized with the tree.  Therefore the caller
	// has to provide stores as parameters to this constructor call.
	TBtreeOps(TSIn& SIn, const PInternalStore &InternalStore, const PLeafStore &LeafStore) {
		Load(SIn);
		IAssert(! ownInternalStore); IAssert(! ownLeafStore);
		internalStore = InternalStore; leafStore = LeafStore; }

	void Load(TSIn& SIn) {
		root.Load(SIn); SIn.Load(nLevels);
		first.Load(SIn); last.Load(SIn);
		SIn.Load(internalCapacity); SIn.Load(leafCapacity);
		SIn.Load(ownInternalStore); SIn.Load(ownLeafStore);
		if (ownInternalStore) internalStore = PInternalStore(SIn);
		if (ownLeafStore) leafStore = PLeafStore(SIn);
		SIn.LoadCs(); }

	void Save(TSOut& SOut) const {
		root.Save(SOut); SOut.Save(nLevels);
		first.Save(SOut); last.Save(SOut);
		SOut.Save(internalCapacity); SOut.Save(leafCapacity);
		SOut.Save(ownInternalStore); SOut.Save(ownLeafStore);
		if (ownInternalStore) internalStore.Save(SOut);
		if (ownLeafStore) leafStore.Save(SOut);
		SOut.SaveCs(); }


	void Clr()
	{
		internalStore->Clr();
		leafStore->Clr();
		nLevels = 1; first.Clr(); last.Clr();
		root = internalStore->AllocNode();
		first.Add(root); last.Add(root);
		PInternalNode pRoot(internalStore, root, true);
		pRoot->Clr();
	}

protected:

	// This is an auto_ptr-style 'smart' pointer, i.e. it doesn't have reference
	// counting, and ownership is transferred on copying (i.e. by doing 'dest = src',
	// you cause 'src' to be set to null).  The class makes sure to check the node
	// back into the store before being destroyed/overwritten.  The user of the pointer
	// is responsible for setting the 'dirty' flag if *pNode gets modified.
	template<typename TStore_>
	class TNodeAutoPtr
	{
	public:
		typedef TStore_ TStore;
		typedef TPt<TStore> PStore;
		typedef typename TStore::TNodeId TNodeId;
		typedef typename TStore::TNode TNode;
	protected:
		TStore *store;
		TNodeId nodeId;
		TNode *pNode;
		bool dirty;
	public:
		void Set(const PStore &Store, TNodeId NodeId, bool Dirty = false) { CheckIn(); store = Store(); nodeId = NodeId; pNode = store->CheckOutNode(nodeId); dirty = Dirty; }
		void CheckIn() { if (pNode) store->CheckInNode(nodeId, pNode, dirty); pNode = 0; store = 0; nodeId = -1; dirty = false; }
		TNodeAutoPtr() : store(0), nodeId(-1), pNode(0), dirty(false) { }
		TNodeAutoPtr(const PStore &Store, TNodeId NodeId, bool Dirty = false) : store(Store()), nodeId(NodeId), dirty(Dirty) { pNode = store->CheckOutNode(nodeId); }
		TNodeAutoPtr(TNodeAutoPtr& other) : store(other.store), nodeId(other.nodeId), pNode(other.pNode), dirty(other.dirty) { other.store = 0; other.nodeId = -1; other.pNode = 0; }
		~TNodeAutoPtr() { CheckIn(); }
		bool Empty() const { return (! store) || (nodeId < 0) || (! pNode); }
		void SetDirty() { dirty = true; }
		TNodeId GetNodeId() { return nodeId; }
		TNodeAutoPtr& operator = (TNodeAutoPtr& other) {
			if (&other != this) {
				CheckIn(); store = other.store; nodeId = other.nodeId; pNode = other.pNode; dirty = other.dirty;
				other.store = 0; other.nodeId = -1; other.pNode = 0; other.dirty = false; }
			return *this; }
		TNode* operator->() { Assert(pNode != 0); return pNode; }
		const TNode* operator->() const { Assert(pNode != 0); return pNode; }
	};

	typedef TNodeAutoPtr<TInternalStore> PInternalNode;
	typedef TNodeAutoPtr<TLeafStore> PLeafNode;

public:

//----------------------------------------------------------------------------
// Adding keys
//----------------------------------------------------------------------------

	typedef TPair<TNodeId, TInt> TKeyLocation;

	// Adds the given key into the tree and returns a (nodeId, idx) pair indicating in which leaf, and at what index in that leaf's 'v' vector,
	// the new key has ended up.  Note that this location can easily change when the tree is next modified (e.g. the leaf might end up being
	// split or merged).
	TKeyLocation Add(const TKey& key, const TDat& dat = TDat())
	{
		TVec<TNodeId> branch;
		Assert(nLevels >= 1);
		branch.Gen(nLevels > 1 ? nLevels - 1 : 1); branch[0] = root;
		PInternalNode pRoot(internalStore, root);
		PLeafNode pLeaf;
		if (pRoot->v.Empty())
		{
			// The root is empty; create a leaf.
			IAssert(nLevels == 1);
			TNodeId leaf = leafStore->AllocNode();
			pLeaf.Set(leafStore, leaf, true);
			pLeaf->Clr(); int idx = pLeaf->v.Add(typename TLeafNode::TKd(key, dat));
			nLevels++; first.Add(leaf); last.Add(leaf);
			// Add the leaf to the root.
			pRoot->v.Add(typename TInternalNode::TKd(key, leaf));
			pRoot.SetDirty();
			return TKeyLocation(leaf, idx);
		}
		// Descend the tree until we find a suitable leaf.
		Assert(nLevels >= 2);
		PInternalNode pCur(pRoot);
		for (int level = 0; level < nLevels - 1; level++) {
			// The internal node is a collection of (maxKey, childId) pairs, where 'maxKey' is the greatest key
			// which currently appears in the subtree rooted in the child node 'childId'.  Find the first
			// child whose maxKey is >= the key that we're currently trying to add.  If there is no such child, this
			// means that the new key is greater than any key currently in the tree, and will be added to the
			// rightmost leaf in the tree.
			typename TInternalNode::TKdV &v = pCur->v;
			int n = v.Len(), idx = 0;
			while (idx + 1 < n && cmp(v[idx].Key, key) < 0) idx++; // ToDo: perhaps replace with bisection
			TNodeId child = v[idx].Dat;
			if (idx == n - 1 && cmp(v[idx].Key, key) < 0) { // This happens when the new key is greater than the current max of the tree.
				pCur->v[idx].Key = key; pCur.SetDirty(); }
			if (level == nLevels - 2) pLeaf.Set(leafStore, child, true);
			else { branch[level + 1] = child; pCur.Set(internalStore, child); }}
		// Now we reached the leaf where the new key should be added.
		Assert(! pLeaf.Empty());
		TNodeId leaf = pLeaf.GetNodeId();
		if (pLeaf->v.Len() < 2 * leafCapacity - 1)
		{
			int idx = pLeaf->v.Len(); pLeaf->v.Add();
			while (idx > 0 && cmp(key, pLeaf->v[idx - 1].Key) < 0) {
				pLeaf->v[idx] = pLeaf->v[idx - 1]; idx--; }
			pLeaf->v[idx].Key = key; pLeaf->v[idx].Dat = dat;
			return TKeyLocation(leaf, idx);
		}
		// The leaf node is at full capacity, so we'll have to split it.
		IAssert(pLeaf->v.Len() == 2 * leafCapacity - 1);
		TNodeId leaf2 = leafStore->AllocNode();
		TKeyLocation retVal(-1, -1);
		PLeafNode pLeaf2(leafStore, leaf2, true);
		{
			// The way splitting is implemented is as follows.  Let C be the leaf capacity, so that the
			// current leaf has the keys K = [k_0, ..., k_{2c-2}].  If we inserted the new key into the correct
			// position in this sequence, we'd get a new sequence K' = [k'_0, ..., k'_{2c-1}].  This now has to be
			// distributed between the two leaves, so that the new key gets k'_c, ..., k'_{2c-1} and the
			// old leaf ends up with just k'_0, ..., k'_{c-1}.  Now, the obvious way of getting K' from K
			// is to start at the end and work backwards through the list, moving each element one place down
			// until we reach the position where the new key needs to be added.  The code below also splits the
			// list K' at the same time, simply by storing the elements k'_c, ..., k'_{2c-1} into the new leaf
			// instead of the old one.
			IAssert(pLeaf->v.Len() == 2 * leafCapacity - 1);
			pLeaf2->v.Gen(leafCapacity);
			bool found = false;
			for (int dest = 2 * leafCapacity - 1; dest >= leafCapacity || ! found; dest--) // This loop needs to continue until everything that needs to end up in leaf2 has in fact been transferred there, and also until the new key has been written into the sequence.
			{
				typename TLeafNode::TKd &kdDest = (dest >= leafCapacity) ? pLeaf2->v[dest - leafCapacity] : pLeaf->v[dest];
				if (! found && (dest == 0 || cmp(pLeaf->v[dest - 1].Key, key) <= 0)) {
					kdDest.Key = key; kdDest.Dat = dat; found = true;
					retVal.Val1 = (dest >= leafCapacity) ? leaf2 : leaf;
					retVal.Val2 = (dest >= leafCapacity) ? dest - leafCapacity : dest; }
				else
					kdDest = pLeaf->v[dest - (found ? 0 : 1)];
			}
			IAssert(retVal.Val1 >= 0); IAssert(retVal.Val2 >= 0); IAssert(found);
			pLeaf->v.Reserve(pLeaf->v.Reserved(), leafCapacity); // truncate the old leaf
			IAssert(pLeaf->v.Len() == leafCapacity); IAssert(pLeaf2->v.Len() == leafCapacity);
			pLeaf2->prev = leaf; pLeaf2->next = pLeaf->next;
			pLeaf->next = leaf2;
			if (pLeaf2->next < 0) { IAssert(last[nLevels - 1] == leaf); last[nLevels - 1] = leaf2; }
			else {
				PLeafNode pNext(leafStore, pLeaf2->next, true);
				IAssert(pNext->prev == leaf); pNext->prev = leaf2; }
		}
		// Insert the new leaf into the parent node -- pCur is still pointing at it.
		TNodeId oldChild = leaf, newChild = leaf2;
		TKey oldChildMax = pLeaf->v.Last().Key, newChildMax = pLeaf2->v.Last().Key;
		pLeaf.CheckIn(); pLeaf2.CheckIn();
		for (int level = nLevels - 2; level >= 0; level--)
		{
			IAssert(pCur.GetNodeId() == branch[level]);
			pCur.SetDirty();
			// The internal node branch[level] (pointed to by 'pCur') currently includes a pointer to 'oldChild';
			// now a new sibling, 'newChild', needs to be inserted immediately after the old child.
			if (pCur->v.Len() < 2 * internalCapacity - 1)
			{
				typename TInternalNode::TKdV &v = pCur->v;
				int idx = v.Len(); v.Add();
				while (idx > 0 && v[idx - 1].Dat != oldChild) {
					v[idx] = v[idx - 1]; idx--; }
				IAssert(idx > 0); // This shouldn't happen -- surely something in the parent node must have been pointing at the old leaf 'nodeId'...
				v[idx].Key = newChildMax;
				v[idx].Dat = newChild;
				v[idx - 1].Key = oldChildMax;
				return retVal;
			}
			// The parent node is at full capacity, so it will have to be split.
			IAssert(pCur->v.Len() == 2 * internalCapacity - 1);
			TNodeId oldParent = pCur.GetNodeId();
			TNodeId newParent = internalStore->AllocNode();
			PInternalNode pCur2(internalStore, newParent, true);
			pCur2->v.Gen(internalCapacity);
			bool found = false;
			for (int dest = 2 * internalCapacity - 1; dest >= internalCapacity || ! found; dest--)
			{
				typename TInternalNode::TKd &kdDest = (dest >= internalCapacity) ? pCur2->v[dest - internalCapacity] : pCur->v[dest];
				if (! found && pCur->v[dest - 1].Dat == oldChild) // Note that inserting the new child onto index 0 isn't possible (unlike in the case of leaf splitting), as it must come after oldChild.
				{
					kdDest.Key = newChildMax; kdDest.Dat = newChild; found = true;
					pCur->v[dest - 1].Key = oldChildMax; // This is a good moment to update the key next to the pointer to the old child,
						// since the old child's max has probably been changed during the split.  If the loop continues (due to
						// dest still being > internalCapacity, the next iteration will transfer this into the suitable place in the new parent.
				}
				else
					kdDest = pCur->v[dest - (found ? 0 : 1)];
			}
			IAssert(found);
			pCur->v.Reserve(pCur->v.Reserved(), internalCapacity); // truncate the old parent
			IAssert(pCur->v.Len() == internalCapacity); IAssert(pCur2->v.Len() == internalCapacity);
			pCur2->prev = oldParent; pCur2->next = pCur->next;
			pCur->next = newParent;
			if (pCur2->next < 0) { IAssert(last[level] == oldParent); last[level] = newParent; }
			else {
				PInternalNode pNext(internalStore, pCur2->next, true);
				IAssert(pNext->prev == oldParent); pNext->prev = newParent; }
			// Prepare to move one level up.
			oldChild = oldParent; newChild = newParent;
			oldChildMax = pCur->v.Last().Key; newChildMax = pCur2->v.Last().Key;
			pCur2.CheckIn();
			if (level > 0) pCur.Set(internalStore, branch[level - 1]); else pCur.CheckIn();
		}
		// If we reached this point, this means that even the root had to be split, so we have to create
		// a new root and the tree will become one level deeper.
		IAssert(first[0] == oldChild); IAssert(last[0] == newChild); IAssert(oldChild == root);
		root = internalStore->AllocNode();
		first.Ins(0, root); last.Ins(0, root); nLevels++;
		pRoot.Set(internalStore, root, true);
		pRoot->v.Gen(2); pRoot->prev = -1; pRoot->next = -1;
		pRoot->v[0].Key = oldChildMax; pRoot->v[0].Dat = oldChild;
		pRoot->v[1].Key = newChildMax; pRoot->v[1].Dat = newChild;
		return retVal;
	}

//----------------------------------------------------------------------------
// Deletion of keys
//----------------------------------------------------------------------------

protected:

	// This method assumes that one of the nodes is at 'capacity - 1' entries and the other one
	// is normal (i.e. from 'capacity' to '2 * capacity - 1') entries.  If this other node is at
	// exactly 'capacity' entries, our method will move all entries entries from 'pRight' into
	// 'pLeft' and set 'deleted' to true, indicating that 'pRight' should be deleted from the tree.
	// Otherwise, the method sets 'deleted' to false and transfers some of the
	// entries from the bigger node to the smaller one; in this case, after the method returns, the nodes will
	// differ in size by at most 1, and both will have at least 'capacity' entries.
	template <typename PNode>
	void DelHelper(PNode &pLeft, PNode &pRight, int capacity, bool &deleted)
	{
		int nLeft = pLeft->v.Len(), nRight = pRight->v.Len();
		pLeft.SetDirty(); pRight.SetDirty();
		if ((nLeft == capacity - 1 && nRight == capacity) || (nLeft == capacity && nRight == capacity - 1)) {
			pLeft->v.AddV(pRight->v); pRight->v.Clr(); deleted = true; return; }
		deleted = false;
		Assert((nLeft == capacity - 1 && nRight > capacity) || (nLeft > capacity && nRight == capacity - 1));
		int transfer = abs(nRight - nLeft) / 2;
		if (nLeft < nRight) {
			for (int i = 0; i < transfer; i++) pLeft->v.Add(pRight->v[i]);
			pRight->v.Del(0, transfer - 1); }
		else {
			for (int i = 0; i < transfer; i++) pRight->v.Add();
			for (int i = nRight - 1; i >= 0; i--) pRight->v[i + transfer] = pRight->v[i];
			for (int i = 0; i < transfer; i++) pRight->v[i] = pLeft->v[nLeft - transfer + i];
			pLeft->v.Del(nLeft - transfer, nLeft - 1); }
		Assert(pLeft->v.Len() >= capacity); Assert(pRight->v.Len() >= capacity);
        Assert(pLeft->v.Len() + pRight->v.Len() == pLeft->v.Len() + pRight->v.Len());
	}

	// Deletes the key and returns 'true' if such a key was in fact found.  If 'dat' is not null, the
	// dat belonging to the deleted key is stored there.  If several copies of this key exist,
	// only one of them is deleted.
	bool Del_(const TKey& key, TDat *dat)
	{
		Assert(nLevels >= 1);
		if (nLevels == 1) return false; // we have just the root, therefore no leaves and therefore no keys either
		// Descend down the tree and store the IDs of the nodes in 'branch'.
		TVec<TNodeId> branch;
		branch.Gen(nLevels > 1 ? nLevels - 1 : 1); branch[0] = root;
		PLeafNode pLeaf; PInternalNode pNode;
		for (int level = 0; level < nLevels - 1; level++) {
			pNode.Set(internalStore, branch[level]);
			int n = pNode->v.Len(), i = 0;
			// Move into the first child whose maxKey is >= the key we're looking for.
			while (i < n && cmp(pNode->v[i].Key, key) < 0) i++; // ToDo: maybe use bisection
			if (i >= n) return false; // If no such child exists, our key is too large and doesn't exist in the tree.
			TNodeId desc = pNode->v[i].Dat;
			if (level == nLevels - 2) pLeaf.Set(leafStore, desc);
			else branch[level + 1] = desc; }
		// We reached a leaf; if the key is present in the tree, it has to be in this leaf.  (Possibly also in
		// some of the subsequent leaves if the key exists in multiple copies.)
		{
			int n = pLeaf->v.Len(), i = 0, cmpResult = 0; Assert(n > 0);
			while (i < n && (cmpResult = cmp(pLeaf->v[i].Key, key)) < 0) i++;
			if (i >= n || cmpResult != 0) return false; // the key doesn't exist in the tree
			if (dat) *dat = pLeaf->v[i].Dat;
			pLeaf->v.Del(i); pLeaf.SetDirty();
			if (i < n - 1 && pLeaf->v.Len() >= leafCapacity)
				// The leaf's maxKey hasn't changed and the leaf is not underfull, so we won't have to change anything
				// in the upper levels of the tree.
				return true;
		}
		// If the leaf is underfull, we have to borrow some keys from a sibling or even merge the two leaves.
		TNodeId leaf = pLeaf.GetNodeId(), leaf2 = -1;
		PLeafNode pLeaf2; bool nodeDeleted = false;
		if (pLeaf->v.Len() < leafCapacity)
		{
			Assert(pLeaf->v.Len() == leafCapacity - 1 || (pLeaf->v.Len() < leafCapacity - 1 && pLeaf->prev < 0 && pLeaf->next < 0 && nLevels == 2));
			int n = pNode->v.Len(), i = 0; while (i < n && pNode->v[i].Dat != leaf) i++; IAssert(i < n);
			if (i + 1 < n) {
				leaf2 = pNode->v[i + 1].Dat; pLeaf2.Set(leafStore, leaf2);
				DelHelper(pLeaf, pLeaf2, leafCapacity, nodeDeleted); }
			else if (i > 0) {
				leaf2 = leaf; pLeaf2 = pLeaf;
				leaf = pNode->v[i - 1].Dat; pLeaf.Set(leafStore, leaf);
				DelHelper(pLeaf, pLeaf2, leafCapacity, nodeDeleted); }
			else {
				// An internal node with just one child is allowed only if that internal node is the root.
				Assert(nLevels == 2); Assert(pNode.GetNodeId() == root);
				Assert(pNode->v.Len() == 1); Assert(pNode->v[0].Dat == leaf);
				if (pLeaf->v.Len() == 0)
				{
					// The leaf is now empty, so we should delete it and the tree will then consist only of the root.
					Assert(first[1] == leaf); Assert(last[1] == leaf);
					pLeaf.CheckIn(); leafStore->FreeNode(leaf);
					pNode->v.Clr(); pNode.SetDirty();
					first.DelLast(); last.DelLast(); nLevels--; return true;
				}
				// The leaf is not empty, but it's the only leaf of the tree, so we have to leave it as it is, even if it's underfull.
				nodeDeleted = false;
			}
			if (nodeDeleted)
			{
				// Remove the deleted node, 'leaf2', from the linked list.
				pLeaf->next = pLeaf2->next; pLeaf.SetDirty();
				Assert(pLeaf2->prev == leaf);
				if (pLeaf->next < 0) { Assert(last[nLevels - 1] == leaf2); last[nLevels - 1] = leaf; }
				else {
					PLeafNode pLeaf3(leafStore, pLeaf->next, true);
					Assert(pLeaf3->prev == leaf2); pLeaf3->prev = leaf; }
				// Delete it from the store.
				pLeaf2.CheckIn(); leafStore->FreeNode(leaf2);
			}
		}
		TNodeId child1 = leaf, child2 = leaf2;
		TKey child1Max = pLeaf->v.Last().Key, child2Max; if (child2 >= 0 && ! nodeDeleted) child2Max = pLeaf2->v.Last().Key;
		pLeaf.CheckIn(); pLeaf2.CheckIn();
		// Move up the tree, where we might have to update the maxKey values or perform further merges.
		for (int level = nLevels - 2; level >= 0; level--)
		{
			TNodeId node = branch[level]; IAssert(pNode.GetNodeId() == node);
			// At this point, we know the following:
			// - 'child1' is a child of 'node', and its maxKey is 'child1Max'.   The corresponding entry in 'node' doesn't necessarily contain the same maxKey
			//   (this can happen if child1 exchanged some entries with a sibling), in which case this entry will need to be updated;
			// - if 'nodeDeleted == true', then 'child2' was formerly also a child of 'node', but has recently been deleted.
			//   The corresponding entry in 'node' still exists and has to be removed now;
			// - if 'nodeDeleted == false' and 'child2 >= 0', then 'child2' is also a child of 'node' and its maxKey is 'child2Max'.
			//   The corresponding entry in 'node' doesn't necessarily contain the same maxKey and might need to be updated.
			// - if 'child2 >= 0', then 'child2' is (or was, if 'nodeDeleted == true') the immediate right sibling of 'child1'.
			int n = pNode->v.Len(), i;
			i = 0; while (i < n && pNode->v[i].Dat != child1) i++;
			Assert(i < n);
			if (child2 >= 0) { Assert(i + 1 < n); Assert(pNode->v[i + 1].Dat == child2); }
			TKey oldNodeMax = pNode->v.Last().Key;
			// Update the maxKey values of the children, if needed.
			if (cmp(pNode->v[i].Key, child1Max) != 0) { pNode.SetDirty(); pNode->v[i].Key = child1Max; }
			if (child2 >= 0 && ! nodeDeleted && cmp(pNode->v[i + 1].Key, child2Max) != 0) { pNode.SetDirty(); pNode->v[i + 1].Key = child2Max; }
			if (nodeDeleted) {
				// The node 'child2' has been deleted.  We have to remove it from pNode's list of children as well.
				pNode->v.Del(i + 1); pNode.SetDirty(); n--; }
			if (pNode->v.Len() >= internalCapacity && cmp(oldNodeMax, pNode->v.Last().Key) == 0)
				// The nodes's maxKey hasn't changed and the node is not underfull, so we won't have to change anything
				// in the upper levels of the tree.
				return true;
			//
			if (level == 0) break; // at the root level, we don't have to worry about underfull nodes
			// This is a good time to really start moving one level upwards.
			TNodeId parent = branch[level - 1], node2 = -1;
			PInternalNode pParent(internalStore, parent), pNode2;
			nodeDeleted = false;
			if (pNode->v.Len() < internalCapacity)
			{
				n = pParent->v.Len(), i = 0; while (i < n && pParent->v[i].Dat != node) i++; IAssert(i < n);
				if (i + 1 < n) {
					node2 = pParent->v[i + 1].Dat; pNode2.Set(internalStore, node2);
					DelHelper(pNode, pNode2, internalCapacity, nodeDeleted); }
				else if (i > 0) {
					node2 = node; pNode2 = pNode;
					node = pParent->v[i - 1].Dat; pNode.Set(internalStore, node);
					DelHelper(pNode, pNode2, internalCapacity, nodeDeleted); }
				else {
					// Now it seems that 'node' was already the only child of its parent even before the current deletion started.
					// This would be OK only if 'node' was a leaf and its parent was the root;
					// but we know that 'node' is an internal node, so this shoudn't be happening.
					IAssert(false); }
				if (nodeDeleted)
				{
					// Remove the deleted node, 'node2', from the linked list.
					pNode->next = pNode2->next; pNode.SetDirty();
					Assert(pNode2->prev == node);
					if (pNode->next < 0) { Assert(last[level] == node2); last[level] = node; }
					else {
						PInternalNode pNode3(internalStore, pNode->next, true);
						Assert(pNode3->prev == node2); pNode3->prev = node; }
					// Delete it from the store.
					pNode2.CheckIn(); internalStore->FreeNode(node2);
				}
			}
			child1 = node; child2 = node2;
			child1Max = pNode->v.Last().Key; if (child2 >= 0 && ! nodeDeleted) child2Max = pNode2->v.Last().Key;
			pNode = pParent;
		}
		// Now we're at the root; if it has just 1 child, and that child is an internal node, we should delete the root.
		Assert(pNode.GetNodeId() == root);
		Assert(pNode->v.Len() > 0);
		if (pNode->v.Len() == 1 && nLevels > 2)
		{
			TNodeId newRoot = pNode->v[0].Dat;
			Assert(first[1] == newRoot); Assert(last[1] == newRoot);
			pNode->v.Clr(); pNode.SetDirty(); pNode.CheckIn(); internalStore->FreeNode(root);
			root = newRoot; first.Del(0); last.Del(0); nLevels--;
		}
		return true;
	}

public:

	bool Del(const TKey& key) { return Del_(key, 0); }
	bool Del(const TKey& key, TDat &dat) { return Del_(key, &dat); }


//----------------------------------------------------------------------------
// Ranged queries
//----------------------------------------------------------------------------

protected:

	template<typename TSink_>
	class TRangeQueryData
	{
	public:
		typedef TSink_ TSink;
		const TKey &minKey, &maxKey;
		bool includeMin, includeMax;
		TSink &sink;
		bool stop;

		TRangeQueryData(const TKey& minKey_, const TKey& maxKey_, bool includeMin_, bool includeMax_, TSink& sink_) :
			minKey(minKey_), maxKey(maxKey_), includeMin(includeMin_), includeMax(includeMax_), sink(sink_), stop(false) { }
	};

	template<typename TSink>
	int RangeQuery_Recursion(TRangeQueryData<TSink> &Q, TNodeId node, int level) const
	{
		int retVal = 0;
		if (level < nLevels - 1)
		{
			PInternalNode pNode(internalStore, node);
			const typename TInternalNode::TKdV &v = pNode->v; int n = v.Len();
			for (int i = 0; i < n; i++)
			{
				int c = cmp(v[i].Key, Q.minKey);
				if (c < 0 || (c == 0 && ! Q.includeMin))
					continue; // This subtree lies entirely to the left of the query range.
				int nResultsInSubtree = RangeQuery_Recursion(Q, v[i].Dat, level + 1);
				retVal += nResultsInSubtree;
				if (Q.stop) break;
				if (nResultsInSubtree < 0) {
					// Since the subtree does not lie entirely to the left of the query range,
					// the only explanation why there are no results in the subtree is that it lies
					// entirely to the right of the query range.
					// - Among other things, this means that the subtree's max is also to the right of the query range.
					Assert((c = cmp(v[i].Key, Q.maxKey), (c > 0 || (c == 0 && ! Q.includeMax))));
					// If this subtree lies entirely to the right of the query range, all subsequent subtrees will do so as well.
					Q.stop = true; break; }
			}
		}
		else
		{
			PLeafNode pNode(leafStore, node);
			const typename TLeafNode::TKdV &v = pNode->v; int n = v.Len();
			for (int i = 0; i < n; i++)
			{
				int c = cmp(v[i].Key, Q.minKey);
				if (c < 0 || (c == 0 && ! Q.includeMin))
					continue; // This key is too small.
				c = cmp(v[i].Key, Q.maxKey);
				if (c > 0 || (c == 0 && ! Q.includeMax)) {
					Q.stop = true; break; }
				retVal++;
				if (! Q.sink(v[i].Key, v[i].Dat)) {
					Q.stop = true; break; }
			}
		}
		return retVal;
	}

public:

	// TSink must implement 'bool operator()(const TKey&, const TDat&)'.  This operator should return true to continue, false to abort.
	// IncludeMin/Max specify whether the minKey and maxKey are considered part of the query range or not;
	// in other words, if we write the query condition as 'minKey op x op maxKey', the question is whether
	// the 'op's should be <= or <.
	// Returns the number of calls made to 'sink'.  The sink will receive keys in ascending order.
	template<typename TSink>
	int RangeQuery_(const TKey& minKey, const TKey& maxKey, bool includeMin, bool includeMax, TSink& sink) const
	{
		int retVal = 0;
		Assert(nLevels >= 1);
		if (nLevels == 1) return retVal; // we have just the root, therefore no leaves and therefore no keys either
		TRangeQueryData<TSink> Q(minKey, maxKey, includeMin, includeMax, sink);
		return RangeQuery_Recursion(Q, root, 0);
	}

public:

	struct TKeySink {
		TKeyV &dest;
		TKeySink(TKeyV &dest_) : dest(dest_) { }
		bool operator()(const TKey &key, const TDat &dat) { dest.Add(key); return true; } };

	struct TKeyDatSink {
		TKdV &dest;
		TKeyDatSink(TKdV &dest_) : dest(dest_) { }
		bool operator()(const TKey &key, const TDat &dat) { dest.Add(TKd(key, dat)); return true; } };

	struct TNullSink {
		bool operator()(const TKey &key, const TDat &dat) { return true; }};

	struct TFindSink {
		bool found; TDat *dat;
		TFindSink(TDat *dat_ = 0) : found(false), dat(dat_) { }
		bool operator()(const TKey &key, const TDat &dat_) { found = true; if (dat) *dat =dat_; return false; } };

public:

	int RangeQuery(const TKey& minKey, const TKey& maxKey, TKeyV& dest, bool ClrDest = true) const {
		if (ClrDest) dest.Clr();
		TKeySink sink(dest); return RangeQuery_(minKey, maxKey, true, true, sink); }

	int RangeQuery(const TKey& minKey, const TKey& maxKey, TKdV& dest, bool ClrDest = true) const {
		if (ClrDest) dest.Clr();
		TKeyDatSink sink(dest); return RangeQuery_(minKey, maxKey, true, true, sink); }

	bool IsKey(const TKey& key) const {
		TFindSink sink; RangeQuery_(key, key, true, true, sink); return sink.found; }

	bool IsKeyGetDat(const TKey& key, TDat& dat) const {
		TFindSink sink(&dat); RangeQuery_(key, key, true, true, sink); return sink.found; }

//----------------------------------------------------------------------------
// Debug funtions
//----------------------------------------------------------------------------

protected:

	template<class PNode>
	void DumpNode(FILE *f, const PNode &pNode)
	{
		static TPrinter<typename PNode::TNode::TKey> keyPrinter;
		static TPrinter<typename PNode::TNode::TDat> datPrinter;
		fprintf(f, "prev = %d, next = %d; %d entries: [", int(pNode->prev), int(pNode->next), int(pNode->v.Len()));
		for (int i = 0; i < pNode->v.Len(); i++) {
			if (i > 0) fprintf(f, ", ");
			keyPrinter(f, pNode->v[i].Key); fprintf(f, ": ");
			datPrinter(f, pNode->v[i].Dat); }
		fprintf(f, "]\n");
	}

public:

	void Dump(FILE *f = 0)
	{
		if (! f) f = stdout;
		fprintf(f, "\nB-tree dump\n");
		fprintf(f, "nLevels = %d, internal capacity = %d..%d, leaf capacity = %d..%d, root = %d, first = [",
			nLevels, internalCapacity, 2 * internalCapacity - 1, leafCapacity, 2 * leafCapacity - 1,
			int(root));
		for (int i = 0; i < nLevels; i++) fprintf(f, "%s%d", (i > 0) ? ", " : "", int(first[i]));
		fprintf(f, "], last = [");
		for (int i = 0; i < nLevels; i++) fprintf(f, "%s%d", (i > 0) ? ", " : "", int(last[i]));
		fprintf(f, "]\n");
		int nAllNodes = 0, nKeys = 0; TIntV nodesByLevel;
		for (int level = 0; level < nLevels; level++) {
			int nNodesOnLevel = 0;
			for (TNodeId node = first[level]; node >= 0; ) {
				fprintf(f, "- Level %d, node %d [id = %d]: ", level, nNodesOnLevel, int(node));
				TNodeId next = -1;
				if (level > 0 && level == nLevels - 1) {
					PLeafNode pLeaf(leafStore, node);
					nKeys += pLeaf->v.Len(); next = pLeaf->next;
					DumpNode(f, pLeaf); }
				else {
					PInternalNode pNode(internalStore, node);
					DumpNode(f, pNode); next = pNode->next; }
				nNodesOnLevel++; node = next; }
			nAllNodes += nNodesOnLevel; nodesByLevel.Add(nNodesOnLevel); }
		fprintf(f, "%d keys, %d nodes; nodes by level: [", nKeys, nAllNodes);
		for (int i = 0; i < nLevels; i++) fprintf(f, "%s%d", (i > 0) ? ", " : "", int(nodesByLevel[i]));
		fprintf(f, "]\n");
	}

protected:

	typedef TPair<TNodeId, TKey> TNodeKeyPr;
	typedef TVec<TNodeKeyPr> TNodeKeyPrV;

	// Returns some information from the node: 'prev', 'next', and the key from the last entry of the node (in 'maxKey').
	// It also asserts that all the keys in the node are in ascending order.
	template<class PNode>
	void Validate_ProcessNode(const PNode &pNode, TNodeId &prev, TNodeId &next, TKey &maxKey, int &nEntries)
	{
		prev = pNode->prev; next = pNode->next;
		nEntries = pNode->v.Len();
		if (nEntries <= 0) maxKey = TKey(); else maxKey = pNode->v.Last().Key;
		for (int i = 1; i < nEntries; i++) IAssert(cmp(pNode->v[i - 1].Key, pNode->v[i].Key) <= 0);
	}

public:

	void Validate()
	{
		IAssert(nLevels > 0); IAssert(first.Len() == nLevels); IAssert(last.Len() == nLevels);
		IAssert(first[0] == root); IAssert(last[0] == root);
		IAssert(root >= 0);
		THash<TNodeId, TVoid> seenInternal, seenLeaves;
		//
		TNodeKeyPrV curLevel;
		PInternalNode pRoot(internalStore, root);
		curLevel.Add(); curLevel[0].Val1 = root;
		if (pRoot->v.Empty()) { IAssert(nLevels == 1); curLevel[0].Val2 = TKey(); }
		else curLevel[0].Val2 = pRoot->v.Last().Key;
		pRoot.CheckIn();
		//
		for (int level = 0; level < nLevels; level++)
		{
			// 'curLevel' contains a list of (nodeId, key) pairs, obtained from the contents of the internal nodes
			// at level 'level - 1'.  This is the list of nodes that *should* appear on the current level, based on
			// the information from the parent level.  Now we'll traverse the actual list of nodes on the current level,
			// starting with 'first[level]' and following the 'next' pointes from there.  We'll verify whether
			// this list matches what we obtained from the parent level (and is now in 'curLevel').  We'll also
			// prepare a similar list for the next level ('level + 1'), storing it in 'nextLevel'.
			// Now we'll go through the nodes
			TNodeKeyPrV nextLevel;
			for (int i = 0; i < curLevel.Len(); i++)
			{
				TNodeId node = curLevel[i].Val1, prev, next; TKey expectedMaxKey = curLevel[i].Val2, maxKey;
				int nEntries;
				if (level > 0 && level == nLevels - 1) {
					PLeafNode pNode(leafStore, node);
					Validate_ProcessNode(pNode, prev, next, maxKey, nEntries);
					IAssert(nEntries < 2 * leafCapacity);
					if (level == 1 && first[level] == node && last[level] == node) IAssert(nEntries > 0);
					else IAssert(nEntries >= leafCapacity);
					for (int j = 0; j < nEntries; j++) nextLevel.Add(TNodeKeyPr(-1, pNode->v[j].Key)); }
				else {
					PInternalNode pNode(internalStore, node);
					Validate_ProcessNode(pNode, prev, next, maxKey, nEntries);
					IAssert(nEntries < 2 * internalCapacity);
					if (level == 0) IAssert(nEntries >= 0); else IAssert(nEntries >= internalCapacity);
					for (int j = 0; j < nEntries; j++) nextLevel.Add(TNodeKeyPr(pNode->v[j].Dat, pNode->v[j].Key)); }
				//
				if (i == 0) { IAssert(prev == -1); IAssert(first[level] == node); }
				else IAssert(prev == curLevel[i - 1].Val1);
				if (i == curLevel.Len() - 1) { IAssert(next == -1); IAssert(last[level] == node); }
				else IAssert(next == curLevel[i + 1].Val1);
				IAssert(cmp(maxKey, expectedMaxKey) == 0);
				IAssert(maxKey == expectedMaxKey); // they shouldn't just compare the same, they should actually be the same, as the max key from the child should have been copied into the parent (the one in 'expectedMaxKey' came from 'curLevel' and thus ultimately from the parent level)
			}
			for (int i = 1; i < nextLevel.Len(); i++)
				IAssert(cmp(nextLevel[i - 1].Val2, nextLevel[i].Val2) <= 0);
			curLevel = nextLevel;
		}
	}

};


//----------------------------------------------------------------------------
// TBtreeNodeMemStore
//----------------------------------------------------------------------------
//
// This store keeps all the nodes in memory; each node is wrapped in a simple
// wrapper class which allows us to point to it with a TPt-based smart pointer.

template <typename TKey_, typename TDat_, typename TNodeId_>
class TBtreeNodeMemStore
{
public:
	TCRef CRef;
	typedef TKey_ TKey;
	typedef TDat_ TDat;
	typedef TNodeId_ TNodeId;
	typedef TBtreeNode<TKey, TDat, TNodeId> TNode;
	typedef TVec<TNodeId> TNodeIdV;

protected:
	class TNodeWrapper;
	typedef TPt<TNodeWrapper> PNodeWrapper;
	typedef TVec<PNodeWrapper> TNodeWrapperV;
	class TNodeWrapper
	{
	private:
		TCRef CRef;
		friend class TPt<TNodeWrapper>;
	public:
		TNode node;
		TNodeWrapper() { }
		explicit TNodeWrapper(TSIn& SIn) : node(SIn) { }
		static PNodeWrapper Load(TSIn& SIn) { return new TNodeWrapper(SIn); }
		void Save(TSOut& SOut) const { node.Save(SOut); }
	};

	TNodeWrapperV nodes;
	TNodeIdV freeNodes;

public:

	TBtreeNodeMemStore() { }
	TBtreeNodeMemStore(TSIn& SIn) : nodes(SIn), freeNodes(SIn) { }
	static TPt<TBtreeNodeMemStore> Load(TSIn &SIn) { return new TBtreeNodeMemStore(SIn); }
	void Save(TSOut &SOut) const { nodes.Save(SOut); freeNodes.Save(SOut); }

	TNodeId AllocNode() {
		TNodeId node;
		if (freeNodes.Empty()) node = nodes.Add();
		else { node = freeNodes.Last(); freeNodes.DelLast(); Assert(node >= 0); Assert(node < nodes.Len()); Assert(nodes[node].Empty()); }
		nodes[node] = new TNodeWrapper();
		return node; }

	void FreeNode(const TNodeId& nodeId) {
		Assert(nodeId >= 0); IAssert(nodeId < nodes.Len());
		Assert(! nodes[nodeId].Empty());
		nodes[nodeId] = 0;
		freeNodes.Add(nodeId); }

	TNode *CheckOutNode(const TNodeId& nodeId)
	{
		Assert(nodeId >= 0); IAssert(nodeId < nodes.Len());
		Assert(! nodes[nodeId].Empty());
		TNodeWrapper &w = *nodes[nodeId];
		return &w.node;
	}

	void CheckInNode(const TNodeId& nodeId, TNode *node, bool dirty)
	{
		Assert(nodeId >= 0); Assert(nodeId < nodes.Len());
		Assert(! nodes[nodeId].Empty());
//		TNodeWrapper &w = *nodes[nodeId];
//		Assert(*nodes[nodeId] == &w.node);
	}

	void Clr() {
		int nUsed = 0, nFree = 0;
		for (TNodeId node = 0; node < nodes.Len(); node++) {
			if (nodes[node].Empty()) { nFree++; continue; }
			nUsed++; }
		Assert(nFree == freeNodes.Len());
		nodes.Clr(); freeNodes.Clr(); }

	// Dummy method for compatibility with TBtreeNodeMemStore_Paranoid.
	void IAssertNoCheckouts() { }
};

//----------------------------------------------------------------------------
// TBtreeNodeMemStore_Paranoid
//----------------------------------------------------------------------------
//
// A paranoid B-tree node store, useful for debugging purposes.  It keeps all nodes
// in memory and makes sure that the user (TBtreeOps) sticks to their side of the contract
// (i.e. no nested checkouts of the same node; eventually check in every node that was checked out;
// don't delete checked-out nodes; set the 'dirty' flag if the node was modified).

template <typename TKey_, typename TDat_, typename TNodeId_>
class TBtreeNodeMemStore_Paranoid
{
public:
	TCRef CRef;
	typedef TKey_ TKey;
	typedef TDat_ TDat;
	typedef TNodeId_ TNodeId;
	typedef TBtreeNode<TKey, TDat, TNodeId> TNode;
	typedef TVec<TNodeId> TNodeIdV;

protected:
	class TNodeWrapper;
	typedef TPt<TNodeWrapper> PNodeWrapper;
	typedef TVec<PNodeWrapper> TNodeWrapperV;
	class TNodeWrapper
	{
	private:
		TCRef CRef;
		friend class TPt<TNodeWrapper>;
	public:
		TNodeId nodeId;
		int checkedOut;
		TNode node;
		TNode *checkedOutNode;

		explicit TNodeWrapper(const TNodeId &NodeId) : nodeId(NodeId), checkedOut(0), checkedOutNode(0) { }
		explicit TNodeWrapper(TSIn &SIn) : nodeId(SIn), checkedOut(TInt(SIn)), node(SIn), checkedOutNode(0) { IAssert(checkedOut == 0); }
		static PNodeWrapper Load(TSIn &SIn) { return new TNodeWrapper(SIn); }
		void Save(TSOut &SOut) const { nodeId.Save(SOut); SOut.Save(checkedOut); node.Save(SOut); IAssert(checkedOut == 0); IAssert(checkedOutNode == 0); }

		void IAssertNodeEqualsBackup() const {
			IAssert(checkedOutNode);
			IAssert(node.prev == checkedOutNode->prev);
			IAssert(node.next == checkedOutNode->next);
			IAssert(node.v.Reserved() == checkedOutNode->v.Reserved());
			IAssert(node.v.Len() == checkedOutNode->v.Len());
			for (int i = 0; i < node.v.Len(); i++) IAssert(node.v[i] == checkedOutNode->v[i]); }
	};

	TNodeWrapperV nodes;
	TNodeIdV freeNodes;

public:

	TBtreeNodeMemStore_Paranoid() { }
	TBtreeNodeMemStore_Paranoid(TSIn& SIn) : nodes(SIn), freeNodes(SIn) { }
	static TPt<TBtreeNodeMemStore_Paranoid> Load(TSIn &SIn) { return new TBtreeNodeMemStore_Paranoid(SIn); }
	void Save(TSOut &SOut) const { nodes.Save(SOut); freeNodes.Save(SOut); }

	TNodeId AllocNode() {
		TNodeId node;
		if (freeNodes.Empty()) node = nodes.Add();
		else { node = freeNodes.Last(); freeNodes.DelLast(); IAssert(node >= 0); IAssert(node < nodes.Len()); IAssert(nodes[node].Empty()); }
		nodes[node] = new TNodeWrapper(node);
		return node; }

	void FreeNode(const TNodeId& nodeId) {
		IAssert(nodeId >= 0); IAssert(nodeId < nodes.Len());
		IAssert(! nodes[nodeId].Empty());
		IAssert(nodes[nodeId]->nodeId == nodeId);
		IAssert(nodes[nodeId]->checkedOut == 0);
		IAssert(nodes[nodeId]->checkedOutNode == 0);
		IAssert(nodes[nodeId]->node.v.Empty()); // possibly this should be just a warning, depends on how the tree is using the store
		nodes[nodeId] = 0;
		freeNodes.Add(nodeId); }

	TNode *CheckOutNode(const TNodeId& nodeId)
	{
		IAssert(nodeId >= 0); IAssert(nodeId < nodes.Len());
		IAssert(! nodes[nodeId].Empty());
		TNodeWrapper &w = *nodes[nodeId];
		IAssert(w.nodeId == nodeId);
		IAssert(w.checkedOut == 0);
		IAssert(w.checkedOutNode == 0);
		w.checkedOut++;
		// Make a copy of this node's TNode structure on the heap
		// and provide this copy to the caller.
		w.checkedOutNode = (TNode *) malloc(sizeof(TNode));
		new (w.checkedOutNode) TNode(w.node);
		w.IAssertNodeEqualsBackup();
		return w.checkedOutNode;
	}

	void CheckInNode(const TNodeId& nodeId, TNode *node, bool dirty)
	{
		IAssert(nodeId >= 0); IAssert(nodeId < nodes.Len());
		IAssert(! nodes[nodeId].Empty());
		TNodeWrapper &w = *nodes[nodeId];
		IAssert(w.nodeId == nodeId);
		IAssert(w.checkedOut == 1);
		IAssert(w.checkedOutNode);
		IAssert(node == w.checkedOutNode);
		if (! dirty)
			// The caller says he hasn't modified the node while it was checked out; verify if this is true.
			w.IAssertNodeEqualsBackup();
		else {
			// Store the modified node from w.checkedOutNode back into w.node.  We use the copy constructor
			// instead of the assignment operator because TVec's assignment operator doesn't transfer MxVals.
			w.node.~TNode();
			new (&(w.node)) TNode(*(w.checkedOutNode)); }
		// Discard the checkedOutNode.  We're being highly paranoid and use malloc/free instead of new/delete
		// so that we can make sure to overwrite the memory with junk data before finally freeing it.
		// (Of course, generally the debug version of the standard library will do this in delete/free anyway.)
		// This should help us catch any errors where the user might be trying to use the old TNode* after it has
		// been checked in.
		w.checkedOutNode->~TNode();
		memset(w.checkedOutNode, 0xcd, sizeof(TNode));
		free(w.checkedOutNode);
		w.checkedOut--;
		w.checkedOutNode = 0;
	}

	void Clr() {
		int nUsed = 0, nFree = 0;
		for (TNodeId node = 0; node < nodes.Len(); node++) {
			if (nodes[node].Empty()) { nFree++; continue; }
			IAssert(nodes[node]->nodeId == node);
			IAssert(nodes[node]->checkedOut == 0);
			nUsed++; }
		IAssert(nFree == freeNodes.Len());
		nodes.Clr(); freeNodes.Clr(); }

	void IAssertNoCheckouts() {
		for (TNodeId node = 0; node < nodes.Len(); node++) {
			if (nodes[node].Empty()) continue;
			IAssert(nodes[node]->checkedOut == 0); }}
};

}

#endif // ____BTREE_H_INCLUDED____
