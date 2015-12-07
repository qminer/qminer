#include "avltree.h"

AvlTree::AvlTree(): Root(0) {
	N = 0;
    Depth.AddDat(0, 0);
    Parent.AddDat(0,0);
    Left.AddDat(0, 0);
    Right.AddDat(0, 0);
    AggregatedCount.AddDat(0,0);
}

TInt AvlTree::First(TInt node) const {
    if(node == 0) {
        return 0;
    }

    while(true) {
        const TInt left = GetLeftNode(node);
        if(left == 0) {
            break;
        }
        node = left;
    }
    return node;
}

TInt AvlTree::Last(TInt node) const {
    while(true) {
        const TInt right = GetRightNode(node);
        if(right == 0) {
            break;
        }
        node = right;
    }
    return node;
}

TInt AvlTree::NextNode(TInt node) const {
    const TInt right = GetRightNode(node);
    if(right != 0) {
        return First(right);
    } else {
        TInt parent = GetParentNode(node);
        while(parent != 0 && node == GetRightNode(parent)) {
            node = parent;
            parent = GetParentNode(parent);
        }
        return parent;
    }
}

TInt AvlTree::PrevNode(TInt node) const {
    const TInt left = GetLeftNode(node);
    if(left != 0) {
        return Last(left);
    } else {
        TInt parent = GetParentNode(node);
        while(parent != 0 && node == GetLeftNode(parent)) {
            node = parent;
            parent = GetParentNode(parent);
        }
        return parent;
    }
}

bool AvlTree::Add(TFlt x, TInt w) {
	//printf("root: %f\n", Root);
    if(Root == 0) {
        Root = ++N;
        Values.AddDat(Root, x);
        Count.AddDat(Root, w);
        Left.AddDat(Root, 0);
        Right.AddDat(Root, 0);
        Parent.AddDat(Root,0);
        // Update depth and aggregates
        UpdateAggregates(Root);
        return true;
    } else {
        TInt node = Root;
        TInt parent = 0;
        TInt cmp;
        do {
            cmp = Compare(node, x);
            if(cmp < 0) {
                parent = node;
                node = GetLeftNode(node);
            } else if (cmp > 0) {
                parent = node;
                node = GetRightNode(node);
            } else {
                // we merge the node
                Merge(node, x, w);
                return false;
            }
        } while(node != 0);

        node = ++N;
        Values.AddDat(node, x);
        Count.AddDat(node, w);
        Left.AddDat(node, 0);
        Right.AddDat(node, 0);
        Parent.AddDat(node, parent);
        if(cmp < 0) {
            Left.AddDat(parent, node);
        } else {
            //assert(cmp > 0);
            Right.AddDat(parent, node);
        }

        Rebalance(node);

        return true;
    }
}

TInt AvlTree::Find(TFlt x) const {
    for(TInt node = Root; node != 0;) {
        const TInt cmp = Compare(node, x);
        if(cmp < 0) {
            node = GetLeftNode(node);
        } else if(cmp > 0) {
            node = GetRightNode(node);
        } else {
            return node;
        }
    }
    return 0;
}

TInt AvlTree::Floor(TFlt x) const {
    TInt f = 0;
    for(TInt node = Root; node != 0; ) {
        const TInt cmp = Compare(node, x);
        if(cmp <= 0) {
            node = GetLeftNode(node);
        } else {
            f = node;
            node = GetRightNode(node);
        }
    }
    return f;
}

TInt AvlTree::FloorSum(long sum) const {
    TInt f = 0;
    for(TInt node = Root; node != 0; ) {
        const TInt left = GetLeftNode(node);
        const long leftCount = GetAggregatedCount(left);
        if(leftCount <= sum) {
            f = node;
            sum -= leftCount + GetCount(node);
            node = GetRightNode(node);
        } else {
            node = GetLeftNode(node);
        }
    }
    return f;
}

long AvlTree::CeilSum(TInt node) const {
    const TInt left = GetLeftNode(node);
    long sum = GetAggregatedCount(left);
    TInt n = node;
    for(TInt p = GetParentNode(node); p != 0; p = GetParentNode(n)) {
        if(n == GetRightNode(p)) {
            const TInt leftP = GetLeftNode(p);
            sum += GetCount(p) + GetAggregatedCount(leftP);
        }
        n = p;
    }
    return sum;
}

void AvlTree::Rebalance(TInt node) {
    for(TInt n = node; n != 0; ) {
        const TInt p = GetParentNode(n);

        UpdateAggregates(n);

        switch(BalanceFactor(n)) {
            case -2: {
                         const TInt right = GetRightNode(n);
                         if(BalanceFactor(right) == 1) {
                             RotateRight(right);
                         }
                         RotateLeft(n);
                         break;
                     }
            case 2: {
                        const TInt left = GetLeftNode(n);
                        if(BalanceFactor(left) == -1) {
                            RotateLeft(left);
                        }
                        RotateRight(n);
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

        n = p;
    }
}

void AvlTree::RotateLeft(TInt node) {
    const TInt r  = GetRightNode(node);
    const TInt lr = GetLeftNode(r);

    Right.AddDat(node, lr);
    if(lr != 0) {
        Parent.AddDat(lr, node);
    }

    const TInt p = GetParentNode(node);
    Parent.AddDat(r, p);
    if(p == 0) {
        Root = r;
    } else if(GetLeftNode(p) == node) {
        Left.AddDat(p, r);
    } else {
         //assert(rightNode(p) == node);
        Right.AddDat(p, r);
    }
    Left.AddDat(r, node);
    Parent.AddDat(node, r);
    UpdateAggregates(node);
    UpdateAggregates(GetParentNode(node));
}

void AvlTree::RotateRight(TInt node) {
    const TInt l = GetLeftNode(node);
    const TInt rl = GetRightNode(l);

    Left.AddDat(node, rl);
    if(rl != 0) {
        Parent.AddDat(rl, node);
    }

    const TInt p = GetParentNode(node);
    Parent.AddDat(l, p);
    if(p == 0) {
        Root = l;
    } else if(GetRightNode(p) == node) {
        Right.AddDat(p, l);
    } else {
        //assert(leftNode(p) == node);
        Left.AddDat(p, l);
    }
    Right.AddDat(l, node);
    Parent.AddDat(node, l);
    UpdateAggregates(node);
    UpdateAggregates(GetParentNode(node));
}
