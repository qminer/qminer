/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef ____SPHERENN_H_INCLUDED____
#define ____SPHERENN_H_INCLUDED____

// TSphereNn maintains a set of (key, latitude, longitude) records and can perform
// spatial queries over them.  Internally, the keys will be stored in a hash table
// and indexed by an oct-tree.
//
// Template parameters:
// - TKey_ is the type used for keys.  Each record must have a unique key.
//   The same requirements apply as to keys in a hash table.
// - TCoord_ is the type used for coordinates and angles.  The default is 'double', which 
//   should be OK in most circumstances.
// - useDegrees tells if the latitudes and longitudes will be in degrees
//   instead of in radians.
//
// Public methods:
// - TSphereNn(radius, maxKeysPerLeaf) - constructor
// - Clr - clears the hash table and the oct-tree
// - EarthRadiusKm - useful as a value of 'radius' when calling the constructor
// - LatLonToCart - convert a point from (lat, lon) to (x, y, z).
// - GetDist - returns the shortest distance (on the surface of the sphere, 
//   i.e. along a great circle) between the two points.
//   Note: LatLonToCart and GetDist exist in two variants each, as a static
//   method and a normal method.  The static method takes the radius of the
//   sphere as a parameter; the normal method is a wrapper that uses the
//   radius from this->radius.
// - GetKeyCoords - looks up the key in the 'keys' hash table and returns its coordinates
// - IsKey, Empty, Len
// - AddKey(key, lat, lon) - adds the key into the data structure.  
//   If such a key already exists, an assertion failure occurs.
// - RangeQuery(lat, lon, dist, TVec<TKey> dest) - finds all points within
//   distance 'dist' (on the surface of the sphere) of the point (lat, lon)
//   and adds their keys to the 'dest' vector
// - RangeQueryT - a generic version of RangeQuery; instead of adding
//   results to a vector, it calls a callback object provided by the caller
// - NnQuery(lat, lon, count, dest) - finds the 'count' nearest neighbors
//   to the query point (lat, lon) and adds their keys to 'dest', in
//   increasing order of distance from the query point.  If there are
//   several points at the same distance, their order is undefined (as is
//   which of them makes it into the result list at all).  If 'count'
//   is greater than the total number of points in the data structure,
//   all of them are returned.
// - Dump - prints the tree to a text file and makes a few assertions about its structure
//
// IMPORTANT NOTE: preferably no two keys should have exactly the same
// coordinates.  THE DATA STRUCTURE DOES NOT SUPPORT MORE THAN
// 'maxKeysPerLeaf' KEYS WITH EXACTLY THE SAME COORDINATES, and will
// behave unpredictably if this happens.  The reason for this is that
// if more than maxKeysPerLeaf keys end up in the same leaf of the tree, 
// the class will try to split this leaf into child nodes, but there's
// no way in an oct-tree to split points with the same coordinates among
// several different nodes.

template <typename TCoord, bool primitive> class TSphereNnCoordIo { public:
	inline static TCoord Load(TSIn& SIn) { return TCoord(SIn); }
	inline static void Save(TSOut& SOut, const TCoord& x) { x.Save(SOut); }};
template <typename TCoord> class TSphereNnCoordIo<TCoord, true> { public:
	inline static TCoord Load(TSIn& SIn) { TCoord x; SIn.Load(x); return x; }
	inline static void Save(TSOut& SOut, const TCoord x) { SOut.Save(x); }};
template <typename TCoord> class TSphereNnCoordTraits { public: enum { primitive = false }; };
#define _(T) template <> class TSphereNnCoordTraits<T> { public: enum { primitive = true }; };
_(float) _(double) _(long double) _(int) _(uint) _(int64) _(uint64) _(short) _(ushort)
#undef _

template <typename TCoord, bool useDeg> class TDegToRad_ { public:
	static inline TCoord ToRad(TCoord coord) { return coord; }
	static inline TCoord FromRad(TCoord coord) { return coord; } };
template <typename TCoord> class TDegToRad_<TCoord, true> { public:
	static inline TCoord ToRad(TCoord coord) { return (coord * TMath::Pi) / TCoord(180);; }
	static inline TCoord FromRad(TCoord coord) { return (coord * 180) / TMath::Pi; } };

template <typename TKey_, typename TCoord_ = double, bool useDegrees = true>
class TSphereNn
{
public:
	typedef TKey_ TKey;
	typedef TVec<TKey> TKeyV;
	typedef TCoord_ TCoord;
	typedef TSphereNnCoordIo<TCoord, TSphereNnCoordTraits<TCoord>::primitive> TCoordIo;
	
	typedef TDegToRad_<TCoord, useDegrees> TDegToRad;

	inline static int LoadInt(TSIn& SIn) { int i; SIn.Load(i); return i; }

	class TKeyVecSink {
	private:
		TKeyV& v;
	public:
		TKeyVecSink(TKeyV& V) : v(V) { }
		TKeyVecSink& operator()(const TKey& key, const TCoord lat, const TCoord lon) { v.Add(key); return *this; }
	};

	class TPoint3
	{
	public:
		TCoord x, y, z;

		double Dist2(const TPoint3 &t) const { 
			double dx = t.x - x, dy = t.y - y, dz = t.z - z;
			return dx * dx + dy * dy + dz * dz; }
		TPoint3() { } 
		TPoint3(TSIn& SIn) : x(TCoordIo::Load(SIn)), y(TCoordIo::Load(SIn)), z(TCoordIo::Load(SIn)) { }
		void Load(TSIn& SIn) { x = TCoordIo::Load(SIn); y = TCoordIo::Load(SIn); z = TCoordIo::Load(SIn); }
		void Save(TSOut& SOut) const { TCoordIo::Save(SOut, x); TCoordIo::Save(SOut, y); TCoordIo::Save(SOut, z); }
	};

