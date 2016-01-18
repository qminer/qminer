/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// SphereNn.cpp : Defines the entry point for the console application.
//

#include "base.h"
#include "sphere.h"
#include <limits>

double DmsToDeg(double deg, double min, double sec)
{
	return deg + (min + sec / 60.0) / 60.0;
}

//-----------------------------------------------------------------------------
// TGenericTimer
//-----------------------------------------------------------------------------
// Sample usage:
//   THiPrecTimer tm1, tm2;
//   tm1.Start(); Foo(); tm1.Stop();
//   tm2.Start(); Bar(); tm2.Stop();
//   tm1.Start(); Baz(); tm1.Stop();
//   printf("Foo + Baz ran for %.2f s, Bar ran for %.2f s.\n", tm1.Sec(), tm2.Sec());
// Other interesting methods: Ms (milliseconds), Mus (microseconds).
// Note: THiPrecTimer measures wall-clock time, THiPrecProcessTimer measures CPU time.

template<typename TTraits_>
class TGenericTimer
{
public:

	TCRef CRef;

protected:

	typedef TTraits_ TTraits;
	typedef typename TTraits::TUnit TUnit;

	TUnit liStart, liTime;
	int running;

public:

	TGenericTimer() : running(0) { liTime = TTraits::Zero(); }

	~TGenericTimer()
	{
		if (running != 0)
		{
			char buf[200]; sprintf(buf, "%s::running is %d.\n", typeid(*this).name(), running);
			WarnNotify(buf);
		}
	}

	TGenericTimer(TSIn& SIn)
	{
		SIn.Load(running);
		liTime = TTraits::Load(SIn);
		if (running > 0) liStart = TTraits::Now();
	}

	void Save(TSOut& SOut) const
	{
		TUnit li = liTime;
		if (running > 0) li = TTraits::Add(TTraits::Now(), liStart);
		SOut.Save(running); TTraits::Save(SOut, li);
	}

	static TPt<TGenericTimer> Load(TSIn& SIn) { return new TGenericTimer(SIn); }

protected:

	void Start(const TUnit& liNow)
		{ if (running++ == 0) liStart = liNow; }
	void Stop(const TUnit& liNow)
	{
		Assert(running > 0);
		if (--running == 0)
			liTime = TTraits::Add(liTime, TTraits::Sub(liNow, liStart));
	}

public:

	void Clr()
	{
		liTime.QuadPart = 0;
		if (running > 0) liStart = TTraits::Now();
	}

	void Start()
		{ if (running++ == 0) liStart = TTraits::Now(); }

	TGenericTimer& Stop()
	{
		Assert(running > 0);
		if (--running == 0)
		{
			TUnit liStop = TTraits::Now();
			liTime = TTraits::Add(liTime, TTraits::Sub(liStop, liStart));
		}
		return *this;
	}

	// Stops the first timer and starts the second one, while making
	// only one call to QueryPerformanceCounter.
	static void StopAndStart(TGenericTimer& tmToStop, TGenericTimer& tmToStart)
	{
		TUnit liNow = TTraits::Now();
		tmToStop.Stop(liNow);
		tmToStart.Start(liNow);
	}

	TUnit Value() const
	{
		TUnit li = liTime;
		if (running > 0)
			li = TTraits::Add(li, TTraits::Sub(TTraits::Now(), liStart));
		return li;
	}


	double Sec() const { return TTraits::ToSec(Value()); }
	double Ms() const { return 1000.0 * Sec(); }
	double Mus() const { return 1000000.0 * Sec(); }

	template<typename TOther>
	TGenericTimer& operator += (const TOther& x) { liTime = TTraits::Add(liTime, x); }

};

//-----------------------------------------------------------------------------
// TGenericTimer traits
//-----------------------------------------------------------------------------

#ifdef GLib_WIN

class TTimerTraits_LargeInteger_Common
{
public:
	typedef LARGE_INTEGER TUnit;
	inline static TUnit Zero() { TUnit u; u.QuadPart = 0; return u; }
	inline static void Save(TSOut& SOut, const TUnit& u) { SOut.SaveBf(&u.QuadPart, sizeof(u.QuadPart)); }
	inline static TUnit Load(TSIn& SIn) { TUnit u; SIn.LoadBf(&u.QuadPart, sizeof(u.QuadPart)); }
	inline static TUnit Add(const TUnit &x, const TUnit &y) { TUnit u; u.QuadPart = x.QuadPart + y.QuadPart; return u; }
	inline static TUnit Sub(const TUnit &x, const TUnit &y) { TUnit u; u.QuadPart = x.QuadPart - y.QuadPart; return u; }
};


class TTimerTraits_Wall : public TTimerTraits_LargeInteger_Common
{
public:
	inline static TUnit Now() { TUnit u; QueryPerformanceCounter(&u); return u; }
	inline static double ToSec(const TUnit& u) {
		TUnit liFreq; QueryPerformanceFrequency(&liFreq);
		return double(u.QuadPart) / double(liFreq.QuadPart); }
};

