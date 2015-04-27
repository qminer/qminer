/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