	class TBbox
	{
	public:
		TCoord xMin, xMax, yMin, yMax, zMin, zMax;
		TCoord MinDist2(const TCoord x, const TCoord y, const TCoord z) const {
			TCoord dx = (x < xMin) ? (xMin - x) : (x > xMax ? x - xMax : 0);
			TCoord dy = (y < yMin) ? (yMin - y) : (y > yMax ? y - yMax : 0);
			TCoord dz = (z < zMin) ? (zMin - z) : (z > zMax ? z - zMax : 0);
			return dx * dx + dy * dy + dz * dz; }
		TCoord MinDist2(const TPoint3 &pt) const {
			return MinDist2(pt.x, pt.y, pt.z); }
		TBbox() { }
		TBbox(TSIn& SIn) : xMin(TCoordIo::Load(SIn)), xMax(TCoordIo::Load(SIn)), yMin(TCoordIo::Load(SIn)), yMax(TCoordIo::Load(SIn)), zMin(TCoordIo::Load(SIn)), zMax(TCoordIo::Load(SIn)) { }
		void Load(TSIn& SIn) { xMin = TCoordIo::Load(SIn); xMax = TCoordIo::Load(SIn); yMin = TCoordIo::Load(SIn); yMax = TCoordIo::Load(SIn); zMin = TCoordIo::Load(SIn); zMax = TCoordIo::Load(SIn); }
		void Save(TSOut& SOut) const { TCoordIo::Save(SOut, xMin); TCoordIo::Save(SOut, xMax); TCoordIo::Save(SOut, yMin); TCoordIo::Save(SOut, yMax); TCoordIo::Save(SOut, zMin); TCoordIo::Save(SOut, zMax); }
	};

	class TNode
	{
	public:
		int children[2][2][2]; // [x][y][z]
		int parent; // Note: this field is not saved by Save(), to ensure binary compatibility with old files.  It is reconstructed by TSphereNn::Load().
		TBbox bbox;
		int nPoints; // 0 if it's an internal node; -1 if it's an unused TNode structure (due to deletion of keys etc.), in which case 'lastPoint' points to the next unused TNode structure
		int firstPoint; // -1 for internal nodes; otherwise a keyId into the hash table
		int lastPoint; // Note: this field is not saved by Save(), to ensure binary compatibility with old files.  It is reconstructed by TSphereNn::Load().

		TNode() : nPoints(0), firstPoint(-1), lastPoint(-1) { 
			for (int i = 0; i < 8; i++) children[(i >> 2) & 1][(i >> 1) & 1][i & 1] = -1; }
		TNode(TSIn& SIn) { 
			for (int i = 0; i < 8; i++) SIn.Load(children[(i >> 2) & 1][(i >> 1) & 1][i & 1]);
			parent = -1; lastPoint = -1; // will be reconstructed by TSphereNn::Load.
			bbox.Load(SIn); SIn.Load(nPoints); SIn.Load(firstPoint); }
		void Save(TSOut& SOut) const {
			for (int i = 0; i < 8; i++) SOut.Save(children[(i >> 2) & 1][(i >> 1) & 1][i & 1]);
			bbox.Save(SOut); SOut.Save(nPoints); SOut.Save(firstPoint); }
		bool IsUnused() const { return nPoints < 0; }
		int NextUnusedNode() const { IAssert(IsUnused()); return lastPoint; }
	};

	class TKeyEntry
	{
	public:
		TCoord lat, lon; 
		TPoint3 pt; // on the unit sphere!
		int nodeId; // index into 'nodes'
		int next; // keyId of the next member of the same node
		int prev; // keyId of the previous member of the same node; is not saved by Save(), for binary compatibility with old files; is reconstructed by TSphereNn::Load().

		TKeyEntry() { }
		TKeyEntry(TSIn& SIn) : lat(TCoordIo::Load(SIn)), lon(TCoordIo::Load(SIn)), pt(SIn), nodeId(LoadInt(SIn)), next(LoadInt(SIn)), prev(-1) { }
		void Save(TSOut& SOut) const { TCoordIo::Save(SOut, lat); TCoordIo::Save(SOut, lon); pt.Save(SOut); SOut.Save(nodeId); SOut.Save(next); } 
	};

	typedef TVec<TNode> TNodeV;
	typedef THash<TKey, TKeyEntry> TKeyHash;

	template<typename T> class TLess { public:
		bool operator()(const T& a, const T& b) const { return a < b; } };
	template<typename T> class TGreater { public:
		bool operator()(const T& a, const T& b) const { return a > b; } };

