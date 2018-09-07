#include <base.h>
#include <mine.h>
#include "microtest.h"


TEST(TVecDelMemCpy) {
    //ASSERT_EQ(20, 5);
    TIntV Vec;
    for (int i = 0; i < 20; i++) {
        Vec.Add(i);
    }
    Vec.DelMemCpy(0, 5);

    ASSERT_EQ(14, Vec.Len());
    ASSERT_EQ(6, Vec[0]);
}