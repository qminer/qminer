/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef tensor_h
#define tensor_h

#include "base.h"

#ifdef GLib_OPENMP
  #include <omp.h>
#endif


//TDTensor dense tensor
//TSTensor sparse tensor
//TKTensor Kruskal tensor
//TTTensor Tucker tensor
namespace TTensor
{

template <class TVal = TFlt, class TSizeMdTy = TInt, class TSizeNzTy = int>
class TTensorOp;    
    
// high number of nonzero elements: TSizeNzTy = int64, high dimensionality of modes: TSizeMdTy = int64
template <class TVal = TFlt, class TSizeMdTy = TInt, class TSizeNzTy = int>
class TSTensor {
private:
	TInt Modes; // number of modes
	TVec<TSizeMdTy> DimV; // dimensions of each mode	
	TVec<TVal, TSizeNzTy> Values; //values of nonzero elements
	TVVec<TSizeMdTy, TSizeNzTy> Coordinates; //coordinates of nonzero elements (X coordinate ranges over data points)	
public:
	// Sets dimensions
	TSTensor(const TVec<TSizeMdTy>& DimV_) {
		Modes = DimV_.Len();
		DimV = DimV_;
	}
	// Sets dimensions and reserves space
	TSTensor(const TVec<TSizeMdTy>& DimV_, const TSizeNzTy& NNonZero) {
		Modes = DimV_.Len();
		DimV = DimV_;
		Values.Gen(NNonZero);
		Coordinates.Gen(NNonZero, Modes);
	}
	// The input is assumed to come from saveSparse.m or similar (not robust)
	// Assumes double for values, int for dimensions and int or int64 for nnz
	TSTensor(const TStr& FileNm, const bool& BigIndex = false) {
		TFIn Reader(FileNm);
		TStr Line;
		Reader.GetNextLn(Line);		
		TStrV StrVec;
		Line.SplitOnAllAnyCh(" ", StrVec, true);
		DimV.Gen(StrVec.Len());
		Modes = DimV.Len();
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			DimV[ModeN] = StrVec[ModeN].GetInt();
		}
		Reader.GetNextLn(Line);		
		if (!BigIndex) {
			int NNZ = Line.GetInt();
			Values.Gen(NNZ);
			Coordinates.Gen(NNZ, Modes);
		} else {
			int64 NNZ = Line.GetInt64();
			Values.Gen(NNZ);
			Coordinates.Gen(NNZ, Modes);
		}
		int64 LineN = 0;
		while (Reader.GetNextLn(Line)) {
			Line.SplitOnAllAnyCh(" ", StrVec, true);
			if (StrVec.Len() == 0) continue;
			Assert(StrVec.Len() == Modes + 1);
			Assert(LineN < Values.Len());
			for (int ModeN = 0; ModeN < Modes; ModeN++) {
				Coordinates.PutXY(LineN, ModeN, StrVec[ModeN].GetInt());
			}
			Values[LineN] = StrVec[Modes].GetFlt();
			LineN++;
		}
	}
	void GenRandom(const TVec<TSizeMdTy>& DimV_, const TSizeNzTy& NNZ) {
		Modes = DimV_.Len();
		DimV = DimV_;
		TRnd Rand;		

		TVec<TVal, TSizeNzTy> Values2; //values of nonzero elements
		Values2.Gen(NNZ);
		TVVec<TSizeMdTy, TSizeNzTy> Coordinates2;
		Coordinates2.Gen(NNZ, Modes);

			
		if (NNZ > TInt::Mx) {
			IAssertR(false, "Int64 NNZ not implemented!");	
		}
		
		THashSet<TStr> Keys; // for duplicates
		Keys.Gen(NNZ); // for duplicates

		TSizeNzTy Offset = 0;
		for (TSizeNzTy ElN = 0; ElN < NNZ; ElN++) {
			Values2[ElN - Offset] = TFlt::GetRnd();
			TStr Coordinate;
			for (int ModeN = 0; ModeN < Modes; ModeN++) {
				if (DimV[ModeN] < TInt::Mx) {
					Coordinates2.At(ElN - Offset, ModeN) = Rand.GetUniDevInt(DimV[ModeN]);
				} else {
					Coordinates2.At(ElN - Offset, ModeN) = Rand.GetUniDevInt64(DimV[ModeN]);
				}
				Coordinate += TInt::GetStr(Coordinates2.At(ElN - Offset, ModeN)) + "_"; // duplicates
			}
			// duplicates: repeat step or add key
			if (Keys.IsKey(Coordinate)) {
				Offset++;
			} else {
				Keys.AddKey(Coordinate);
			}			
			//printf("%s, offset %d\n", Coordinate.CStr(), Offset);
		}		

		Values.Gen(NNZ - Offset); //values of nonzero elements
		Coordinates.Gen(NNZ - Offset, Modes);
		for (TSizeNzTy ElN = 0; ElN < NNZ - Offset; ElN++) {
			Values[ElN] = Values2[ElN];			
			for (int ModeN = 0; ModeN < Modes; ModeN++) {
				Coordinates.At(ElN, ModeN) = Coordinates2.At(ElN, ModeN);
			}
		}
		

		
	}

