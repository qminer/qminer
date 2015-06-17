#ifndef SRC_GLIB_BASE_OPT_H_
#define SRC_GLIB_BASE_OPT_H_

namespace TOpt {

class TOptUtil {
public:
	/// Finds a feasible solution to the linear system of equations x*A <= b
	/// if the input vector x is not empty then it is used as a starting point
	static void FindFeasible(const TFltVV& A, const TFltV& b, TFltV& x,
			const PNotify Notify=TNotify::NullNotify);
};

}

#endif /* SRC_GLIB_BASE_OPT_H_ */