class TTimerTraits_Cpu : public TTimerTraits_LargeInteger_Common
{
public:
	inline static TUnit Now() {
		FILETIME ftc, fte, ftk, ftu; GetProcessTimes(GetCurrentProcess(), &ftc, &fte, &ftk, &ftu);
		TUnit li; li.LowPart = ftu.dwLowDateTime; li.HighPart = ftu.dwHighDateTime; return li; }
	inline static double ToSec(const TUnit& u) {
		return double(u.QuadPart) / 10000000.0; }
	inline static TUnit Add(const TUnit &x, const TUnit &y) { TUnit u; u.QuadPart = x.QuadPart + y.QuadPart; return u; }
	inline static TUnit Add(const TUnit &x, const FILETIME &y) { TUnit u, yy;
		yy.LowPart = y.dwLowDateTime; yy.HighPart = y.dwHighDateTime;
		u.QuadPart = x.QuadPart + yy.QuadPart; return u; }
};

#elif defined(GLib_UNIX)

class TTimerTraits_Clock_Common
{
public:
	typedef intmax_t TUnit; // should be signed because clock_t is also signed, to avoid problems with subtraction in case of wraparound (note that clock_t is likely to be 32-bit)
	inline static TUnit Zero() { return 0; }
	inline static void Save(TSOut& SOut, const TUnit& u) { SOut.SaveBf(&u, sizeof(u)); }
	inline static TUnit Load(TSIn& SIn) { TUnit u; SIn.LoadBf(&u, sizeof(u)); }
	inline static TUnit Add(const TUnit &x, const TUnit &y) { return x + y; }
	inline static TUnit Sub(const TUnit &x, const TUnit &y) { return x - y; }
	inline static double ToSec(const TUnit& u) {
		clock_t freq = CLOCKS_PER_SEC;
		return double(u) / double(freq); }
};


class TTimerTraits_Wall : public TTimerTraits_Clock_Common
{
public:
	inline static TUnit Now() {
		struct tms t; clock_t wall = times(&t);
		return (TUnit) wall; }
};

class TTimerTraits_Cpu : public TTimerTraits_Clock_Common
{
public:
	inline static TUnit Now() {
		struct tms t; clock_t wall = times(&t);
		return (TUnit) t.tms_utime; }
};

#else

#error Unsupported platform!

#endif

//-----------------------------------------------------------------------------
// TGenericTimer typedefs
//-----------------------------------------------------------------------------

typedef TGenericTimer<TTimerTraits_Wall> THiPrecTimer;
typedef TPt<THiPrecTimer> PHiPrecTimer;

typedef TGenericTimer<TTimerTraits_Cpu> THiPrecProcessTimer;
typedef TPt<THiPrecProcessTimer> PHiPrecProcessTimer;

//-----------------------------------------------------------------------------
// TStDev
//-----------------------------------------------------------------------------
// Let X_1, ..., X_n be independent, identically distributed
// variables with E[X_i] = \mu and D[X_i] = \sigma^2.
//
// Let Xa := 1/n \sum_{i=1}^n X_i.
// Then E[Xa] = 1/n \sum_{i=1}^n E[X_i] = 1/n \sum_{i=1}^n \mu = \mu.
//
// Let S^2 := 1/n \sum_{i=1}^n (X_i - Xa)^2
//          = 1/n \sum_{i=1}^n (X_i^2 - 2 X_i Xa + Xa^2)
//          = 1/n \sum_{i=1}^n X_i^2 - 2 Xa/n \sum_{i=1}^n X_i + Xa^2
//          = 1/n \sum_{i=1}^n X_i^2 - 1/n^2 \sum_{i=1}^n \sum_{j=1}^n X_i X_j.
// Then E[S^2] = 1/n \sum_{i=1}^n E[X_i^2] - 1/n^2 \sum_{i=1}^n \sum_{j=1}^n E[X_i X_j] = (*)
// and for i \not= j, X_i and X_j are independent and thus E[X_i X_j] = E[X_i]^2,
//         (*) = 1/n \sum_{i=1}^n E[X_i^2] - 1/n^2 \sum_{i=1}^n E[X_i^2] - 1/n^2 \sum_{i=1}^n \sum_{j=1}^n [j \not= i] E[X_i]^2
//             = E[X_1^2]                  - 1/n E[X_1^2]                - n(n-1)/n^2 E[X_1]^2
//             = (n-1)/n E[X_1^2] - (n-1)/n E[X_1]^2
//             = (n-1)/n D[X_1]
//             = (n-1)/n \sigma^2.
// Thus S^2 is a biased estimator of \sigma^2.  The value
//     (S')^2 := n/(n-1) S^2
//             = 1/(n-1) \sum_{i=1}^n (X_i - Xa)^2
// is an unbiased estimator, however.
//
// D[Xa] = E[Xa^2] - E[Xa]^2
//       = E[1/n^2 \sum_{i=1}^n \sum_{j=1}^n X_i X_j] - \mu^2
//       = 1/n^2 \sum_{i=1}^n \sum_{j=1}^n E[X_i X_j] - \mu^2
//       = 1/n^2 \sum_{i=1}^n E[X_i^2] + 1/n^2 \sum_{i=1}^n \sum_{j=1}^n [i \not= j] E[X_i X_j] - \mu^2
//       = 1/n E[X_1^2] + n(n-1)/n^2 E[X_1]^2 - n^2/n^2 E[X_1]^2
//       = 1/n E[X_1^2] - 1/n E[X_1]^2
//       = 1/n D[X_1]
//       = 1/n \sigma^2.
// Thus, if (S')^2 is an unbiased estimator of \sigma^2, the value
//         1/n (S')^2
//       = 1/(n(n-1)) \sum_{i=1}^n (X_i - Xa)^2
// is an unbiased estimator of D[Xa].

