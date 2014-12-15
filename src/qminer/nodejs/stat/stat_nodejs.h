#ifndef QMINER_STAT_NODEJS
#define QMINER_STAT_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "utils.h"
#include "la_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-Stat

class TNodeJsStat : public node::ObjectWrap {
public:
	static void Init(v8::Handle<v8::Object> exports);
public:
   //# 
	//# **Functions and properties:**
	//# 
	//# - `num = la.mean(vec)` - returns mean `num` of vector `vec`.
	//# - `vec = la.mean(mat)` - returns `vec` containing the mean of each column from matrix `mat`. 1 is col mean, 2 is row mean.
	JsDeclareFunction(mean);
	//# - `vec = la.std(mat)` - returns `vec` containing the standard deviation of each column from matrix `mat`.
	//# - `vec = la.std(mat, flag)` - set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
	//# - `vec = la.std(mat, flag, dim)` - computes the standard deviations along the dimension of `mat` specified by parameter `dim`. 1 is col std, 2 is row std.
	JsDeclareFunction(std);
	//# - `zscoreResult = la.zscore(mat)` - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`.
	//# - `zscoreResult = la.zscore(mat, flag)` - returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
	//# - `zscoreResult = la.zscore(mat, flag, dim)` -  Computes the standard deviations along the dimension of X specified by parameter `dim`. Returns `zscoreResult` containing the standard deviation `zscoreResult.sigma` of each column from matrix `mat`, mean vector `zscoreResult.mu` and z-score matrix `zscoreResult.Z`. Set `flag` to 0 to normalize Y by n-1; set flag to 1 to normalize by n.
	JsDeclareFunction(zscore);
private:
   // 
};

#endif

