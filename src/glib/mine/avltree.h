#ifndef AVLTREE_H
#define AVLTREE_H

class AvlTree {

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

        AvlTree();

        void SetVec(TIntV& Vec, TInt Key, TInt Val) {
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
        void SetVec(TFltV& Vec, TInt Key, TFlt Val) {
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
        void SetVec(TIntH& Vec, TInt Key, TInt Val) {
        	Vec.AddDat(Key, Val);
        }
        void SetVec(TIntFltH& Vec, TInt Key, TFlt Val) {
        	Vec.AddDat(Key, Val);
        }
        void GetVec(TIntH Vec, TInt Key, TInt& Val) {
        	Val = Vec.GetDat(Key);
        }
        TFlt GetVec(TIntFltH Vec, TInt Key) {
        	return Vec.GetDat(Key);
        }
        TInt GetVec(TIntV Vec, TInt Key) {
			return Vec[Key];
		}
        TFlt GetVec(TFltV Vec, TInt Key) {
        	return Vec[Key];
        }

        //
        // Node comparison
        //

        // O(1)
        inline TInt Compare(TInt node, TFlt x) const {
            if(GetValue(node) < x) {
                return 1;
            } else if(GetValue(node) == x) {
                return 0;
            } else {
                return -1;
            }
        }
    
        // O(1)
        inline TInt Compare(TInt nodeA, TInt nodeB) const {
            return Compare(nodeA, GetValue(nodeB));
        }

        //
        // Tree accessors
        //

        // O(1)
        inline TInt GetRoot() const {
            return Root;
        }
        // O(1)
        inline TInt GetSize() const {
            return N;
        }

        //
        // Node accessors
        //

        // O(1)
        inline TInt GetParentNode(TInt node) const {
        	return Parent.GetDat(node);
        	//return Parent[node];
        }
        // O(1)
        inline TInt GetLeftNode(TInt node) const {
        	return Left.GetDat(node);
        	//return Left[node];
        }
        // O(1)
        inline TInt GetRightNode(TInt node) const {
            return Right.GetDat(node);
            //return Right[node];
        }
        // O(1)
        inline TInt GetDepth(TInt node) const {
            return Depth.GetDat(node);
        	//return Depth[node];
        }
        // O(1)
        inline TInt GetCount(TInt node) const {
            return Count.GetDat(node);
        	//return Count[node];
        }
        // O(1)
        inline TInt GetAggregatedCount(TInt node) const {
            return AggregatedCount.GetDat(node);
        }
        // O(1)
        inline TFlt GetValue(TInt node) const {
        	return Values.GetDat(node);
        }

        //
        // Tree accessors
        //

        // O(log(n))
        TInt First(TInt node) const;

        // O(log(n))
        inline TInt First() const {
            return First(Root);
        }

        // O(log(n)) 
        TInt Last(TInt node) const;

        // O(log(n))
        TInt NextNode(TInt node) const;

        // O(log(n))
        TInt PrevNode(TInt node) const;

        //
        // Mutators
        //

        // O(1)
        inline void UpdateAggregates(TInt node) {
            // Updating depth
        	TInt greater = GetDepth(GetLeftNode(node));
        	if (GetDepth(GetLeftNode(node)) < GetDepth(GetRightNode(node))) {
        		greater = GetDepth(GetRightNode(node));
        	}
        	SetVec(Depth, node, 1 + greater);
        	TInt val = GetCount(node) + GetAggregatedCount(GetLeftNode(node)) + GetAggregatedCount(GetRightNode(node));
            AggregatedCount.AddDat(node, val);
        }

        // O(log(n))
        void Update(TInt node, TFlt x, TInt w) {
        	TFlt old = Values.GetDat(node);
        	Values.AddDat(node, old + w * (x - GetValue(node)) / GetCount(node));
        	TInt old_count = Count.GetDat(node);
        	SetVec(Count, node, old_count + w);

            for(TInt n = node; n != 0; n = GetParentNode(n)) {
               UpdateAggregates(n);
            }
        }

        // O(log(n))
        void Merge(TInt node, TFlt x, TInt w) {
            EAssert(GetValue(node) == x);
            TInt old_count = Count.GetDat(node);
            SetVec(Count, node, old_count + w);
            
            for(TInt n = node; n != 0; n = GetParentNode(n)) {
               UpdateAggregates(n);
            }
        }

        // O(log(n)) 
        bool Add(TFlt x, TInt w);

        // O(log(n))
        TInt Find(TFlt x) const;
        
        // O(log(n))
        TInt Floor(TFlt x) const;

        // O(log(n))
        TInt FloorSum(long sum) const;

        // O(log(n))
        long CeilSum(TInt node) const;

    private:
        // O(1)
        inline TInt BalanceFactor(TInt node) const {
            return GetDepth(GetLeftNode(node)) - GetDepth(GetRightNode(node));
        }

        // (O(log(n)^2)
        void Rebalance(TInt node);

        // O(log(n))
        void RotateLeft(TInt node);

        // O(log(n))
        // TODO to factor with rotateLeft
        void RotateRight(TInt node);

    public:
        // 
        // For test or debugging purposes
        //

        // Check balance integrity
        bool CheckBalance(TInt node) const {
            if(node == 0) {
                return GetDepth(node) == 0;
            } else {
            	TInt greater = GetDepth(GetLeftNode(node));
            	if (GetDepth(GetLeftNode(node)) < GetDepth(GetRightNode(node))) {
            		greater = GetDepth(GetRightNode(node));
            	}
            	return GetDepth(node) == 1 + greater
                    && abs(GetDepth(GetLeftNode(node)) - GetDepth(GetRightNode(node))) <= 1
                    && CheckBalance(GetLeftNode(node))
                    && CheckBalance(GetRightNode(node))
                ;
            }
        }

        inline bool CheckBalance() const {
            return CheckBalance(Root);
        }

        // Check aggregates integrity
        bool CheckAggregates(TInt node) const {
            if(node == 0) {
                return GetCount(node) == 0;
            } else {
            	TInt val = Count.GetDat(node) + AggregatedCount.GetDat(GetLeftNode(node)) + AggregatedCount.GetDat(GetRightNode(node));
                return AggregatedCount.GetDat(node) == val && CheckAggregates(GetLeftNode(node)) && CheckAggregates(GetRightNode(node));
            }
        }
        inline bool CheckAggregates() const {
            return CheckAggregates(Root);
        }

        // Check integrity (order of nodes)
        bool CheckIntegrity(TInt node) const {
            if(node == 0) {
                return true;
            } else {
                bool ok = true;
                if(GetLeftNode(node) != 0) {
                    ok &= Values.GetDat(node) >= Values.GetDat(GetLeftNode(node));
                    ok &= CheckIntegrity(GetLeftNode(node));
                }
                if(GetRightNode(node) != 0) {
                    ok &= Values.GetDat(node) <= Values.GetDat(GetRightNode(node));
                    ok &= CheckIntegrity(GetRightNode(node));
                }
                return ok;
            }
        }
        inline bool CheckIntegrity() const {
            return CheckIntegrity(Root);
        }

        // Print as rows
        void print(TInt node) const {
            if(node == 0)
                return;
        }
        void print() const {
            print(Root);
        }

};

#endif