// Note: TStDev originally supported only integer counts.
// Now it's templatized and thus also supports non-integer "counts" (really weights),
// but this should not be used together with the keepVals flag, as the results
// (i.e. medians) will be incorrect (they will be equivalent to rounding the counts 
// down to the nearest integer).  To make this work properly, 'vals' should contain
// (value, weight) pairs, but I can't be bothered making such a big change for now.
// So for the time being, the class checks that TCount is an integer type if the
// keepVals flag is set.

#define STDEV_DEFINED

//class TStDev;
template<typename TCount_> class TStDev_;

typedef TStDev_<int> TStDev;
typedef TStDev_<double> TStDevF;
typedef TVec<TStDev> TStDevV;

template<typename TCount_>
class TStDev_
{
public:
	typedef typename TCount_ TCount;
	TCount n; double sum, sum2; // the number of values, their sum, and the sum of their squares
	double sumXLnX; // the sum of x * ln(x) over all the values x.
	double minX, maxX; // the smallest and the largest value
	mutable double med, med2; // the median (or median2) saved by the last call of Med and Med2
	mutable TFltV vals; // values are stored here, but only if the keepVals flag is set
	// The keepVals flag should be set if you want to be able to compute the medians.
	// medSaved and med2Saved tell if the medians stored in med and med2 are still valid
	// (i.e. if no new values have been added since those two medians were computed).
	// Med and Med2() use this to avoid the call to vals.Sort() if the values have not changed.
	enum { keepVals = 1, medSaved = 2, med2Saved = 4 };
	mutable int flags;
	explicit TStDev_(const bool KeepVals = false) { Clr(KeepVals); }
	TStDev_(TCount N, double Sum, double Sum2, double SumXLnX, double MinX, double MaxX, bool KeepVals = false)
		: n(N), sum(Sum), sum2(Sum2), sumXLnX(SumXLnX), minX(MinX), maxX(MaxX), flags(KeepVals ? keepVals : 0) { }
	TStDev_(TCount N, double Sum, double Sum2, double SumXLnX, double MinX, double MaxX, bool KeepVals, const TFltV& vals1, const TFltV& vals2)
		: n(N), sum(Sum), sum2(Sum2), sumXLnX(SumXLnX), minX(MinX), maxX(MaxX), flags(KeepVals ? keepVals : 0)
		{ if (KeepVals) { vals = vals1; vals.AddV(vals2); } }
	void Clr() { n = 0; sum = 0.0; sum2 = 0.0; sumXLnX = 0.0; vals.Clr(); med = 0.0; med2 = 0.0; flags = 0; minX = 0.0; maxX = 0.0; }
	void Clr(const bool KeepVals) { 
		if (KeepVals) IAssert(std::numeric_limits<TCount>::is_integer); 
		Clr(); if (KeepVals) flags |= keepVals; }
	void Add(double d, TCount count) { sum += d * count; sum2 += d * d * count;
		if (d <= -1e-8) sumXLnX = std::numeric_limits<double>::quiet_NaN(); else if (d < 1e-8) sumXLnX += 0.0; else sumXLnX += count * d * log(d);
		if (n == 0 || d < minX) minX = d; if (n == 0 || d > maxX) maxX = d;
		n += count; flags &= ~(medSaved | med2Saved); if (flags & keepVals) for (int i = 0; i < count; i++) vals.Add(d); }
	void Add(double d) { sum += d; sum2 += d * d; n++;
		if (d <= -1e-8) sumXLnX = std::numeric_limits<double>::quiet_NaN(); else if (d < 1e-8) sumXLnX += 0.0; else sumXLnX += d * log(d);
		if (n == 1 || d < minX) minX = d; if (n == 1 || d > maxX) maxX = d;
		flags &= ~(medSaved | med2Saved); if (flags & keepVals) vals.Add(d); }
	TStDev& operator << (double d) { Add(d); return *this; }
	TStDev& operator += (double d) { Add(d); return *this; }
	void Add(const TStDev& s) {
		if (s.n > 0) { if (n == 0 || s.minX < minX) minX = s.minX; if (n == 0 || s.maxX > maxX) maxX = s.maxX; }
		sum += s.sum; sum2 += s.sum2; sumXLnX += s.sumXLnX; n += s.n; flags &= ~(medSaved | med2Saved);
		if (flags & keepVals) { IAssert(s.flags & keepVals); vals.AddV(s.vals); }}
	TStDev& operator << (const TStDev& s) { Add(s); return *this; }
	TStDev& operator += (const TStDev& s) { Add(s); return *this; }

