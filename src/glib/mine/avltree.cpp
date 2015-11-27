

AvlTree::AvlTree(): _root(NIL) {
	_n = 0;
    _depth[NIL]     = 0;
    _parent[NIL]    = 0;
    _left[NIL]      = 0;
    _right[NIL]     = 0;
}

TInt AvlTree::first(int node) const {
    if(node == NIL) {
        return NIL;
    }

    while(true) {
        const int left = leftNode(node);
        if(left == NIL) {
            break;
        }
        node = left;
    }
    return node;
}

TInt AvlTree::last(int node) const {
    while(true) {
        const int right = rightNode(node);
        if(right == NIL) {
            break;
        }
        node = right;
    }
    return node;
}

TInt AvlTree::nextNode(int node) const {
    const int right = rightNode(node);
    if(right != NIL) {
        return first(right);
    } else {
        int parent = parentNode(node);
        while(parent != NIL && node == rightNode(parent)) {
            node = parent;
            parent = parentNode(parent);
        }
        return parent;
    }
}

TInt AvlTree::prevNode(int node) const {
    const int left = leftNode(node);
    if(left != NIL) {
        return last(left);
    } else {
        int parent = parentNode(node);
        while(parent != NIL && node == leftNode(parent)) {
            node = parent;
            parent = parentNode(parent);
        }
        return parent;
    }
}

bool AvlTree::add(double x, TInt w) {
	//printf("root: %f\n", _root);
    if(_root == NIL) {
        _root = ++_n;
        _values[_root] = x;
        //printf(" x: %f", x);
        _count[_root] = w;
        _left[_root] = NIL;
        _right[_root] = NIL;
        _parent[_root] = NIL;
        // Update depth and aggregates
        updateAggregates(_root);
        return true;
    } else {
        int node = _root;
        int parent = NIL;
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
        } while(node != NIL);

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
        _left[node] = NIL;
        _right[node] = NIL;
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
    for(int node = _root; node != NIL;) {
        const int cmp = compare(node, x);
        if(cmp < 0) {
            node = leftNode(node);
        } else if(cmp > 0) {
            node = rightNode(node);
        } else {
            return node;
        }
    }
    return NIL;
}

int AvlTree::floor(double x) const {
    int f = NIL;
    for(int node = _root; node != NIL; ) {
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
    int f = NIL;
    for(int node = _root; node != NIL; ) {
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
    for(int p = parentNode(node); p != NIL; p = parentNode(n)) {
        if(n == rightNode(p)) {
            const int leftP = leftNode(p);
            sum += count(p) + aggregatedCount(leftP);
        }
        n = p;
    }
    return sum;
}

void AvlTree::rebalance(int node) {
    for(int n = node; n != NIL; ) {
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
    if(lr != NIL) {
        _parent[lr] = node;
    }

    const int p = parentNode(node);
    _parent[r] = p;
    if(p == NIL) {
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
    if(rl != NIL) {
        _parent[rl] = node;
    }

    const int p = parentNode(node);
    _parent[l] = p;
    if(p == NIL) {
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
