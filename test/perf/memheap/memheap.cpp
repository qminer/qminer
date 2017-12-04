/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <base.h>

static TRnd Rnd = TRnd(1);

int UniRnd(const int& MaxVecSize) {
    return Rnd.GetUniDevInt(MaxVecSize);
}

int Pow2Rnd(const int& MaxVecSize) {
    const int Pow = Rnd.GetUniDevInt(TMath::FloorLog2((uint64)MaxVecSize));
    return Rnd.GetUniDevInt(TMath::Pow2(Pow), TInt::GetMn(TMath::Pow2(Pow + 1), MaxVecSize));
}

template <typename TRndFun>
void RandAlloc(TVec<TIntV>& IntVV, TRndFun& RndFun, const int& MaxVecSize, const bool& FillP) {
    IntVV.Shuffle(Rnd);
    for (TIntV& IntV : IntVV) {
        // create new vector
        const TSize NewSize = RndFun(MaxVecSize);
        IntV.Gen(NewSize);
        // fill it with random stuff
        if (FillP) {
            for (int IntN = 0; IntN < IntV.Len(); IntN++) {
                Rnd.GetUniDevInt(TInt::Mx);
            }
        }
    }
}

template <typename TRndFun>
void Pow2Alloc(TVec<TIntV>& IntVV, TRndFun& RndFun, const int& MaxVecSize, const bool& FillP) {
    IntVV.Shuffle(Rnd);
    for (TIntV& IntV : IntVV) {
        // create new vector
        const TSize NewSize = RndFun(MaxVecSize);
        const TSize Pow2Size = TMath::IsPow2(NewSize) ? NewSize :
            TMath::Pow2<TSize>(TMath::FloorLog2((uint64)NewSize) + 1);
        IntV.Gen(Pow2Size, NewSize);
        if (FillP) {
            for (int IntN = 0; IntN < IntV.Len(); IntN++) {
                Rnd.GetUniDevInt(TInt::Mx);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // create environment
    Env=TEnv(argc, argv, TNotify::StdNotify);

    // get command line parameters
    Env.PrepArgs("Heap fragmentation tests", 0);
    // database
    TStr Type = Env.GetIfArgPrefixStr("-type=", "random", "random or pow2");
    TStr Size = Env.GetIfArgPrefixStr("-size=", "uni", "uni or pow2");
    const int Vecs = Env.GetIfArgPrefixInt("-vecs=", 1000000, "Number of vectors on the heap");
    const int MaxVecSize = Env.GetIfArgPrefixInt("-max_size=", 10, "Power of 2 of max vector size");
    printf("==============================\n");

    auto RndFun = (Size == "uni") ? UniRnd : Pow2Rnd;

    // prepare placeholder for allocations
    TVec<TIntV> IntVV(Vecs);
    // measure
    while (true) {
        TTmStopWatch StopWatch(true);
        if (Type == "random") {
            RandAlloc(IntVV, RndFun, TMath::Pow2(MaxVecSize), false);
        } else {
            Pow2Alloc(IntVV, RndFun, TMath::Pow2(MaxVecSize), false);
        }

        printf("%.2f msecs\n", StopWatch.GetMSec());
    }

    return 0;
}