	template<typename THeapKey_, typename THeapDat_, typename TCmp_>
	class THeap
	{
	public:
		typedef THeapKey_ TKey; typedef THeapDat_ TDat; typedef TCmp_ TCmp;
		typedef TKeyDat<TKey, TDat> TEntry;
		TVec<TEntry> v;
		TCmp cmp;
		void Clr() { v.Clr(); }
		int Len() const { return v.Len(); }
		int Sift(int i) {
			int n = v.Len(); TEntry ent = v[i];
			while (2 * i + 1 < n) {
				int ci = 2 * i + 1;
				if (ci + 1 < n && cmp(v[ci + 1].Key, v[ci].Key)) ci += 1;
				if (! cmp(v[ci].Key, ent.Key)) break;
				v[i] = v[ci]; i = ci; }
			v[i] = ent; return i; }
		int Lift(int i) {
			TEntry ent = v[i];
			while (i > 0) {
				int p = (i - 1) / 2;
				if (! cmp(ent.Key, v[p].Key)) break;
				v[i] = v[p]; i = p; }
			v[i] = ent; return i; }
		int Add(const TKey& key, const TDat& dat) { int i = v.Add(); v[i].Key = key; v[i].Dat = dat; return Lift(i); }
		bool Empty() const { return v.Empty(); }
		void DelRoot() { IAssert(! v.Empty()); v[0] = v.Last(); v.DelLast(); if (! v.Empty()) Sift(0); }
	};

	// Note: internally, all cartesian coordinates are maintained on the unit sphere.
	// The radius is just used to convert parameters and results during queries.
	TCoord radius; 
	static TCoord eps;  // used when comparing angles

	TNodeV nodes; // oct-tree
	int root; // index into 'nodes'
	int maxKeysPerLeaf;
	TKeyHash keys; 
	int firstUnusedNode; // Note: not saved by Save, for backwards binary compatibility.

	void Dump(FILE *f, const int nodeId, const TStr &prefix, int &nNodes, int &nKeys) const
	{
		const TNode &N = nodes[nodeId];
		nNodes++;
		fprintf(f, "%s- Node %d, nPoints = %d; [%.2f..%.2f] x [%.2f..%.2f] x [%.2f..%.2f]\n",
			prefix.CStr(), nodeId, N.nPoints, double(N.bbox.xMin), double(N.bbox.xMax),
			double(N.bbox.yMin), double(N.bbox.yMax),
			double(N.bbox.zMin), double(N.bbox.zMax));
		bool hasKeys = false, hasChildren = false;
		int nPoints = 0; int prevKeyId = -1;
		for (int keyId = N.firstPoint; keyId >= 0; ) {
			const TKeyEntry &K = keys[keyId];
			IAssert(K.nodeId == nodeId);
			hasKeys = true; nKeys++; nPoints++;
			fprintf(f, "%s  - KeyId %d, lat %.3f, lon %.3f, (%.3f, %.3f, %.3f)\n", prefix.CStr(),
				keyId, double(K.lat), double(K.lon), double(K.pt.x), double(K.pt.y), double(K.pt.z));
			IAssert(keys[keyId].prev == prevKeyId); prevKeyId = keyId;
			keyId = K.next; }
		IAssert(N.lastPoint == prevKeyId);
		for (int i = 0; i < 8; i++) {
			int childId = N.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
			if (childId < 0) continue;
			hasChildren = true;
			IAssert(nodes[childId].parent == nodeId);
			Dump(f, childId, prefix + ". ", nNodes, nKeys); }
		IAssert(! (hasChildren && hasKeys));
		IAssert(nPoints == N.nPoints);
	}

	// This is the recursive part of the ranged query algoritm.  See RangeQueryT for more.
	template<typename TSink>
	int RangeQuery_Recurse(const TPoint3& Q, const TCoord qChord2, int nodeId, TSink& sink) const
	{
		const TNode& node = nodes[nodeId];
		int result = 0;
		if (node.nPoints > 0) { // this is a leaf
			int keyId = node.firstPoint;
			while (keyId >= 0)
			{
				const TKeyEntry &ke = keys[keyId];
				TCoord dist2 = Q.Dist2(ke.pt);
				if (dist2 <= qChord2) {
					result++; 
					sink(keys.GetKey(keyId), ke.lat, ke.lon); }
				keyId = ke.next;
			}}
		else { // this is an internal node
			for (int i = 0; i < 8; i++)
			{
				int child = node.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
				if (child < 0) continue;
				if (nodes[child].bbox.MinDist2(Q) > qChord2) 
					continue; // the child's bounding box is disjoing with the query sphere
				result += RangeQuery_Recurse(Q, qChord2, child, sink);
			}}
		return result;

	}

	int AllocNode()
	{
		if (firstUnusedNode < 0) return nodes.Add();
		int nodeId = firstUnusedNode; firstUnusedNode = nodes[nodeId].NextUnusedNode();
		nodes[nodeId] = TNode(); return nodeId;
	}

	void FreeNode(const int nodeId)
	{
		nodes[nodeId].nPoints = -1;
		nodes[nodeId].lastPoint = firstUnusedNode;
		firstUnusedNode = nodeId;
	}

