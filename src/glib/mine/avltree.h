#ifndef AVLTREE_H
#define AVLTREE_H

class AvlTree {

    private:
        int       _root;
        int       _n = 0;
        // TODO We should reallocate tables (ie allow dynamic arrays)
        int       _parent[1000];
        int       _left[1000];
        int       _right[1000];
        char      _depth[1000];
        int       _count[1000];
        double    _values[1000];
        int       _aggregatedCount[1000];

    public:
        static const int NIL = 0;

        AvlTree();

        //
        // Node comparison
        //

        // O(1)
        inline int compare(int node, double x) const {
            if(value(node) < x) {
                return 1;
            } else if(value(node) == x) {
                return 0;
            } else {
                return -1;
            }
        }
    
        // O(1)
        inline int compare(int nodeA, int nodeB) const {
            return compare(nodeA, value(nodeB));
        }

        //
        // Tree accessors
        //

        // O(1)
        inline int root() const {
            return _root;
        }
        // O(1)
        inline int size() const {
            return _n;
        }

        //
        // Node accessors
        //

        // O(1)
        inline int parentNode(int node) const {
            return _parent[node];
        }
        // O(1)
        inline int leftNode(int node) const {
            return _left[node];
        }
        // O(1)
        inline int rightNode(int node) const {
            return _right[node];
        }
        // O(1)
        inline int depth(int node) const {
            return _depth[node];
        }
        // O(1)
        inline int count(int node) const {
            return _count[node];
        }
        // O(1)
        inline int aggregatedCount(int node) const {
            return _aggregatedCount[node];
        }
        // O(1)
        inline double value(int node) const {
            return _values[node];
        }

        //
        // Tree accessors
        //

        // O(log(n))
        int first(int node) const;

        // O(log(n))
        inline int first() const {
            return first(_root);
        }

        // O(log(n)) 
        int last(int node) const;

        // O(log(n))
        int nextNode(int node) const;

        // O(log(n))
        int prevNode(int node) const;

        //
        // Mutators
        //

        // O(1)
        inline void updateAggregates(int node) {
            // Updating depth
        	int gn = depth(leftNode(node));
        	if (leftNode(node)<rightNode(node)) {
        		gn = rightNode(node);
        	}
            _depth[node] = 1 + gn;
            _aggregatedCount[node] = count(node) + aggregatedCount(leftNode(node)) + aggregatedCount(rightNode(node));
        }

        // O(log(n))
        void update(int node, double x, int w) {
            _values[node] += w * (x - value(node)) / count(node);
            _count[node] += w;
            
            for(int n = node; n != NIL; n = parentNode(n)) {
               updateAggregates(n);
            }
        }

        // O(log(n))
        void merge(int node, double x, int w) {
            //assert(value(node) == x);
            _count[node] += w;
            
            for(int n = node; n != NIL; n = parentNode(n)) {
               updateAggregates(n);
            }
        }

        // O(log(n)) 
        bool add(double x, int w);

        // O(log(n))
        int find(double x) const;
        
        // O(log(n))
        int floor(double x) const;

        // O(log(n))
        int floorSum(long sum) const;

        // O(log(n))
        long ceilSum(int node) const;

    private:
        // O(1)
        inline int balanceFactor(int node) const {
            return depth(leftNode(node)) - depth(rightNode(node));
        }

        // (O(log(n)^2)
        void rebalance(int node);

        // O(log(n))
        void rotateLeft(int node);

        // O(log(n))
        // TODO to factor with rotateLeft
        void rotateRight(int node);

    public:
        // 
        // For test or debugging purposes
        //

        // Check balance integrity
        bool checkBalance(int node) const {
            if(node == NIL) {
                return depth(node) == 0;
            } else {
            	int gn = depth(leftNode(node));
                if (leftNode(node)<rightNode(node)) {
            	    gn = rightNode(node);
            	}
                return depth(node) == 1 + gn
                    && TFlt::Abs(depth(leftNode(node)) - depth(rightNode(node))) <= 1
                    && checkBalance(leftNode(node))
                    && checkBalance(rightNode(node))
                ;
            }
        }
        inline bool checkBalance() const {
            return checkBalance(_root);
        }

        // Check aggregates integrity
        bool checkAggregates(int node) const {
            if(node == NIL) {
                return count(node) == 0;
            } else {
                return _aggregatedCount[node] == _count[node] + _aggregatedCount[leftNode(node)] + _aggregatedCount[rightNode(node)]
                    && checkAggregates(leftNode(node))
                    && checkAggregates(rightNode(node))
                ;
            }
        }
        inline bool checkAggregates() const {
            return checkAggregates(_root);
        }

        // Check integrity (order of nodes)
        bool checkIntegrity(int node) const {
            if(node == NIL) {
                return true;
            } else {
                bool ok = true;
                if(leftNode(node) != NIL) {
                    ok &= _values[node] >= _values[leftNode(node)];
                    ok &= checkIntegrity(leftNode(node));
                }
                if(rightNode(node) != NIL) {
                    ok &= _values[node] <= _values[rightNode(node)];
                    ok &= checkIntegrity(rightNode(node));
                }
                return ok;
            }
        }
        inline bool checkIntegrity() const {
            return checkIntegrity(_root);
        }

        // Print as rows
        void print(int node) const {
            if(node == NIL)
                return;
            /*
            cout << "Node " << node << "=> ";
            cout << "Value:" << _values[node] << " ";
            cout << "(" << _values[leftNode(node)] << ";";
            cout << "" << _values[rightNode(node)] << ") ";
            cout << "Depth: " << depth(node) << " ";
            cout << "Count: " <<_count[node] << " ";
            cout << "Aggregate: " << _aggregatedCount[node] << endl;
            */
            print(leftNode(node));
            print(rightNode(node));
        }
        void print() const {
            print(_root);
        }

};

#endif