	int Count() const { return n; }
	// Avg() returns the value of Xa from the above discussion.
	// This is an unbiased estimator of \mu.
	double Avg() const { if (n <= 0) return 0.0; else return sum / double(n); }
	// Var() returns the value of S^2 from the above discussion.
	double Var() const { if (n <= 1) return 0.0;
		double avg = sum / double(n); double s2 = sum2 / double(n) - avg * avg;
		return (s2 < 1e-8) ? 0.0 : s2; }
	// Std() returns the value of S from the above discussion.
	// This is a biased estimator of \sigma.
	double Std() const { if (n <= 1) return 0.0;
		double avg = sum / double(n); double s2 = sum2 / double(n) - avg * avg;
		return (s2 < 1e-8) ? 0.0 : sqrt(s2); }
	// Ste() returns the value of sqrt( 1/n (S')^2 ) from the above
	// discussion.  This is an unbiased estimator of D[Xa].
	// In other words, this tells us how far the value of Xa is from \mu, on average.
	double Ste() const { if (n <= 1) return 0.0;
		double avg = sum / double(n); double s2 = sum2 / double(n) - avg * avg;
		return (s2 < 1e-8) ? 0.0 : sqrt(s2 / double(n - 1)); }
	// The median of the values obtained so far.
	// If there is an even number of values, X_1, ..., X_{2k}, the value X_{k+1} is returned.
	double Med() const { IAssert(flags & keepVals);
		if (flags & medSaved) return med;
		flags |= medSaved; QSort(vals);
		med = vals.Empty() ? 0.0 : double(vals[vals.Len() / 2]); return med; }
	// The median; if there is an even number of values, X_1, ..., X_{2k},
	// the value (X_k + X_{k+1})/2 is returned.
	double Med2() const { IAssert(flags & keepVals);
		if (flags & med2Saved) return med2;
		QSort(vals); flags |= med2Saved;
		if (vals.Empty()) med2 = 0.0;
		else if (vals.Len() & 1) med2 = vals[vals.Len() / 2];
		else med2 = 0.5 * (vals[(vals.Len() - 1) / 2] + vals[vals.Len() / 2]);
		return med2; }
	// The maximum and minimum.
	double Max() const { return maxX; }
	double Min() const { return minX; }
	// The entropy.  Let s = sum_i x_i and let p_i = x_i / s.
	// We are interested in H = - sum_i p_i ln p_i = - (1/s) sum_i x_i ln (x_i / s)
	// = - (1/s) sum_i x_i ln x_i + (1/s) sum_i x_i ln s =
	// = - (1/s) sumXLnX          + (1/s) s         ln s
	double Entropy() const {
		if (n <= 0 || sum < 1e-8) return 0.0;
		return - sumXLnX / sum + log(sum); }
	// If X is distributed according to Beta(a, b), 
	// it turns out that E[X] = a / (a + b) and D[X] = ab / [(a + b)^2 (a + b + 1)].  
	// Thus we can estimate a and b with 
	// a = Avg (Avg * (1 - Avg) / Var - 1) and b = (1 - Avg)(Avg (1 - Avg) / Var - 1).
	double Alpha() const { double avg = Avg(), var = Var(); return avg * (avg * (1 - avg) / var - 1); }
	double Beta() const { double avg = Avg(), var = Var(); return (1 - avg) * (avg * (1 - avg) / var - 1); }
	void AlphaBeta(double &alpha, double &beta) const {
		double avg = Avg(), var = Var(); 
		double temp = avg * (1 - avg) / var - 1; 
		alpha = avg * temp; beta = (1 - avg) * temp; }
};

template<typename TCount>
inline TStDev_<TCount> operator + (const TStDev_<TCount>& s1, const TStDev_<TCount>& s2) {
	return TStDev_(s1.n + s2.n, s1.sum + s2.sum, s1.sum2 + s2.sum2, s1.sumXLnX + s2.sumXLnX,
		(s1.n == 0 ? s2.minX : ((s2.n == 0 || s1.minX < s2.minX)? s1.minX : s2.minX)),
		(s1.n == 0 ? s2.maxX : ((s2.n == 0 || s1.maxX > s2.maxX)? s1.maxX : s2.maxX)),
		(s1.flags | s2.flags) & TStDev::keepVals, s1.vals, s2.vals); }

