#ifndef AVLTREE_H
#define AVLTREE_H

class TAvlTree {

    private:
        TInt Root;
        TInt N;
        TIntH Parent;
        TIntH Left;
        TIntH Right;
        TIntH Depth;
        TIntH Count;
        TIntFltH Values;
        TIntH AggregatedCount;

    public:

        TAvlTree();

        void SetVec(TIntV& Vec, const TInt& Key, const TInt& Val) const {
        	if (Vec.Len()-1 == Key) {
        		Vec.Add(Val);
        	}
        	else if (Vec.Len()-1 < Key) {
        		Vec.Add(Val);
        		Vec[Key] = Val;
        	}
        	else {
        		Vec[Key] = Val;
        	}
        }
        void SetVec(TFltV& Vec, TInt Key, TFlt Val) const {
        	if (Vec.Len()-1 == Key) {
				Vec.Add(Val);
			}
			else if (Vec.Len()-1 < Key) {
				Vec.Add(Val);
				Vec[Key] = Val;
			}
			else {
				Vec[Key] = Val;
			}
        }
        void SetVec(TIntH& Vec, const TInt& Key, const TInt& Val) const {
        	Vec.AddDat(Key, Val);
        }
        void SetVec(TIntFltH& Vec, const TInt& Key, const TFlt& Val) const {
        	Vec.AddDat(Key, Val);
        }
        TInt GetVec(const TIntH& Vec, const TInt& Key, const TInt& Val) const {
        	return Vec.GetDat(Key);
        }
        TFlt GetVec(TIntFltH& Vec, const TInt& Key) const {
        	return Vec.GetDat(Key);
        }
        TInt GetVec(TIntV& Vec, const TInt& Key) const {
			return Vec.GetVal(Key);
		}
        TFlt GetVec(TFltV& Vec, const TInt& Key) const {
        	return Vec.GetVal(Key);
        }

        //
        // Node comparison
        //

        // O(1)
        TInt Compare(const TInt& Node, const TFlt& x) const {
            if(GetValue(Node) < x) {
                return 1;
            } else if(GetValue(Node) == x) {
                return 0;
            } else {
                return -1;
            }
        }
    
        // O(1)
        TInt Compare(const TInt& NodeA, const TInt& NodeB) const {
        	TInt B = (int)GetValue(NodeB);
            return Compare(NodeA, B);
        }

        //
        // Tree accessors
        //

        // O(1)
        TInt GetRoot() const {
            return Root;
        }
        // O(1)
        TInt GetSize() const {
            return N;
        }

        //
        // Node accessors
        //

        // O(1)
        TInt GetParentNode(const TInt& Node) const {
        		//return GetVec(Parent, Node);
                return Parent.GetDat(Node);
        		//return Parent.GetVal(Node);
        }
        TInt GetParentNode(const TIntH& Parent, const TInt& Node) const {
        	return Parent.GetDat(Node);
        }
        TInt GetParentNode(const TIntV& Parent, const TInt& Node) const {
        	return Parent.GetVal(Node);
        }
        // O(1)
        TInt GetLeftNode(const TInt& Node) const {
        	return Left.GetDat(Node);
        	//return Left[node];
        }
        // O(1)
        TInt GetRightNode(const TInt& Node) const {
            return Right.GetDat(Node);
            //return Right[node];
        }
        // O(1)
        TInt GetDepth(const TInt& Node) const {
            return Depth.GetDat(Node);
        	//return Depth[node];
        }
        // O(1)
        TInt GetCount(const TInt& Node) const {
            return Count.GetDat(Node);
        	//return Count[node];
        }
        // O(1)
        TInt GetAggregatedCount(const TInt& Node) const {
            return AggregatedCount.GetDat(Node);
        }
        // O(1)
        TFlt GetValue(const TInt& Node) const {
        	return Values.GetDat(Node);
        }

        //
        // Tree accessors
        //

        // O(log(n))
        TInt First(TInt& Node) const;

        // O(log(n))
        TInt First() {
            return First(Root);
        }

        // O(log(n)) 
        TInt Last(TInt& Node) const;

        // O(log(n))
        TInt NextNode(TInt& Node) const;

        // O(log(n))
        TInt PrevNode(TInt& Node) const;

        //
        // Mutators
        //

        // O(1)
        void UpdateAggregates(const TInt& Node) {
            // Updating depth
        	TInt Ln = GetLeftNode(Node);
        	TInt Rn = GetRightNode(Node);
        	TInt Greater = GetDepth(Ln);
        	if (GetDepth(Ln) < GetDepth(Rn)) {
        		Greater = GetDepth(Rn);
        	}
        	SetVec(Depth, Node, 1 + Greater);
        	TInt Val = GetCount(Node) + GetAggregatedCount(Ln) + GetAggregatedCount(Rn);
            AggregatedCount.AddDat(Node, Val);
        }