	void Display() {
		printf("Number of modes %d\n", Modes);
		printf("Dimensions: ");
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			printf("%I64d ", DimV[ModeN]);
		}		
		printf("\n");
		printf("NNZ: %I64d\n", Values.Len());
		for (int64 LineN = 0; LineN < Values.Len(); LineN++) {
			for (int ModeN = 0; ModeN < Modes; ModeN++) {
				printf("%I64d ", Coordinates.At(LineN, ModeN));
			}
			printf("%f\n", Values[LineN]);
		}
	}
	
	int GetModes() const {return Modes;}	
	const TVec<TSizeMdTy>& GetDimV() const {return DimV;}
	TSizeMdTy GetDim(const int& DimN) const {Assert((DimN >= 0) && (DimN < Modes) && (DimV.Len() == Modes)); return DimV[DimN];}
	TSizeNzTy GetNNZ() const {return Values.Len();}
	const TVVec<TSizeMdTy, TSizeNzTy>& GetCoordinates() const {return Coordinates;}
	const TVec<TVal, TSizeNzTy>& GetValues() const {return Values;}
	
	TVal GetNorm() const {
		TVal norm = 0.0;
		for (TSizeNzTy ElN = 0; ElN < Values.Len(); ElN++){
			norm += Values[ElN] * Values[ElN];
		}
		return sqrt(norm);
	}

	bool IsConsistent() const {
		if (DimV.Len() != Modes) return false;
		if (DimV.Len() != Coordinates.GetYDim()) return false;
		if (Values.Len() != Coordinates.GetXDim()) return false;
		//read coordinates and check for bounds
		for (TSizeNzTy RowN = 0; RowN < Coordinates.GetXDim(); RowN++) {
			for (TSizeNzTy ColN = 0; ColN < Coordinates.GetYDim(); ColN++) {
				if ((Coordinates.At(RowN,ColN) < 0) || (Coordinates.At(RowN,ColN) >= DimV[(int)ColN])) return false;				
			}
		}
		return true;
	}
};