//-----------------------------------------------------------------------------


// A small test of TSphereNn::GetDist.  The distance reported should be a little over 100 km.
void Test1()
{
	typedef TSphereNn<TStr, double, true> TMySphereNn;
	TMySphereNn sphereNn(TMySphereNn::EarthRadiusKm());
	double latLj = DmsToDeg(46, 3, 20), lonLj = DmsToDeg(14, 30, 30); 
	double latMb = DmsToDeg(46, 33, 0), lonMb = DmsToDeg(15, 39, 0); 
	int keyLj = sphereNn.AddKey("Ljubljana", latLj, lonLj);
	int keyMb = sphereNn.AddKey("Maribor", latMb, lonMb);
	double dist1 = sphereNn.GetDist(latLj, lonLj, latMb, lonMb);
	double dist2 = sphereNn.GetDist(latMb, lonMb, latLj, lonLj);
	printf("Distance from %s to %s is %.3f = %.3f km.\n", 
		sphereNn.keys.GetKey(keyLj).CStr(),
		sphereNn.keys.GetKey(keyMb).CStr(),
		dist1, dist2);
}

// This class contains naive implementations of nearest-neighbor queries and ranged queries
// and can compare its results with those of TSphereNn.
template <typename TKey_>
class TNaiveSphereNn
{
public:
	typedef TKey_ TKey;
	typedef TVec<TKey> TKeyV;
	typedef TSphereNn<TKey, double, true> TSphere;
	TSphere sphereNn;
	TVec<TKeyDat<TKey, TFltPr> > points;
	THiPrecProcessTimer tmNnQuery, tmRangeQuery;

	TNaiveSphereNn() : sphereNn(TSphere::EarthRadiusKm()) { }

	// Returns a random subset of 'howMany' keys from those currently in 'points'; they are given in random order.
	void GetRandomSubsetOfKeys(TRnd& rnd, int howMany, TKeyV& keys)
	{
		int n = points.Len();
		IAssert(howMany <= n);
		TIntV v; v.Gen(n); for (int i = 0; i < n; i++) v[i] = i;
		v.Shuffle(rnd); keys.Clr(); keys.Gen(howMany);
		for (int i = 0; i < howMany; i++) keys[i] = points[v[i]].Key;
	}

	// Deletes the given key from 'sphereNn' and also from our local list of points.
	void DelKey(const TKey& key)
	{
		sphereNn.DelKey(key);
		int found = -1;
		for (int i = 0; i < points.Len(); i++) if (points[i].Key == key) { IAssert(found < 0); found = i; }
		IAssert(found >= 0); points.Del(found);
	}