        // O(log(n))
        void Update(const TInt& Node, const TFlt& X, const TFlt& W) {
        	TFlt Old = Values.GetDat(Node);
        	Values.AddDat(Node, Old + W * (X - GetValue(Node)) / GetCount(Node));
        	TInt Oldcount = Count.GetDat(Node);
        	SetVec(Count, Node, Oldcount + W);

            for(TInt N = Node; N != 0; N = GetParentNode(N)) {
               UpdateAggregates(N);
            }
        }

        // O(log(n))
        void Merge(const TInt& Node, const TFlt& X, const TInt& W) {
            TInt OldCount = Count.GetDat(Node);
            SetVec(Count, Node, OldCount + W);
            
            for(TInt N = Node; N != 0; N = GetParentNode(N)) {
               UpdateAggregates(N);
            }
        }

        // O(log(n)) 
        bool Add(const TFlt& X, const TInt& W);

        // O(log(n))
        TInt Find(const TFlt& X) const;
        
        // O(log(n))
        TInt Floor(const TFlt& X) const;

        // O(log(n))
        TInt FloorSum(TInt Sum) const;

        // O(log(n))
        TInt CeilSum(const TInt& Node) const;

    private:
        // O(1)
        inline TInt BalanceFactor(const TInt& Node) const {
        	TInt Ln = GetLeftNode(Node);
        	TInt Rn = GetRightNode(Node);
            return GetDepth(Ln) - GetDepth(Rn);
        }

        // (O(log(n)^2)
        void Rebalance(const TInt& Node);

        // O(log(n))
        void RotateLeft(const TInt& Node);

        // O(log(n))
        // TODO to factor with rotateLeft
        void RotateRight(const TInt& Node);

    public:
        // 
        // For test or debugging purposes
        //

        // Check balance integrity
        bool CheckBalance(const TInt& Node) const {
        	TInt Ln = GetLeftNode(Node);
        	TInt Rn = GetRightNode(Node);
            if(Node == 0) {
                return GetDepth(Node) == 0;
            } else {
            	TInt Greater = GetDepth(Ln);
            	if (GetDepth(Ln) < GetDepth(Rn)) {
            		Greater = GetDepth(Rn);
            	}
            	return GetDepth(Node) == 1 + Greater
                    && TInt::Abs(GetDepth(Ln) - GetDepth(Rn)) <= 1
                    && CheckBalance(GetLeftNode(Node))
                    && CheckBalance(GetRightNode(Node))
                ;
            }
        }

        inline bool CheckBalance() const {
            return CheckBalance(Root);
        }

        // Check aggregates integrity
        bool CheckAggregates(const TInt& Node) const {
            if(Node == 0) {
                return GetCount(Node) == 0;
            } else {
            	TInt val = Count.GetDat(Node) + AggregatedCount.GetDat(GetLeftNode(Node)) + AggregatedCount.GetDat(GetRightNode(Node));
                return AggregatedCount.GetDat(Node) == val && CheckAggregates(GetLeftNode(Node)) && CheckAggregates(GetRightNode(Node));
            }
        }
        inline bool CheckAggregates() const {
            return CheckAggregates(Root);
        }

        // Check integrity (order of Nodes)
        bool CheckIntegrity(const TInt& Node) const {
            if(Node == 0) {
                return true;
            } else {
                bool Ok = true;
                if(GetLeftNode(Node) != 0) {
                    Ok &= Values.GetDat(Node) >= Values.GetDat(GetLeftNode(Node));
                    Ok &= CheckIntegrity(GetLeftNode(Node));
                }
                if(GetRightNode(Node) != 0) {
                    Ok &= Values.GetDat(Node) <= Values.GetDat(GetRightNode(Node));
                    Ok &= CheckIntegrity(GetRightNode(Node));
                }
                return Ok;
            }
        }
        inline bool CheckIntegrity() const {
            return CheckIntegrity(Root);
        }

        // Print as rows
        void Print(const TInt& Node) const {
            if(Node == 0)
                return;
        }
        void Print() const {
            Print(Root);
        }
        void Reset() {
        	// Clear values
        	Root = 0;
        	N = 0;
        	Parent.Clr();
        	Left.Clr();
        	Right.Clr();
        	Depth.Clr();
        	Count.Clr();
        	Values.Clr();
        	AggregatedCount.Clr();
        	// Set values
        	N = 0;
            SetVec(Depth, 0, 0);
            SetVec(Parent, 0,0);
            SetVec(Left, 0, 0);
            SetVec(Right, 0, 0);
            AggregatedCount.AddDat(0,0);
        }
        void SaveState(TSOut& SOut) const {
        	Root.Save(SOut);
        	N.Save(SOut);
        	Parent.Save(SOut);
        	Left.Save(SOut);
        	Right.Save(SOut);
        	Depth.Save(SOut);
        	Count.Save(SOut);
        	Values.Save(SOut);
        	AggregatedCount.Save(SOut);
        };
        void LoadState(TSIn& SIn)  {
			Root.Load(SIn);
			N.Load(SIn);
			Parent.Load(SIn);
			Left.Load(SIn);
			Right.Load(SIn);
			Depth.Load(SIn);
			Count.Load(SIn);
			Values.Load(SIn);
			AggregatedCount.Load(SIn);
        };
};

#endif
