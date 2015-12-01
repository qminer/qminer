#include "avltree.h"

AvlTree::AvlTree(): _root(0) {
	_n = 0;
    _depth[0]     = 0;
    _parent[0]    = 0;
    _left[0]      = 0;
    _right[0]     = 0;
}

TInt AvlTree::first(int node) const {
    if(node == 0) {
        return 0;
    }

    while(true) {
        const int left = leftNode(node);
        if(left == 0) {
            break;
        }
        node = left;
    }
    return node;
}

TInt AvlTree::last(int node) const {
    while(true) {
        const int right = rightNode(node);
        if(right == 0) {
            break;
        }
        node = right;
    }
    return node;
}

TInt AvlTree::nextNode(int node) const {
    const int right = rightNode(node);
    if(right != 0) {
        return first(right);
    } else {
        int parent = parentNode(node);
        while(parent != 0 && node == rightNode(parent)) {
            node = parent;
            parent = parentNode(parent);
        }
        return parent;
    }
}

TInt AvlTree::prevNode(int node) const {
    const int left = leftNode(node);
    if(left != 0) {
        return last(left);
    } else {
        int parent = parentNode(node);
        while(parent != 0 && node == leftNode(parent)) {
            node = parent;
            parent = parentNode(parent);
        }
        return parent;
    }
}

bool AvlTree::add(double x, TInt w) {
	//printf("root: %f\n", _root);
    if(_root == 0) {
        _root = ++_n;
        _values[_root] = x;
        //printf(" x: %f", x);
        _count[_root] = w;
        _left[_root] = 0;
        _right[_root] = 0;
        _parent[_root] = 0;
        // Update depth and aggregates
        updateAggregates(_root);
        return true;
    } else {
        int node = _root;
        int parent = 0;
        int cmp;
        do {
            cmp = compare(node, x);
            if(cmp < 0) {
                parent = node;
                node = leftNode(node);
            } else if (cmp > 0) {
                parent = node;
                node = rightNode(node);
            } else {
                // we merge the node
                merge(node, x, w);
                return false;
            }
        } while(node != 0);

        //printf("%i\n", _n);
        node = ++_n;
        //printf("node, _n %i, %i\n", node, _n);
        _values[node] = x;
        //printf(" x: %f", x);
        //for (int i=0; i<10; i++) {
        //	printf("%f, ",_values[i]);
        //}
        //printf("\n");

        _count[node] = w;
        _left[node] = 0;
        _right[node] = 0;
        _parent[node] = parent;
        if(cmp < 0) {
            _left[parent] = node;
        } else {
            //assert(cmp > 0);
            _right[parent] = node;
        }

        rebalance(node);

        return true;
    }
}

int AvlTree::find(double x) const {
    for(int node = _root; node != 0;) {
        const int cmp = compare(node, x);
        if(cmp < 0) {
            node = leftNode(node);
        } else if(cmp > 0) {
            node = rightNode(node);
        } else {
            return node;
        }
    }
    return 0;
}

int AvlTree::floor(double x) const {
    int f = 0;
    for(int node = _root; node != 0; ) {
        const int cmp = compare(node, x);
        if(cmp <= 0) {
            node = leftNode(node);
        } else {
            f = node;
            node = rightNode(node);
        }
    }
    return f;
}

int AvlTree::floorSum(long sum) const {
    int f = 0;
    for(int node = _root; node != 0; ) {
        const int left = leftNode(node);
        const long leftCount = aggregatedCount(left);
        if(leftCount <= sum) {
            f = node;
            sum -= leftCount + count(node);
            node = rightNode(node);
        } else {
            node = leftNode(node);
        }
    }
    return f;
}

long AvlTree::ceilSum(int node) const {
    const int left = leftNode(node);
    long sum = aggregatedCount(left);
    int n = node;
    for(int p = parentNode(node); p != 0; p = parentNode(n)) {
        if(n == rightNode(p)) {
            const int leftP = leftNode(p);
            sum += count(p) + aggregatedCount(leftP);
        }
        n = p;
    }
    return sum;
}

void AvlTree::rebalance(int node) {
    for(int n = node; n != 0; ) {
        const int p = parentNode(n);

        updateAggregates(n);

        switch(balanceFactor(n)) {
            case -2: {
                         const int right = rightNode(n);
                         if(balanceFactor(right) == 1) {
                             rotateRight(right);
                         }
                         rotateLeft(n);
                         break;
                     }
            case 2: {
                        const int left = leftNode(n);
                        if(balanceFactor(left) == -1) {
                            rotateLeft(left);
                        }
                        rotateRight(n);
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

void AvlTree::rotateLeft(int node) {
    const int r  = rightNode(node);
    const int lr = leftNode(r);

    _right[node] = lr;
    if(lr != 0) {
        _parent[lr] = node;
    }

    const int p = parentNode(node);
    _parent[r] = p;
    if(p == 0) {
        _root = r;
    } else if(leftNode(p) == node) {
        _left[p] = r;
    } else {
         //assert(rightNode(p) == node);
        _right[p] = r;
    }
    _left[r] = node;
    _parent[node] = r;
    updateAggregates(node);
    updateAggregates(parentNode(node));
}

void AvlTree::rotateRight(int node) {
    const int l = leftNode(node);
    const int rl = rightNode(l);

    _left[node] = rl;
    if(rl != 0) {
        _parent[rl] = node;
    }

    const int p = parentNode(node);
    _parent[l] = p;
    if(p == 0) {
        _root = l;
    } else if(rightNode(p) == node) {
        _right[p] = l;
    } else {
        //assert(leftNode(p) == node);
        _left[p] = l;
    }
    _right[l] = node;
    _parent[node] = l;
    updateAggregates(node);
    updateAggregates(parentNode(node));
}