	// Splits the node 'nodeId', which is assumed to be a leaf.  New children are created below it
	// and the keys from 'nodeId' moved into them.  (Note that if the purpose of the split was to
	// split a former leaf that reached or exceeded maxKeysPerLeaf keys, there is no guarantee
	// that splitting will solve this problem -- it depends on where the keys are in space, perhaps
	// further splits on lower levels will be needed.  Orm if you have a lot of keys with the exactm
	// same coordinates, it might even be impossible to split them into sufficiently small leaves.)
	void SplitLeaf(int nodeId)
	{
		IAssert(nodes[nodeId].nPoints > 0);
		while (nodes[nodeId].firstPoint >= 0)
		{
			// Remove the first key from this node's point list.
			int keyId = nodes[nodeId].firstPoint; TKeyEntry &ke = keys[keyId];
			nodes[nodeId].firstPoint = keys[keyId].next;
			nodes[nodeId].nPoints -= 1;
			// Which child should the key move into?
			TNode *N = &(nodes[nodeId]), *C;
			TCoord mx = (N->bbox.xMin + N->bbox.xMax) / 2, my = (N->bbox.yMin + N->bbox.yMax) / 2, mz = (N->bbox.zMin + N->bbox.zMax) / 2;
			int ix = (ke.pt.x < mx ? 0 : 1), iy = (ke.pt.y < my ? 0 : 1), iz = (ke.pt.z < mz ? 0 : 1);
			int childId = N->children[ix][iy][iz];
			if (childId >= 0) C = &(nodes[childId]);
			else // This child does not yet exist, so we have to create it.
			{
				childId = AllocNode(); 
				N = 0; // not necessarily valid any more since AllocNode might have called nodes.Add(), which might have reallocated the array
				nodes[nodeId].children[ix][iy][iz] = childId;
				C = &(nodes[childId]); C->parent = nodeId;
				C->bbox = nodes[nodeId].bbox;
				if (ix == 0) C->bbox.xMax = mx; else C->bbox.xMin = mx;
				if (iy == 0) C->bbox.yMax = my; else C->bbox.yMin = my;
				if (iz == 0) C->bbox.zMax = mz; else C->bbox.zMin = mz;
				C->nPoints = 0; C->firstPoint = -1; C->lastPoint = -1;
			}
			// Move the key into the child.
			keys[keyId].nodeId = childId;
			keys[keyId].prev = C->lastPoint; keys[keyId].next = -1;
			if (C->lastPoint >= 0) { IAssert(keys[C->lastPoint].next < 0); keys[C->lastPoint].next = keyId; }
			C->lastPoint = keyId;
			if (C->firstPoint < 0) C->firstPoint = keyId;
			C->nPoints++;
		}
		IAssert(nodes[nodeId].nPoints == 0);
		IAssert(nodes[nodeId].firstPoint < 0);
		nodes[nodeId].lastPoint = -1;
	}

	// For reasons of backwards compatibility with old binary files, certain fields aren't saved by the Save
	// methods of this class and subsidiary structures.  Therefore, they need to be reconstructed after loading.
	// This method performs the following reconstructions:
	// - Unused nodes are connected into a linked list via 'firstUnusedNode' and 'TNode::lastPoint'.
	// - Keys belonging to the same node are connected to a *doubly* linked list whereas they have been saved
	//   only as a singly linked list.  This requires us to fix 'TNode::lastPoint' and 'TKeyEntry::prev'.
	// - The tree is augmented with pointers to parents whereas just pointers to children are saved.
	//   This requires us to fix 'TNode::parent'.
	void ReconstructAfterLoad()
	{
		firstUnusedNode = -1;
		for (int nodeId = 0; nodeId < nodes.Len(); ++nodeId) {
			TNode &N = nodes[nodeId];
			// Connect unused nodes into a singly linked list.
			if (N.IsUnused()) { N.lastPoint = firstUnusedNode; firstUnusedNode = nodeId; continue; }
			// Make each child node point to its parent.
			for (int i = 0; i < 8; i++) {
				int childId = N.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
				if (childId < 0) continue;
				IAssert(! nodes[childId].IsUnused());
				nodes[childId].parent = nodeId; }
			// Convert the singly linked list of keys for this node into a doubly linked one.
			int keyId = N.firstPoint; int prevKeyId = -1; 
			while (keyId >= 0) { 
				keys[keyId].prev = prevKeyId;
				prevKeyId = keyId;
				keyId = keys[keyId].next; }
			N.lastPoint = prevKeyId; }
	}

public:
	void Clr()
	{
		nodes.Clr(); keys.Clr(); 
		root = nodes.Add(); firstUnusedNode = -1;
		TNode& N = nodes[root]; N = TNode();
		N.bbox.xMin = -1.1; N.bbox.xMax = 1.1;
		N.bbox.yMin = -1.1; N.bbox.yMax = 1.1;
		N.bbox.zMin = -1.1; N.bbox.zMax = 1.1;
	}
	// http://en.wikipedia.org/wiki/Great-circle_distance#Radius_for_spherical_Earth
	static inline TCoord EarthRadiusKm() { return (TCoord) 6371.01; }
	TSphereNn(TCoord Radius = 1, int MaxKeysPerLeaf = 20) : radius(Radius), maxKeysPerLeaf(MaxKeysPerLeaf) { Clr(); }

	void Save(TSOut& SOut) const {
		nodes.Save(SOut); keys.Save(SOut);  SOut.Save(root); SOut.Save(maxKeysPerLeaf); SOut.Save(radius); SOut.SaveCs(); }
	void Load(TSIn& SIn) { 
		nodes.Load(SIn); keys.Load(SIn); SIn.Load(root); SIn.Load(maxKeysPerLeaf); SIn.Load(radius); SIn.LoadCs(); ReconstructAfterLoad(); }
	TSphereNn(TSIn& SIn) { Load(SIn); }

