#include "avltree.h"

TAvlTree::TAvlTree(): Root(0) {
	N = 0;
    SetVec(Depth, 0, 0);
    SetVec(Parent, 0,0);
    SetVec(Left, 0, 0);
    SetVec(Right, 0, 0);
    AggregatedCount.AddDat(0,0);
}

TInt TAvlTree::First(TInt& Node) const {
    if(Node == 0) {
        return 0;
    }

    while(true) {
        const TInt Left = GetLeftNode(Node);
        if(Left == 0) {
            break;
        }
        Node = Left;
    }
    return Node;
}

TInt TAvlTree::Last(TInt& Node) const {
    while(true) {
        const TInt Right = GetRightNode(Node);
        if(Right == 0) {
            break;
        }
        Node = Right;
    }
    return Node;
}

TInt TAvlTree::NextNode(TInt& Node) const {
    TInt Right = GetRightNode(Node);
    if(Right != 0) {
        return First(Right);
    } else {
        TInt ParentN = GetParentNode(Node);
        while(ParentN != 0 && Node == GetRightNode(ParentN)) {
            Node = ParentN;
            ParentN = GetParentNode(ParentN);
        }
        return ParentN;
    }
}

TInt TAvlTree::PrevNode(TInt& Node) const {
    TInt Left = GetLeftNode(Node);
    if(Left != 0) {
        return Last(Left);
    } else {
        TInt ParentN = GetParentNode(Node);
        while(ParentN != 0 && Node == GetLeftNode(ParentN)) {
            Node = ParentN;
            ParentN = GetParentNode(ParentN);
        }
        return ParentN;
    }
}

bool TAvlTree::Add(const TFlt& X, const TInt& W) {
    if(Root == 0) {
        Root = ++N;
        Values.AddDat(Root, X);
        SetVec(Count, Root, W);
        SetVec(Left, Root, 0);
        SetVec(Right, Root, 0);
        SetVec(Parent, Root,0);
        // Update depth and aggregates
        UpdateAggregates(Root);
        return true;
    } else {
        TInt Node = Root;
        TInt ParentN = 0;
        TInt Cmp;
        do {
            Cmp = Compare(Node, X);
            if(Cmp < 0) {
                ParentN = Node;
                Node = GetLeftNode(Node);
            } else if (Cmp > 0) {
                ParentN = Node;
                Node = GetRightNode(Node);
            } else {
                // we merge the node
                Merge(Node, X, W);
                return false;
            }
        } while(Node != 0);

        Node = ++N;
        Values.AddDat(Node, X);
        SetVec(Count, Node, W);
        SetVec(Left, Node, 0);
        SetVec(Right, Node, 0);
        SetVec(Parent, Node, ParentN);
        if(Cmp < 0) {
            SetVec(Left, ParentN, Node);
        } else {
            //assert(cmp > 0);
            SetVec(Right, ParentN, Node);
        }

        Rebalance(Node);

        return true;
    }
}

TInt TAvlTree::Find(const TFlt& X) const {
    for(TInt Node = Root; Node != 0;) {
        const TInt Cmp = Compare(Node, X);
        if(Cmp < 0) {
            Node = GetLeftNode(Node);
        } else if(Cmp > 0) {
            Node = GetRightNode(Node);
        } else {
            return Node;
        }
    }
    return 0;
}

TInt TAvlTree::Floor(const TFlt& X) const {
    TInt F = 0;
    for(TInt Node = Root; Node != 0; ) {
        const TInt Cmp = Compare(Node, X);
        if(Cmp <= 0) {
            Node = GetLeftNode(Node);
        } else {
            F = Node;
            Node = GetRightNode(Node);
        }
    }
    return F;
}

TInt TAvlTree::FloorSum(TInt Sum) const {
    TInt F = 0;
    for(TInt Node = Root; Node != 0; ) {
        TInt Left = GetLeftNode(Node);
        const TInt LeftCount = GetAggregatedCount(Left);
        if(LeftCount <= Sum) {
            F = Node;
            Sum -= LeftCount + GetCount(Node);
            Node = GetRightNode(Node);
        } else {
            Node = GetLeftNode(Node);
        }
    }
    return F;
}

TInt TAvlTree::CeilSum(const TInt& Node) const {
    TInt Left = GetLeftNode(Node);
    TInt Sum = GetAggregatedCount(Left);
    TInt N = Node;
    for(TInt P = GetParentNode(Node); P != 0; P = GetParentNode(N)) {
        if(N == GetRightNode(P)) {
            TInt LeftP = GetLeftNode(P);
            Sum += GetCount(P) + GetAggregatedCount(LeftP);
        }
        N = P;
    }
    return Sum;
}

void TAvlTree::Rebalance(const TInt& Node) {
    for(TInt N = Node; N != 0; ) {
        const TInt P = GetParentNode(N);

        UpdateAggregates(N);

        switch(BalanceFactor(N)) {
            case -2: {
                         const TInt Right = GetRightNode(N);
                         if(BalanceFactor(Right) == 1) {
                             RotateRight(Right);
                         }
                         RotateLeft(N);
                         break;
                     }
            case 2: {
                        const TInt Left = GetLeftNode(N);
                        if(BalanceFactor(Left) == -1) {
                            RotateLeft(Left);
                        }
                        RotateRight(N);
                        break;
                    }
            case -1:
            case  1:
            case  0:
                    break;
            default:
            	// We should throw an error
            	EAssert(true == false);

        }

        N = P;
    }
}

void TAvlTree::RotateLeft(const TInt& Node) {
    TInt R  = GetRightNode(Node);
    TInt Lr = GetLeftNode(R);

    SetVec(Right, Node, Lr);
    if(Lr != 0) {
        SetVec(Parent, Lr, Node);
    }

    TInt P = GetParentNode(Node);
    SetVec(Parent, R, P);
    if(P == 0) {
        Root = R;
    } else if(GetLeftNode(P) == Node) {
        SetVec(Left, P, R);
    } else {
         //assert(rightNode(p) == node);
        SetVec(Right, P, R);
    }
    SetVec(Left, R, Node);
    SetVec(Parent, Node, R);
    UpdateAggregates(Node);
    UpdateAggregates(GetParentNode(Node));
}

void TAvlTree::RotateRight(const TInt& Node) {
    TInt L = GetLeftNode(Node);
    TInt Rl = GetRightNode(L);
    SetVec(Left, Node, Rl);
    if(Rl != 0) {
        SetVec(Parent, Rl, Node);
    }

    TInt P = GetParentNode(Node);
    SetVec(Parent, L, P);
    if(P == 0) {
        Root = L;
    } else if(GetRightNode(P) == Node) {
        SetVec(Right, P, L);
    } else {
        //assert(leftNode(p) == node);
        SetVec(Left, P, L);
    }
    SetVec(Right, L, Node);
    SetVec(Parent, Node, L);
    UpdateAggregates(Node);
    UpdateAggregates(GetParentNode(Node));
}
