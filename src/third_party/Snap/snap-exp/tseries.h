namespace TSnap {
/// Info, article..
/// 
TFltV GetTsFeatures(std::string line, TStrV features, int N);
void EgonetPropEvolutionBatch(TStr InFNm, TIntFltVH &nn, TIntFltVH &ne, TIntFltVH &edi, TIntFltVH &adi, TIntFltVH &dc, TIntFltVH &bc, TIntFltVH &cc, TIntFltVH &de, TIntFltVH &wcc, TIntV &ids, TIntStrH &idmap, TIntIntVH &years, int &minyear,int &maxyear);
}