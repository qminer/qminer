#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	int output = RUN_ALL_TESTS();
	system("pause");
	return output;
}

