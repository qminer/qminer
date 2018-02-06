#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch_v2.1.1.hpp"

//#include <base.h>
//#include <mine.h>

unsigned int Factorial(unsigned int number) {
    return number <= 1 ? number : Factorial(number - 1)*number;
}

TEST_CASE("Factorials are computed", "[factorial]") {
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);
}

//TEST_CASE( "Tuples" ) {
//    TIntPr IntPair1{ 3, 4 };
//    TIntPr IntPair2 = { 3, 4 };
//    TFltPr FltPr1{ 2.0, 3.0 };
//    TFltPr FltPr2 = { 2.0, 3.0 };
//
//    CHECK(IntPair1.Val1 == 3);
//    CHECK(IntPair1.Val2 == 4);
//    CHECK(IntPair2.Val1 == 3);
//    CHECK(IntPair2.Val2 == 4);
//    CHECK(FltPr1.Val1 == 2.0);
//    CHECK(FltPr1.Val2 == 3.0);
//    CHECK(FltPr2.Val1 == 2.0);
//    CHECK(FltPr2.Val2 == 3.0);
//}
