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

/////////////////////////////////////////////////
// BagOfWords-Active-Learning
typedef enum {baltCat, baltQuery} TBowALType;

ClassTP(TBowAL, PBowAL)//{
private:
    TBowALType ALType;
    // general properties
    PBowDocBs BowDocBs;
    TFlt SvmC, SvmJ;
    TIntV LabeledDIdV, UnlabeledDIdV; // labeled/unlabeled pointers to DocSet!!!
    PBowDocWgtBs BowDocWgtBs;
    PSVMTrainSet DocSet;
    TFltIntPrV DistDIdPrV;
    // Cat properties
    TInt CId;
    // Query properties
    TStr QueryStr;
    PBowSpV QuerySpV;
    TBool InfoRetModeP;
    TInt MnPosDocs;
    TInt MnNegDocs;
    // stats
    TStr KeyWdStr;
    TInt PosDocN;
    TInt NegDocN;
public:
    // active learning based on documents labeled with categories
    TBowAL(const PBowDocBs& _BowDocBs, const int& _CId,
        const double& _SvmC, const double& _SvmJ, const TIntV& _DIdV);
    static PBowAL NewFromCat(const PBowDocBs& BowDocBs, const int& CId,
        const double& SvmC, const double& SvmJ, const TIntV& DIdV = TIntV()) {
            return PBowAL(new TBowAL(BowDocBs, CId, SvmC, SvmJ, DIdV));}
    // active learning based on query -- must also find initial
    // set of positive and negative documents
    TBowAL(const PBowDocBs& _BowDocBs, const TStr& _QueryStr,
        const int& _MnPosDocs, const int& _MnNegDocs, const double& _SvmC,
        const double& _SvmJ, const TIntV& _DIdV);
    static PBowAL NewFromQuery(const PBowDocBs& BowDocBs, const TStr& QueryStr,
        const int& MnPosDocs, const int& MnNegDocs, const double& SvmC,
        const double& SvmJ, const TIntV& DIdV = TIntV()) {
            return PBowAL(new TBowAL(BowDocBs, QueryStr,
                MnPosDocs, MnNegDocs, SvmC, SvmJ, DIdV));
    }

    TBowAL(TSIn& SIn){Fail;}
    static PBowAL Load(TSIn& SIn){return new TBowAL(SIn);}
    void Save(TSOut& SOut){Fail;}

    int GetCId() const {return CId;}
    // prepares a list of queries to ask the user
    bool GenQueryDIdV(const bool& GenStatP = false);
    // number of queries
    int GetQueryDIds() const { return DistDIdPrV.Len(); }
    // get the document id and it's distance of the N-th query
    void GetQueryDistDId(const int& QueryDIdN, double& Dist, int& DId){
        Dist=DistDIdPrV[QueryDIdN].Val1; DId=DistDIdPrV[QueryDIdN].Val2;}
    // adds the query feedback back into the datasets
    void MarkQueryDoc(const int& QueryDId, const bool& AssignToCatP);
    // marks the remaining of the unlabeled documents in the BOW using SVM
    void MarkUnlabeledPosDocs();
    // returns a list of DocIds of all positive documents
    // (both labeled and positively classified unlabeled)
    void GetAllPosDocs(TIntV& PosDIdV);

    TBowALType GetType() const { return TBowALType(int(ALType)); }
    TStr GetQueryStr() const { return QueryStr; }
    bool GetInfoRetMode() const { return InfoRetModeP; }

    TStr GetKeyWdStr() const { return KeyWdStr; }
    int GetPosDocN() const { return PosDocN; }
    int GetNegDocN() const { return NegDocN; }
};