	// Converts from (latitude, longitude, radius) into (x, y, z).  The longitude should be in the range [-pi/2, pi/2].
	static void LatLonToCart(const TCoord lat_, const TCoord lon_, const TCoord radius, TCoord &x, TCoord &y, TCoord &z)
	{
		const TCoord lat = TDegToRad::ToRad(lat_), lon = TDegToRad::ToRad(lon_);
		x = radius * cos(lat) * cos(lon);
		y = radius * cos(lat) * sin(lon);
		z = radius * sin(lat);
	}
	// Converts from (latitude, longitude) to (x, y, z), using the radius from this instance's member field.
	void LatLonToCart(const TCoord lat, const TCoord lon, TCoord &x, TCoord &y, TCoord &z) const { LatLonToCart(lat, lon, radius, x, y, z); }
	// Returns the distance between two points on the sphere, along the great circle connecting them.
	static inline TCoord GetDist(const TCoord lat1, const TCoord lon1, const TCoord lat2, const TCoord lon2, const TCoord radius)
	{
		TCoord x1, x2, y1, y2, z1, z2;
		// Find points on the unit sphere for the given (lat, lon) pairs.
		LatLonToCart(lat1, lon1, 1, x1, y1, z1);
		LatLonToCart(lat2, lon2, 1, x2, y2, z2);
		// Since these two vectors are unit-length, their dot product is the cosine of the angle between them.
		const TCoord cosAngle = x1 * x2 + y1 * y2 + z1 * z2;
		// Let a be an angle and b = 2a; 
		// then      cos b = cos(a + a) = (cos a)^2 - (sin a)^2 = 2(cos a)^2 - 1,   hence (cos a)^2 = (1 + cos b) / 2.
		// Similarly cos b =              (cos a)^2 - (sin a)^2 = 1 - 2 (sin a)^ 2, hence (sin a)^2 = (1 - cos b) / 2.
		// Therefore (tan a)^2 = (sin a)^2 / (cos a)^2 
		//                     = (1 - cos b) / (1 + cos b) 
		//                     = (1 - cos b)(1 + cos b) / (1 + cos b)^2 
		//                     = (1 - (cos b)^2) / (1 + cos b)^2 
		//                     = (sin b)^2 / (1 + cos b)^2.  
		// Therefore tan a = +/- sin b / (1 + cos b).
		//   This formula results in a division by zero if b = pi + 2 k pi, but this also means that a = pi/2 + k pi,
		// therefore cos a = 0 and computing tan a directly also results in a division by zero, so the formula still makes sense 
		// from that point of view.  
		//   This leaves us with the question of the sign (after taking the square root);
		// the denominator, 1 + cos b, is never negative, and sin b = sin(a + a) = 2 sin a cos a clearly has the same
		// sign as sin a / cos a = tan a.  Therefore, we shouldn't change the sign after taking the square root.  
		//   This leaves us with the formula tan a = sin b / (1 + cos b).  
		//   Thus b = 2a = 2 arc tan (sin b / (1 + cos b)) = 2 arc tan (+/- sqrt(1 - (cos b)^2) / (1 + cos b)).
		//   Should we use + or - after taking the square root?   Actually we can't know.  The sign we take
		// there will end up determining the sign of the entire outcome, i.e. of b.  And we can't really know
		// whether b is positive or negative because we only know its cosine, and cos(b) = cos(-b).
		//   Fortunately for our purposes, i.e. computing the distance along the sphere, the sign of the angle
		// doesn't matter.  
		//   How do we handle the division by zero problem?  This happens when cos b = -1, which means that
		// b = 2 pi, i.e. our two points were on diametrally opposed points of the sphere.  
		if (cosAngle < -1 + eps) return radius * 2 * TMath::Pi;
		else if (cosAngle > 1 - eps) return 0;
		const TCoord sinAngle = sqrt(1 - (cosAngle * cosAngle));
		const TCoord angle = 2 * atan(sinAngle / (1 + cosAngle));
		// Since the angle is in radians at this point, we just have to multiply it by the radius to get the arc length.
		return radius * angle;
		// Incidentally, one might ask why we don't simply use a more direct formula: since we know cos b,
		// and we can take sin b = sqrt(1 - (cos b)^2), we could simply take b (?)= arc tan(sin b / cos b).
		// However, this formula has a few drawbacks.  
		// - First of all, it leads to a division by zero twice as often, namely for each b = pi/2 + k pi.  
		//   Note that these are otherwise completely reasonable and legitimate situations (when the two 
		//   points are 90 degrees apart on the sphere).  The previous formula works correctly in these cases.
		// - At b = pi (+ 2 k pi), this formula doesn't cause a division by zero (the previous formula does),
		//   but it returns a misleading result: cos b = -1, sin b = 0 and it therefore returns atan 0 = 0.
		//   But this is just a special case of the next drawback:
		// - If b is in the range (pi/2, pi), its cosine will be negative, and our new formula will
		//   therefore take the arc tan of something negative.  Now, the tangent is of course a periodic 
		//   function and its value is negative for angles in any of the ranges (pi/2 + k pi, k pi).
		//   Among these, of particular interest are (-pi/2, 0) and (pi/2, pi).  Since the implementation
		//   of arctan can't actually determine, given the tangent of an angle, which of these ranges
		//   the angle falls from, it simply has to choose one of these ranges more or less arbitrarily.
		//   The C/C++ function atan returns values from (-pi/2, 0), not from (pi/2, pi), so for our
		//   purposes it would really return b - pi rather than b.  So we'd have to handle these cases
		//   (when cos b is negative) separately and return arc tan (...) + pi.
		// We could work around this by returning atan2(cos b, sqrt(1 - (cos b)^2)), which would return
		// correct values for every b, even b = pi + 2 k pi etc.  However, this effectively means that
		// we're simply pushing all the special case handling onto atan2 instead of doing it by ourselves.
	}
	// Returns the distance between two points, using the radius from this instance's member field.
	TCoord GetDist(const TCoord lat1, const TCoord lon1, const TCoord lat2, const TCoord lon2) const {
		return GetDist(lat1, lon1, lat2, lon2, radius); }

