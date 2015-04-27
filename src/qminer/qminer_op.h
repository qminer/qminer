/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef QMINER_OP_H
#define QMINER_OP_H

#include "qminer_core.h"
#include "qminer_ftr.h"
#include "qminer_aggr.h"

namespace TQm {

///////////////////////////////
// QMiner-Operator-Search
class TOpSearch : public TOp {
public:
	TOpSearch(): TOp("Search") { }
	static POp New() { return new TOpSearch; }
	
	// inherited interface
	void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interfaces
	PRecSet Exec(const TWPt<TBase>& Base, const PQuery& Query);
};

////////////////////////////////////////////////
// QMiner-Operator-Linear-Search
//  TODO: IsInRange shold be cleand to work on time fileds and accept TimeOfDay
typedef enum { oolstLess, oolstEqual, oolstNotEqual, oolstGreater, oolstIsInRange, oolstIsIn, oolstIsNotIn } TOpLinSearchType;
class TOpLinSearch : public TOp {
private:
	void ParseQuery(const TWPt<TBase>& Base, const uint& StoreId, const PJsonVal& QueryElt, 
		int& FieldId, TOpLinSearchType& LinSearchType, TStr& FieldVal);
public:
	TOpLinSearch(): TOp("SearchLin") { }
	static POp New() { return new TOpLinSearch(); }
	
	// inherited interface
	void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface, over full store
	PRecSet Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const int& FieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const TTm& FieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TTm& MnFieldVal, const TTm& MxFieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const uint64& MnFieldVal, const uint64& MxFieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const TIntV& FieldVals);		
	// actual interface, over given record set
	PRecSet Exec(const TWPt<TBase>& Base, const PRecSet& RecSet, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const TTm& FieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const PRecSet& RecSet, const int& FieldId, 
		const TTm& MnFieldVal, const TTm& MxFieldVal);
	PRecSet Exec(const TWPt<TBase>& Base, const PRecSet& RecSet, 
		const int& FieldId, const bool& FieldVal);
};

////////////////////////////////////
// QMiner-Operator-GroupBy
class TOpGroupBy: public TOp {
public:
	TOpGroupBy(): TOp("GroupBy") { }
	static POp New() { return new TOpGroupBy; }
	
	// inherited interface
	void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, const uint& StoreId, 
		const int& FieldId, TRecSetV& OutRecSetV);
};
////////////////////////////////////
// QMiner-Operator-SplitBy
class TOpSplitBy: public TOp {
public:
	TOpSplitBy(): TOp("SplitBy") { }
	static POp New() { return new TOpSplitBy; }
	
	// inherited interface
	void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, const uint& StoreId, 
		const int& FieldId, const int& SplitWinSize, TRecSetV& OutRecSetV);
};

////////////////////////////////////
// QMiner-Operator-Agglomerative-Clustering
class TOpAggClust: public TOp {
private:
	TRecSetV ClusterRecSetV;
	
private: 
	void GetRecIds(const PRecSet& InRecSet, const TVec<TIntV>& RecNV, TRecSetV& OutRecSetV);

public:
	TOpAggClust(): TOp("AggClust") { }
	static POp New() { return new TOpAggClust; }
	
	// inherited interface
	void Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, const uint& StoreId, 
		const int& FieldId, const int& ClusterN, TRecSetV& OutRecSetV);
	void Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, const uint& StoreId, 
		const int& FieldId, const double& Sim, TRecSetV& OutRecSetV);
	void Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, const uint& StoreId, 
		const int& FieldId, TRecSetV& OutRecSetV);
};

}  // namespace

#endif