	// Adds the given key to 'sphereNn' and also to our local list of points, 'points'.
	int AddKey(const TKey& key, const double lat, const double lon)
	{
		int retVal1 = sphereNn.AddKey(key, lat, lon);
		int retVal2 = points.Add(); 
		points[retVal2].Key = key; points[retVal2].Dat.Val1 = lat; points[retVal2].Dat.Val2 = lon;
		//IAssert(retVal1 == retVal2); // Note: this assertion would be justified if no deletions have taken place, but in general it isn't.
		return retVal1;
	}
	// Performs a range query in O(n) time by iterating through all the points in 'points'
	// and usint GetDist to see if they are close enough to the query point.
	int RangeQuery(const double qLat, const double qLon, const double qDist, TKeyV& dest) const 
	{
		dest.Clr();
		for (int i = 0; i < points.Len(); i++)
		{
			double dist = sphereNn.GetDist(qLat, qLon, points[i].Dat.Val1, points[i].Dat.Val2, sphereNn.radius);
			if (dist <= qDist)
				dest.Add(points[i].Key);
		}
		return dest.Len();
	}
	// Tests if sphereNn's results of the given query match those of our naive implementation.
	void TestRangeQuery(const double qLat, const double qLon, const double qDist) 
	{
		TKeyV results1, results2; 
		tmRangeQuery.Start();
		sphereNn.RangeQuery(qLat, qLon, qDist, results1);
		tmRangeQuery.Stop();
		this->RangeQuery(qLat, qLon, qDist, results2);
		IAssert(results2.Len() == results1.Len());
		results1.Sort(); results2.Sort();
		for (int i = 0; i < results1.Len(); i++) 
			IAssert(results1[i] == results2[i]);
	}
	// Performs a nearest-neighbor query in O(n log n) time by calculating the distance of every point
	// from the query point, sorting them, and reporting the first few of them.
	int NnQuery(const double qLat, const double qLon, const int count, const double qDist, TKeyV& dest) const 
	{
		typedef TKeyDat<TFlt, TKey> TKd;
		TVec<TKd> v;
		for (int i = 0; i < points.Len(); i++)
		{
			double dist = sphereNn.GetDist(qLat, qLon, points[i].Dat.Val1, points[i].Dat.Val2, sphereNn.radius);
			if (qDist >= 0 && dist > qDist) continue;
			v.Add(TKd(dist, points[i].Key));
		}
		v.Sort();
		dest.Clr();
		for (int i = 0; i < v.Len() && i < count; i++)
			dest.Add(v[i].Dat);
		return dest.Len();
	}
	// Tests if sphereNn's results of the given query match those of our naive implementation.
	// It returns the distance of the query point from the most distant among the 'count' nearest neighbors.
	double TestNnQuery(const double qLat, const double qLon, const int count, const double qDist) 
	{
		TKeyV results1, results2; 
		this->NnQuery(qLat, qLon, count, qDist, results2);
		tmNnQuery.Start();
		sphereNn.NnQuery(qLat, qLon, count, qDist, results1);
		tmNnQuery.Stop();
		// In principle, both lists should be sorted in increasing order of distance from Q = (qLat, qLon).
		// But if several points are at equal distance Q, their relative order is undefined.
		// If the k'th nearest neighbor is just one of a big group of equally distant neighbors,
		// it could even happen that each of the two lists contains some points that aren't present in the other list.
		IAssert(results1.Len() == results2.Len());
		double prevDist = -1;
		double eps = TSphere::eps * sphereNn.EarthRadiusKm();
		for (int i = 0; i < results1.Len(); i++)
		{
			const TKey& key1 = results1[i], key2 = results2[i];
			double lat, lon; bool ok = sphereNn.GetKeyCoords(key1, lat, lon); 
			IAssert(ok);
			double dist1 = sphereNn.GetDist(qLat, qLon, lat, lon);
			ok = sphereNn.GetKeyCoords(key2, lat, lon);
			IAssert(ok);
			double dist2 = sphereNn.GetDist(qLat, qLon, lat, lon);
			IAssert(fabs(dist1 - dist2) < eps);
			IAssert(prevDist < dist1 + eps);
			prevDist = dist1;
		}
		return prevDist;
	}

};

// Creates a test set of 'nPoints' random points and performs 'nQueries' queries of
// each type, comparing the results of TSphereNn and the naive implementation.
// An assertion failure occurs in case of any mismatch.
void Test2(TRnd rnd, int nPoints, int nQueries, int nCycles)
{
	TNaiveSphereNn<TInt> sphereNn;
	// As an extra precaution, the keys won't be integers 0..nPoints - 1, but slightly larger
	// integers, with gaps between them and in a random order.
	TIntV keys; keys.Add(rnd.GetUniDevInt(10));
	for (int i = 1; i < nPoints * nCycles; i++) keys.Add(keys.Last() + rnd.GetUniDevInt(10) + 1);
	keys.Shuffle(rnd);
	//keys.Clr(); for (int i = 0; i < nPoints * nCycles; i++) keys.Add(i);
	int iNextNewKey = 0;
	for (int iCycle = 0; iCycle < nCycles; iCycle++)
	{
		printf("Cycle %d/%d staring.\n", iCycle, nCycles);
		// Generate 'nPoints' random points.  They will be distributed uniformly 
		// in the (lat, lon) space, which in practice means they will tend to be concentrated
		// around the Earth's poles.
		int nPointsToAdd = (iCycle == 0) ? nPoints : rnd.GetUniDevInt((nPoints * 2 + 2) / 3, nPoints);
		printf("Adding %d points...\n", nPointsToAdd);
		for (int i = 0; i < nPointsToAdd; i++)
		{
			double lat = rnd.GetUniDev() * 180 - 90;
			double lon = rnd.GetUniDev() * 360 - 180;
			sphereNn.AddKey(keys[iNextNewKey++], lat, lon);
			if (false) sphereNn.sphereNn.Dump();
		}
		if (false) sphereNn.sphereNn.Dump();
		sphereNn.sphereNn.TestStructure();
		printf("Number of points is now %d.\n", sphereNn.points.Len()); IAssert(sphereNn.points.Len() == sphereNn.sphereNn.Len());
		if (false) 
		{PSOut SOut = TMOut::New();
		sphereNn.sphereNn.Save(*SOut);
		TMOut *mOut = (TMOut *) SOut(); printf("Saved to a stream, %d bytes\n", mOut->Len());
		PSIn SIn = mOut->GetSIn();
		sphereNn.sphereNn.Clr();
		sphereNn.sphereNn.Load(*SIn);}
		// Run the queries.
		printf("Performing %d queries of each type...\n", nQueries);
		for (int i = 0; i < nQueries; i++)
		{
			double lat = rnd.GetUniDev() * 180 - 90;
			double lon = rnd.GetUniDev() * 360 - 180;
			int nNeigh = rnd.GetUniDevInt(10) + 10;
			//int nNeigh = rnd.GetUniDevInt(3) + 3;
			double neighDist = sphereNn.TestNnQuery(lat, lon, nNeigh, -1);
			double qDist = neighDist * (0.5 + 1.0 * rnd.GetUniDev());
			sphereNn.TestNnQuery(lat, lon, nNeigh, qDist);
			// For a ranged query, we have to choose a reasonable distance that will produce
			// a moderate number of results, i.e. not so small as to have 0 results and not so big
			// as to have an unreasonable number of them.  Since we have just learned that
			// there are up to 'nNeigh' points within 'neighDist' distance of (lat, lon),
			// we can use 'neighDist' (or a small multiple of it) as the query distance in
			// the ranged query around the same query point.
			qDist = neighDist * (1 + rnd.GetUniDev());
			sphereNn.TestRangeQuery(lat, lon, qDist);
		}
		// Delete a few keys.  At the end of the last cycle, we'll delete all points.
		int nPointsToDel = (iCycle == nCycles - 1) ? sphereNn.points.Len() : rnd.GetUniDevInt((nPoints + 2) / 3, (nPoints + 1) / 2);
		printf("Deleting %d points...\n", nPointsToDel);
		TIntV keysToDel; sphereNn.GetRandomSubsetOfKeys(rnd, nPointsToDel, keysToDel);
		for (int i = 0; i < nPointsToDel; i++) 
		{
			sphereNn.DelKey(keysToDel[i]);
			if (false) sphereNn.sphereNn.Dump();
		}
		sphereNn.sphereNn.TestStructure();
		printf("Number of points is now %d.\n", sphereNn.points.Len()); IAssert(sphereNn.points.Len() == sphereNn.sphereNn.Len());
	}
	printf("Time spent: %.3f s for NN queries, %.3f s for range queries\n",
		sphereNn.tmNnQuery.Sec(), sphereNn.tmRangeQuery.Sec());
}