// high dimensionality of modes: TSizeMdTy = int64
template <class TVal = TFlt, class TSizeMdTy = TInt, class TSizeNzTy = int>
class TKTensor {
private:
	TInt Modes; // number of modes	
	TVec<TSizeMdTy> DimV; // dimensions of each mode (number of modesis an int!)
	TInt R; // number of components
	TVec<TVal> Lambda; //coefficient vector: number of components is an int! length = R	
	TVec<TVVec<TVal, TSizeMdTy> > U; //basis matrices, columns should be normalized (x = rowIdx, y = colIdx), ydim = R
public:
	TKTensor(const TVec<TSizeMdTy>& DimV_, const int& R_) {
		Modes = DimV_.Len();
		R = R_;
		Lambda.Gen(R); //Lambda.PutAll(0.0);
		DimV = DimV_;
		U.Gen(Modes, 0);
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			// Create DimV[ModeN] x R matrix
			TVVec<TVal, TSizeMdTy> Factor; 
			Factor.Gen(DimV[ModeN], R);
			U.Add(Factor);
		}
	}
	TKTensor(const TStr& FileNm, const bool& BigIndex = false) {		
		TFIn Reader(FileNm);
		TStr Line;
		// dimensions
		Reader.GetNextLn(Line);		
		TStrV StrVec;
		Line.SplitOnAllAnyCh(" ", StrVec, true);
		DimV.Gen(StrVec.Len());
		Modes = DimV.Len();
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			if (!BigIndex) {
				DimV[ModeN] = StrVec[ModeN].GetInt();
			} else {
				DimV[ModeN] = StrVec[ModeN].GetInt64();
			}
		}		
		// lambda
		Reader.GetNextLn(Line);	
		Line.SplitOnAllAnyCh(" ", StrVec, true);
		R = StrVec.Len();
		Lambda.Gen(R);
		for (int FacN = 0; FacN < R; FacN++) {
			Lambda[FacN] = StrVec[FacN].GetFlt();
		}
		printf("startt U\n");
		// U matrices
		U.Gen(Modes, 0);
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			TVVec<TVal, TSizeMdTy> Factor; 
			Factor.Gen(DimV[ModeN], R);
			for (TSizeMdTy RowN = 0; RowN < DimV[ModeN]; RowN++) {
				Reader.GetNextLn(Line);	
				Line.SplitOnAllAnyCh(" ", StrVec, true);
				for (int FacN = 0; FacN < R; FacN++) {
					Factor.PutXY(RowN, FacN, StrVec[FacN].GetFlt());
				}
			}
			U.Add(Factor);
		}

	}
	void GenRandom(const TVec<TSizeMdTy>& DimV_, const int& R_) {
		Modes = DimV_.Len();
		R = R_;
		Lambda.Gen(R);		
		TRnd Rand;

		for (int FacN = 0; FacN < R; FacN++) {
			Lambda[FacN] = Rand.GetNrmDev();
		}
		DimV = DimV_;
		U.Gen(Modes, 0);
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			// Create DimV[ModeN] x R matrix
			TVVec<TVal, TSizeMdTy> Factor; 
			Factor.Gen(DimV[ModeN], R);
			for (TSizeMdTy RowN = 0; RowN < DimV[ModeN]; RowN++) {
				for (int ColN = 0; ColN < R; ColN++) {
					Factor.At(RowN, ColN) = Rand.GetNrmDev();;
				}
			}
			U.Add(Factor);
		}
	}

	void Display() {
		printf("Number of modes %d\n", Modes);
		printf("Dimensions: ");
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			printf("%I64d ", DimV[ModeN]);
		}		
		printf("\n");
		printf("Number of factors %d\n", R);
		printf("lambda:\n");
		for (int FacN = 0; FacN < R; FacN++) {
			printf("%f ", Lambda[FacN]);
		}
		printf("\n");
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			printf("Mode %d factor:\n", ModeN);
			for (TSizeMdTy RowN = 0; RowN < DimV[ModeN]; RowN++) {
				for (int FacN = 0; FacN < R; FacN++) {
					printf("%f ", U[ModeN].At(RowN, FacN));
				}
				printf("\n");
			}
		}		
	}


	int GetModes() const {return Modes;}	
	const TIntV& GetDimV() const {return DimV;}
	TSizeMdTy GetDim(const int& DimN) const {Assert((DimN >= 0) && (DimN < Modes) && (DimV.Len() == Modes)); return DimV[DimN];}
	const TVVec<TVal, TSizeMdTy>& GetFactor(const int& DimN) const {Assert((DimN >= 0) && (DimN < Modes) && (U.Len() ==  Modes)); return U[DimN];}
	int GetR() const {return R;}
	const TVec<TVal>& GetLambda() const {return Lambda;}
	TVal GetNorm() const {
		// 5.2.5. in http://prod.sandia.gov/techlib/access-control.cgi/2006/067592.pdf
		TVal norm = 0.0;
		TVVec<TVal> HadGram(R, R);
		HadGram.PutAll(1.0);		
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			TVVec<TVal> Temp(R, R);
			Temp.PutAll(0.0);
			for (TSizeMdTy RowN = 0; RowN < DimV[ModeN]; RowN++) {
				for (int Col1N = 0; Col1N < R; Col1N++) {
					for (int Col2N = Col1N; Col2N < R; Col2N++) {
						TVal Prod = U[ModeN].At(RowN, Col1N) * U[ModeN].At(RowN, Col2N);
						Temp.At(Col1N, Col2N) += Prod;						
					}
				}
			}
			for (int Col1N = 0; Col1N < R; Col1N++) {
				for (int Col2N = Col1N; Col2N < R; Col2N++) {
					HadGram.At(Col1N, Col2N) *= Temp.At(Col1N, Col2N);										
				}
			}
		}
		for (int RowN = 0; RowN < R; RowN++) {
			for (int ColN = RowN; ColN < R; ColN++) {
				if (RowN != ColN) {
					norm += 2 * HadGram.At(RowN, ColN) * Lambda[RowN] * Lambda[ColN];
				} else {
					norm += HadGram.At(RowN, ColN) * Lambda[RowN] * Lambda[ColN];
				}
			}
		}
		return sqrt(norm);
	}

	bool IsConsistent() const {
		if (DimV.Len() != Modes) return false;
		if (Modes != U.Len()) return false;
		if (Lambda.Len() != R) return false;
		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			if (DimV[ModeN] != U[ModeN].GetXDim()) return false;
			if (Lambda.Len() != U[ModeN].GetYDim()) return false;
		}
		return true;
	}

	//pointer to sparse tensor, pointer to current CP, index of mode to update	
	void CP_ALS_Update(const TSTensor<TVal, TSizeMdTy, TSizeNzTy>& X, const int& UpdateIdx, const int& nThreads = 1) {
		//printf("iter start, compute flat * khatri rao\n");
		// U[UpdateIdx] = (X_(UpdateIdx) * KhatriRao_{i != UpdateIdx}U_i) * pseudoinv(had_prod_{i != UpdateIdx} (U_i'U_i))
		
		
		U[UpdateIdx].PutAll(0);		
		TSizeNzTy NNZ = X.GetNNZ();

		TTmStopWatch Sw;

		Sw.Start();
		const int R_int = R; // Convert TInt to int for OpenMP C++20 compatibility
		#pragma omp parallel for num_threads(nThreads)
		for (int ColN = 0; ColN < R_int; ColN++) {
			for (TSizeNzTy ElN = 0; ElN < NNZ; ElN++) {
				TVal Temp = X.GetValues().GetVal(ElN);
				for (int ModeN = 0; ModeN < Modes; ModeN++) {
					if (ModeN == UpdateIdx) continue;
					Temp *= U[ModeN].At(X.GetCoordinates().At(ElN, ModeN) , ColN);
				}
				//#pragma omp critical
				U[UpdateIdx].At(X.GetCoordinates().At(ElN, UpdateIdx), ColN) += Temp;
			}
		}
		Sw.Stop();		
		printf("%f\n", Sw.GetSec());

		//printf("X_(UpdateIdx) * KhatriRao_{i != UpdateIdx}U_i) computed\n", UpdateIdx);

		// hadamard product of matrices U[i]'*U[i], i != UpdateIdx
		TVVec<TVal> HadGram(R, R);
		HadGram.PutAll(1.0);	

		for (int ModeN = 0; ModeN < Modes; ModeN++) {
			if (ModeN == UpdateIdx) continue;
			TVVec<TVal> Temp(R, R);
			Temp.PutAll(0.0);
			for (TSizeMdTy RowN = 0; RowN < DimV[ModeN]; RowN++) {
				for (int Col1N = 0; Col1N < R; Col1N++) {
					for (int Col2N = Col1N; Col2N < R; Col2N++) {
						TVal Prod = U[ModeN].At(RowN, Col1N) * U[ModeN].At(RowN, Col2N);
						Temp.At(Col1N, Col2N) += Prod;						
					}
				}
			}
			for (int Col1N = 0; Col1N < R; Col1N++) {
				for (int Col2N = Col1N; Col2N < R; Col2N++) {
					HadGram.At(Col1N, Col2N) *= Temp.At(Col1N, Col2N);										
				}
			}
		}			

		for (int Col1N = 0; Col1N < R; Col1N++) {
			for (int Col2N = 0; Col2N < Col1N; Col2N++) {
				HadGram.At(Col1N, Col2N) *= HadGram.At(Col2N, Col1N);										
			}
		}

		//printf("Hadamard computed\n"); 

		// psuedoinverse of hadamard
		TFltVV HadGramInv; HadGramInv.Gen(R, R);
		TLinAlg::InverseSVD(HadGram, HadGramInv);	

		//printf("Pseudo inverse computed\n");
		// multiply with A.U[UpdateIdx] * IHadGram
		TVec<TVal> Temp(R);
		for (TSizeMdTy RowN = 0; RowN < DimV[UpdateIdx]; RowN++) {			
			Temp.PutAll(0.0);
			for (int ColN = 0; ColN < R; ColN++) {
				for (int k = 0; k < R; k++) {
					Temp[ColN] += U[UpdateIdx].At(RowN, k) * HadGramInv.At(k, ColN);
				}
			}
			for (int ColN = 0; ColN < R; ColN++) {
				U[UpdateIdx].At(RowN, ColN) = Temp[ColN];
			}
		}		
		//printf("Multiply completed\n");
		// norm A.U[UpdateIdx][:, ColN], set lambda[ColN] to norm and normalize A.U[UpdateIdx][:, ColN]
		for (int ColN = 0; ColN < R; ColN++) {
			double norm = 0.0;
			for (TSizeMdTy RowN = 0; RowN < DimV[UpdateIdx]; RowN++) {
				norm += U[UpdateIdx].At(RowN, ColN) * U[UpdateIdx].At(RowN, ColN);
			}
			Lambda[ColN] = sqrt(norm);
			for (TSizeMdTy RowN = 0; RowN < DimV[UpdateIdx]; RowN++) {
				U[UpdateIdx].At(RowN, ColN) /= Lambda[ColN];
			}
		}
		//printf("Normalization, lambda finished\n");
		
	};

	//pointer to sparse tensor, pointer to initial CP, stopping criterion (number of iterations, tolerance)
	void CP_ALS(const TSTensor<TVal, TSizeMdTy, TSizeNzTy>& X, const int& NumIter, const double& Tol = 0.0, const int& nThreads = 1) {
		//Check if each of this and X are consistent
		Assert(IsConsistent());
		Assert(X.IsConsistent());	
		//Check if this and X are compatible (modes and dimensions)
		Assert(X.GetModes() == GetModes());	
		for (int ModeN = 0; ModeN < X.GetModes(); ModeN++) {
			Assert(X.GetDim(ModeN) == GetDim(ModeN));		
		}

		TVal OldRelRes = 1.0; TVal RelRes = 1.0;
		for (int IterN = 0; IterN < NumIter; IterN++) {
			// Update
			CP_ALS_Update(X, IterN % X.GetModes(), nThreads);
			if (Tol > 0.0) {
				// Tolerance check
				TVal normX = X.GetNorm();
				TVal normA = GetNorm();
				TVal innerXA = TTensorOp<TVal, TSizeMdTy, TSizeNzTy>::InnerProduct(X, *this);
				OldRelRes = RelRes;
				RelRes = sqrt(normX * normX - 2 * innerXA + normA * normA)/normX;
				printf("Iter: %d, res: %f, fit: %f\n", IterN, RelRes, 1.0-RelRes);
				if (IterN > 0) {
					if (abs(RelRes - OldRelRes) < Tol) {
						break;
					}
				}
			} else {
				printf("Iter: %d\n", IterN);
			}
		}
	};


};


template <class TVal, class TSizeMdTy, class TSizeNzTy>
class TTensorOp {
public:
	static TVal InnerProduct(const TSTensor<TVal, TSizeMdTy, TSizeNzTy>& X, const TKTensor<TVal, TSizeMdTy>& A) {
		TVal innerp = 0.0;
		TSizeNzTy NNZ = X.GetNNZ();
		int R = A.GetR();
		int Modes = A.GetModes();
		for (TSizeNzTy ElN = 0; ElN < NNZ; ElN++) {
			TVal sum = 0.0;		
			for (int ColN = 0; ColN < R; ColN++) {
				TVal prod = A.GetLambda().GetVal(ColN);
				for (int ModeN = 0; ModeN < Modes; ModeN++) {
					prod *= A.GetFactor(ModeN).At(X.GetCoordinates().At(ElN, ModeN), ColN);
				}
				sum += prod;
			}
			innerp += X.GetValues().GetVal(ElN) * sum;
		}
		return innerp;
	}

};
}

#endif