	bool GetKeyCoords(const TKey& key, TCoord &lat, TCoord &lon) const {
		int keyId = keys.GetKeyId(key); if (keyId < 0) return false;
		lat = keys[keyId].lat; lon = keys[keyId].lon; return true; }
	bool IsKey(const TKey& key) const { return keys.IsKey(key); }
	bool Empty() const { return keys.Empty(); }
	int Len() const { return keys.Len(); }

	void DelKeyId(const int keyId) 
	{
		if (keyId < 0) return;
		// Remove the key from the doubly linked list of keys belonging to its leaf node.
		TKeyEntry& ke = keys[keyId];
		int nodeId = ke.nodeId, next = ke.next, prev = ke.prev;
		if (next >= 0) { IAssert(keys[next].prev == keyId); keys[next].prev = prev; }
		else { IAssert(nodes[nodeId].lastPoint == keyId); nodes[nodeId].lastPoint = prev; }
		if (prev >= 0) { IAssert(keys[prev].next == keyId); keys[prev].next = next; }
		else { IAssert(nodes[nodeId].firstPoint == keyId); nodes[nodeId].firstPoint = next; }
		// Remove the key from the 'keys' hash table.
		keys.DelKeyId(keyId);
		// If the node is now empty, delete it; it might similarly be necessary to delete some parent nodes.
		IAssert(nodes[nodeId].nPoints > 0); nodes[nodeId].nPoints--;
		if (nodes[nodeId].nPoints > 0) return; // The current node isn't empty yet.
		while (nodeId >= 0) 
		{
			// Node 'nodeId' is empty and needs to be deleted.
			// Note: We won't delete the root, even if it's empty.
			int parent = nodes[nodeId].parent;
			if (parent < 0) { IAssert(nodeId == root); break; } 
			// Remove the pointer to 'nodeId' from its parent node.
			TNode &P = nodes[parent]; 
			bool hasChildren = false, foundNode = false;
			for (int i = 0; i < 8; i++) {
				int &child = P.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
				if (child == nodeId) { IAssert(! foundNode); child = -1; foundNode = true; }
				else if (child >= 0) hasChildren = true; }
			IAssert(foundNode);
			// Delete 'nodeId' now.
			FreeNode(nodeId); 
			// If the parent is now empty, it will be deleted as well
			if (hasChildren) break;
			nodeId = parent;
		}
	}

	void DelKey(const TKey& key) { int keyId = keys.GetKeyId(key); IAssert(keyId >= 0); DelKeyId(keyId); }

	// Adds the key into the tree and keyhash, and returns the key ID.
	int AddKey(const TKey& key, const TCoord lat, const TCoord lon)
	{
		IAssert(! keys.IsKey(key));
		int keyId = keys.AddKey(key);
		TKeyEntry& ke = keys[keyId];
		ke.lat = lat; ke.lon = lon; ke.next = -1; ke.nodeId = -1; ke.prev = -1;
		LatLonToCart(lat, lon, 1, ke.pt.x, ke.pt.y, ke.pt.z);
		int nodeId = root; IAssert(nodeId >= 0);
		// Descend down the tree.
		while (nodes[nodeId].nPoints == 0)
		{
			TNode *N = &(nodes[nodeId]);
			// Which child do we have to descend into?
			TCoord mx = (N->bbox.xMin + N->bbox.xMax) / 2, my = (N->bbox.yMin + N->bbox.yMax) / 2, mz = (N->bbox.zMin + N->bbox.zMax) / 2;
			int ix = (ke.pt.x < mx ? 0 : 1), iy = (ke.pt.y < my ? 0 : 1), iz = (ke.pt.z < mz ? 0 : 1);
			int childId = N->children[ix][iy][iz];
			if (childId >= 0) { 
				nodeId = childId; 
				if (nodes[nodeId].nPoints >= maxKeysPerLeaf) 
					// This child node is a leaf, but it's already full, so we have to split it.
					SplitLeaf(nodeId);
				continue; }
			// A suitable child does not yet exist, so we have to create it.
			childId = AllocNode(); 
			N = &(nodes[nodeId]); // the old N might have been invalidated if AllocNode called nodes.Add() and reallocated the array
			N->children[ix][iy][iz] = childId; 
			TNode &C = nodes[childId];
			C.bbox = N->bbox; C.parent = nodeId;
			if (ix == 0) C.bbox.xMax = mx; else C.bbox.xMin = mx;
			if (iy == 0) C.bbox.yMax = my; else C.bbox.yMin = my;
			if (iz == 0) C.bbox.zMax = mz; else C.bbox.zMin = mz;
			C.nPoints = 0; C.firstPoint = -1; C.lastPoint = -1; nodeId = childId; break;
		}
		//
		{
			TNode &N = nodes[nodeId];
			ke.nodeId = nodeId; ke.next = N.firstPoint; 
			if (N.firstPoint >= 0) { IAssert(keys[N.firstPoint].prev < 0); keys[N.firstPoint].prev = keyId; }
			N.firstPoint = keyId;
			if (N.lastPoint < 0) N.lastPoint = keyId;
			N.nPoints++; // return nodeId;
			return keyId;
		}
	}

	// Returns the length of the chord corresponding to the given arc, on the unit sphere.
	static inline TCoord ArcToChord_UnitSphere(const TCoord arc)
	{
		// What angle does this arc have?  
		TCoord qAngle = arc; // on the unit sphere, angle (in radius) = arc length
		TCoord qChord;
		if (qAngle >= TMath::Pi - eps) 
			// This query will cover the entire sphere.
			qChord = 3; // for good measure, though 2 should be enough
		else // chord / 2 = radius * sin(angle / 2).
			qChord = 2 * sin(qAngle * 0.5);
		return qChord;
	}