// This function is very similar to Test2, but without the naive implementation.
// Thus it can be used to test the speed on large sets of points, but without 
// testing the correctness of results.
void Test3(TRnd rnd, int nPoints, int nQueries)
{
	typedef TSphereNn<TInt, double, true> TMySphereNn;
	THiPrecProcessTimer tmAdd, tmNnQuery, tmNnRangeQuery, tmRangeQuery;
	PSIn SIn;
	TMySphereNn sphereNn(TMySphereNn::EarthRadiusKm());
	// 
	TIntV keys; keys.Add(rnd.GetUniDevInt(10));
	for (int i = 1; i < nPoints; i++) keys.Add(keys.Last() + rnd.GetUniDevInt(10) + 1);
	keys.Shuffle(rnd);
	keys.Clr(); for (int i = 0; i < nPoints; i++) keys.Add(i);
	// Generate 'nPoints' random points.  They will be distributed uniformly 
	// in the (lat, lon) space, which in practice means they will tend to be concentrated
	// around the Earth's poles.
	printf("Adding %d points...\n", nPoints);
	for (int i = 0; i < nPoints; i++)
	{
		double lat = rnd.GetUniDev() * 180 - 90;
		double lon = rnd.GetUniDev() * 360 - 180;
		tmAdd.Start();
		sphereNn.AddKey(keys[i], lat, lon);
		tmAdd.Stop();
	}
	if (false) sphereNn.Dump();
	// Run the queries.
	printf("Performing %d queries of each type...\n", nQueries);
	TStDev avgResultsNn, avgResultsRange, avgResultsNnRange;
	for (int i = 0; i < nQueries; i++)
	{
		if (i % 100 == 0) printf("%d    \r", i);
		double lat = rnd.GetUniDev() * 180 - 90;
		double lon = rnd.GetUniDev() * 360 - 180;
		int nNeigh = rnd.GetUniDevInt(10) + 10;
		//int nNeigh = rnd.GetUniDevInt(3) + 3;
		TIntV results1, results2, results3;
		tmNnQuery.Start();
		sphereNn.NnQuery(lat, lon, nNeigh, results1);
		tmNnQuery.Stop();
		IAssert(! results1.Empty());
		double lat2, lon2; sphereNn.GetKeyCoords(results1.Last(), lat2, lon2);
		double neighDist = sphereNn.GetDist(lat, lon, lat2, lon2);
		//
		double qDist = neighDist * (0.3 + 0.4 * rnd.GetUniDev());
		tmNnRangeQuery.Start();
		sphereNn.NnQuery(lat, lon, nNeigh, qDist, results3);
		tmNnRangeQuery.Stop();
		//IAssert(! results3.Empty()); // Actually, there's no guarantee that there will be some neighbors in this distance.
		// For a ranged query, we have to choose a reasonable distance that will produce
		// a moderate number of results, i.e. not so small as to have 0 results and not so big
		// as to have an unreasonable number of them.  Since we have just learned that
		// there are up to 'nNeigh' points within 'neighDist' distance of (lat, lon),
		// we can use 'neighDist' (or a small multiple of it) as the query distance in
		// the ranged query around the same query point.
		qDist = neighDist * (1 + rnd.GetUniDev());
		tmRangeQuery.Start();
		sphereNn.RangeQuery(lat, lon, qDist, results2);
		tmRangeQuery.Stop();
		avgResultsNn.Add(results1.Len()); avgResultsRange.Add(results2.Len());
		avgResultsNnRange.Add(results3.Len());
	}
	printf("Time spent: %.3f s to add %d points,\n"
		"  %.6f us per query for NN queries (avg %.2f +/- %.2f results),\n"
		"  %.6f us per query for NN-range queries (avg %.2f +/- %.2f results),\n"
		"  %.6f us per query for range queries (avg %.2f +/- %.2f results),\n"
		"  (%d queries of each type)\n",
		tmAdd.Sec(), nPoints, 
		tmNnQuery.Mus() / double(nQueries), avgResultsNn.Avg(), avgResultsNn.Std(),
		tmNnRangeQuery.Mus() / double(nQueries), avgResultsNnRange.Avg(), avgResultsNnRange.Std(),
		tmRangeQuery.Mus() / double(nQueries), avgResultsRange.Avg(), avgResultsRange.Std(),
		nQueries);
}

