#include "base.h"
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

TEST(TStr, OperatorPlusEquals) {
	TStr E; TStr A;

	// += string

	// empyt+= full
	E = ""; A = "abc";
	E += A;
	EXPECT_EQ(E, "abc");
	// self+= self
	A += A;
	EXPECT_EQ(A, "abcabc");
	// full+= empty
	E = "";
	A += E;
	EXPECT_EQ(A, "abcabc");

	// += char *

	// empyt+= full
	E = "";
	E += "abc";
	EXPECT_EQ(E, "abc");
	// full+= empty
	A = "abc";
	A += "";
	EXPECT_EQ(A, "abc");
}