	// Enumerates all the points within distance 'qDist' (as measured along the great circles on the sphere)
	// from the given query point (qLat, qLon).  For each key within this query area, the call 'sink(key, keyLat, keyLon)' is made.
	template<typename TSink> 
	int RangeQueryT(const TCoord qLat, const TCoord qLon, const TCoord qDist, TSink& sink) const
	{
		TCoord qDist1 = qDist / radius; // remember that internally we have everything on the unit sphere
		// The query distance is measured along the arc of a great circle.  
		// This is equivalent to looking for all points whose Euclidean distance
		// from Q is <= than the chord of this arc.
		TCoord qChord = ArcToChord_UnitSphere(qDist1);
		// Also convert the query point into cartesian coordinates.
		TPoint3 Q; LatLonToCart(qLat, qLon, 1.0, Q.x, Q.y, Q.z);
		int result = 0;
		if (root >= 0 && nodes[root].nPoints >= 0) 
			result = RangeQuery_Recurse(Q, qChord * qChord, root, sink);
		return result;
	}

	// Adds, to 'dest', all the keys within distance 'qDist' of the query point (qLat, qLon).
	int RangeQuery(const TCoord qLat, const TCoord qLon, const TCoord qDist, TKeyV& dest, const bool clrDest = true) const {
		if (clrDest) dest.Clr();
		TKeyVecSink sink(dest); return RangeQueryT(qLat, qLon, qDist, sink); }


	int NnQuery(const TCoord qLat, const TCoord qLon, const int count, TKeyV& dest, const bool clrDest = true) const
	{
		return NnQuery(qLat, qLon, count, (TCoord) -1, dest, clrDest);
	}

	// Neighbors with distance greater than 'qDist' from the query point are ignored, even if this means
	// that less than 'count' neighbors will be returned.  'qDist' is measured as an arc along the surface
	// of the sphere, same as it would be in a range query.  If qDist < 0, then the nearest 'count' neighbors
	// are returned no matter how far from the query point they are.
	int NnQuery(const TCoord qLat, const TCoord qLon, const int count, const TCoord qDist, TKeyV& dest, const bool clrDest = true) const
	{
		/*
			Pseudocode of a k-nearest-neighbor query:
			- Let Q be our query point.
			  MinDist(boundingBox, Q) is the distance from Q to the nearest point on or in the bounding box.
			- Maintain a heap Results with the k nearest neighbours known so far, sorted so that the
			  most distant among them is in the root of the heap.  Initially this heap is empty.
		    - Maintain a heap called ToDo of nodes to be processed, sorted by MinDist(node.boundingBox, Q).
			- Add the root to ToDo, unless it's empty.
			- While ToDo is not empty:
			  - Remove the node N from the root of ToDo.
			  - If Results contains >= k points and MinDist(N.boundingBox, Q) > dist(Results.root, Q) then break;
			  - If it's a leaf:
			    - For each point X in N:
				  - If Results contains < k points, add X to Results
				  - Else If dist(Q, X) < dist(Results.root, Q) then 
				      put X into Results.root and sink it;
			  - Else, if N is an internal node:
			    - For each nonempty child M of N:
				  - If MinDist(M.boundingBox, Q) <= dist(Results.root, Q) or Results contains < k nodes
				    then insert M into ToDo<
		*/
		TCoord qChord2; if (qDist < 0) qChord2 = -1; 
		else { TCoord qDist1 = qDist / radius; qChord2 = ArcToChord_UnitSphere(qDist1); qChord2 *= qChord2; }
		//
		TPoint3 Q; LatLonToCart(qLat, qLon, 1.0, Q.x, Q.y, Q.z);
		THeap<TCoord, int, TGreater<TCoord> > results; // the ints are key IDs; the TCoord is the distance from the query point
		THeap<TCoord, int, TLess<TCoord> > toDo; // the ints are node IDs; the TCoord is the MinDist^2 from the query point to the node
		if (root >= 0) toDo.Add(nodes[root].bbox.MinDist2(Q), root);
		FILE *fLog = 0; // stderr; 
		if (fLog) fprintf(fLog, "NnQuery: lat = %g, lon = %g; x = %g, y = %g, z = %g)\n", double(qLat), double(qLon), double(Q.x), double(Q.y), double(Q.z));
		while (! toDo.Empty())
		{
			int nodeId = toDo.v[0].Dat; TCoord nMinDist2 = toDo.v[0].Key;
			if (fLog) fprintf(fLog, "Dequeued node %d, MinDist^2 = %g, nPoints = %d\n", nodeId, double(nMinDist2), nodes[nodeId].nPoints);
			if (results.Len() >= count && nMinDist2 > results.v[0].Key) 
				break; // nothing we can find in this node or the remaining ones in 'toDo' can improve the results any further
			toDo.DelRoot();
			const TNode &N = nodes[nodeId];
			if (N.nPoints > 0) { // leaf node; examine its points and possibly add them into 'results'
				for (int keyId = N.firstPoint; keyId >= 0; keyId = keys[keyId].next) 
				{
					const TKeyEntry &ke = keys[keyId]; Assert(ke.nodeId == nodeId);
					TCoord dist = ke.pt.Dist2(Q); 
					if (fLog) fprintf(fLog, " - Point with keyId %d, lat %.3f, lon %.3f, dist^2 = %.6g; results.Len() = %d, count = %d, results.v[0].Key = %g\n",
						keyId, double(ke.lat), double(ke.lon), double(dist), results.Len(), count, (results.Empty() ? -666 : double(results.v[0].Key)));
					if (qChord2 >= 0 && dist > qChord2) continue; 
					if (results.Len() < count) 
						// We don't even know 'count' neighbors yet, so we can simply add the current point into the results set.
						results.Add(dist, keyId); 
					else if (dist < results.v[0].Key) {
						// We already know 'count' results but the current point is better than at least one of them, 
						// so we can replace the most distant neighbor in 'results' with our current point.
						if (fLog) fprintf(fLog, " - This point will replace old point %d in the results heap.\n", int(results.v[0].Dat));
						results.v[0].Key = dist; results.v[0].Dat = keyId;
						results.Sift(0); }
				}}
			else { // internal node; examine its children and add them into 'toDo'
				for (int i = 0; i < 8; i++) {
					int childId = N.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
					if (childId < 0) continue;
					TCoord childMinDist2 = nodes[childId].bbox.MinDist2(Q);
					if (fLog) fprintf(fLog, " - Child node [%d] = %d, MinDist^2 = %g, results.Len() = %d, count = %d, results.v[0].Ley = %g\n",
						i, childId, double(childMinDist2), results.Len(), count, (results.Empty() ? -666 : double(results.v[0].Key)));
					if (qChord2 >= 0 && childMinDist2 > qChord2) continue; // hopeless subtree
					if (results.Len() >= count && childMinDist2 > results.v[0].Key) 
						continue; // this child is hopeless
					toDo.Add(childMinDist2, childId); }}
		}
		// If we extract the results from 'results' one by one, we'll get them in decreasing order of distance from Q.
		TIntV resultV; while (! results.Empty()) { resultV.Add(results.v[0].Dat); results.DelRoot(); }
		if (clrDest) dest.Clr();
		// So we'll insert them into 'dest' in reverse order, i.e. in increasing order of distance from Q.
		for (int i = resultV.Len() - 1; i >= 0; i--) dest.Add(keys.GetKey(resultV[i]));
		return resultV.Len();
	}