// Testing of AddKey and DelKeyId in various combinations.
void Test4(TRnd rnd, int nPoints)
{
	typedef TSphereNn<TInt, double, true> TMySphereNn;
	for (int pass = 0; pass < 100; pass++)
	{
		TMySphereNn sphereNn(TMySphereNn::EarthRadiusKm());
		sphereNn.TestStructure();
		// 
		TIntV keys; keys.Add(rnd.GetUniDevInt(10));
		for (int i = 1; i < nPoints; i++) keys.Add(keys.Last() + rnd.GetUniDevInt(10) + 1);
		keys.Shuffle(rnd);
		keys.Clr(); for (int i = 0; i < nPoints; i++) keys.Add(i);
		TIntV keyIds;
		for (int i = 0; i < nPoints; i++)
		{
			double lat = rnd.GetUniDev() * 180 - 90;
			double lon = rnd.GetUniDev() * 360 - 180;
			if (nPoints == 73 && i == 69 && pass == 57)
				printf("!\n");
			int keyId = sphereNn.AddKey(keys[i], lat, lon);
			keyIds.Add(keyId);
			sphereNn.TestStructure();
		}
		TIntV delOrder;
		bool ids = ((pass % 2) == 0); int Pass = pass / 2;
		if (ids) delOrder = keyIds; else delOrder = keys;
		if (Pass == 0) { }
		else if (Pass == 1) { delOrder = keys; delOrder.Reverse(); }
		else if (Pass == 2) { delOrder = keys; delOrder.Sort(); }
		else if (Pass == 3) { delOrder = keys; delOrder.Sort(); delOrder.Reverse(); }
		else { delOrder = keys; delOrder.Shuffle(rnd); }
		for (int i = 0; i < delOrder.Len(); i++) 
		{
			if (ids) sphereNn.DelKeyId(delOrder[i]);
			else sphereNn.DelKey(delOrder[i]); 
			sphereNn.TestStructure();
		}
	}
}

int main(int argc, char ** argv)
{
	TStr s = "Hello world!";
	printf("%s\n", s.CStr());
	Test1();
	//
	if (true)
	{
		TRnd rnd(123);
		int nPoints = rnd.GetUniDevInt(10) + 10;
		int nQueries = 100, nCycles = 10;
		while (nPoints <= 1000) {
			Test2(rnd, nPoints, nQueries, nCycles);
			nPoints += rnd.GetUniDevInt(10) + 10; }
	}
	//
	if (false)
	{
		TRnd rnd(123);
		int nPoints = 1000000; if (argc > 1) nPoints = TStr(argv[1]).GetInt();
		int nQueries = 10000; if (argc > 2) nQueries = TStr(argv[2]).GetInt();
		Test3(rnd, nPoints, nQueries);
	}
	//
	if (true)
	{
		TRnd rnd(123);
		for (int nPoints = 1; nPoints <= 100; nPoints++)
			Test4(rnd, nPoints);
	}
	/*
	Test2(TRnd(147), 584, 10);
	Test2(TRnd(400), 70, 1);
	Test2(TRnd(500), 70, 1);
	TRnd rnd(123); //for (int nPoints = 10; nPoints += 1; nPoints <= 200) Test2(rnd, nPoints, 1000);
	for (int i = 0; ; i++) {
		printf("%d ", i);
		Test2(TRnd(i), 584, 10); }
	//Test2(rnd, 234, 1000);
	*/
	return 0;
}