	void Dump(FILE *f = 0)
	{
		int nUsedNodes = 0, nKeys = 0;
		if (! f) f = stdout;
		fprintf(f, "TSphereNn::Dump\n");
		Dump(f, root, "", nUsedNodes, nKeys);
		int nUnusedNodes = 0; for (int nodeId = firstUnusedNode; nodeId >= 0; nodeId = nodes[nodeId].NextUnusedNode()) nUnusedNodes++;
		int nAllNodes = nodes.Len(); 
		fprintf(f, "TSphereNn::Dump ends: %d nodes (%d used + %d unused), %d = %d keys\n", nAllNodes, nUsedNodes, nUnusedNodes, nKeys, keys.Len());
		IAssert(nAllNodes == nUsedNodes + nUnusedNodes);
		IAssert(nKeys == keys.Len());
	}

	void TestStructure() const
	{
		int nFreeNodes1 = 0, nKeysSeen = 0;
		TBoolV seen; seen.Gen(nodes.Len()); seen.PutAll(false);
		for (int nodeId = 0; nodeId < nodes.Len(); nodeId++)
		{
			const TNode& N = nodes[nodeId];
			if (N.nPoints < 0) { nFreeNodes1++; continue; }
			if (N.parent < 0) { IAssert(nodeId == root); seen[nodeId] = true; }
			if (N.nPoints > 0) 
			{
				// Must be a leaf node; make sure it has no children.
				for (int i = 0; i < 8; i++) IAssert(N.children[(i >> 2) & 1][(i >> 1) & 1][i & 1] < 0);
				// Examine its child list.
				int nPoints = 0, prevKeyId = -1;
				int keyId = N.firstPoint; while (keyId >= 0) {
					nPoints++; nKeysSeen++; 
					const TKeyEntry &ke = keys[keyId];
					IAssert(ke.nodeId == nodeId);
					IAssert(ke.prev == prevKeyId);
					prevKeyId = keyId; keyId = ke.next; }
				IAssert(N.lastPoint == prevKeyId);
				IAssert(N.nPoints == nPoints); 
				IAssert(nPoints <= maxKeysPerLeaf);
			}
			else 
			{
				// Must be an internal node; make sure it has no points.
				IAssert(N.nPoints == 0);
				IAssert(N.firstPoint < 0); IAssert(N.lastPoint < 0);
				// Make sure all the children point to this parent.
				int nChildren = 0;
				for (int i = 0; i < 8; i++) { 
					int childNodeId = N.children[(i >> 2) & 1][(i >> 1) & 1][i & 1];
					if (childNodeId < 0) continue;
					nChildren++; IAssert(! seen[childNodeId]);
					seen[childNodeId] = true; IAssert(nodes[childNodeId].parent == nodeId); }
				if (nChildren <= 0) IAssert(nodeId == root);
			}
		}
		IAssert(nKeysSeen == keys.Len());
		int nFreeNodes2 = 0;
		for (int nodeId = firstUnusedNode; nodeId >= 0; nodeId = nodes[nodeId].lastPoint) {
			IAssert(nodes[nodeId].nPoints < 0);
			nFreeNodes2++; }
		IAssert(nFreeNodes1 == nFreeNodes2);
	}

};

// An angle of 10^{-7} radians on a sphere the size of the earth corresponds to an arc of a bit less than 1 m.
template <typename TKey_, typename TCoord_, bool useDegrees>
TCoord_ TSphereNn<TKey_, TCoord_, useDegrees>::eps = (TCoord_) 1e-7;



#endif // ____SPHERENN_H_INCLUDED____