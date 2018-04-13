#include <base.h>
#include <mine.h>
#include "microtest.h"

TEST(TQQueueTestUnlimited) {
    try {
        TQQueue<TInt> Q(64, -1);
        ASSERT_TRUE(Q.Empty());
        for (int i = 0; i < 100; i++) {
            Q.Push(i);
            ASSERT_EQ(Q.Len(), i + 1);
            ASSERT_FALSE(Q.Empty());
        }
        for (int i = 0; i < 100; i++) {
            ASSERT_EQ(Q.Front(), i);
            Q.Pop();
            ASSERT_EQ(Q.Len(), 99 - i);

            if (i < 99) {
                ASSERT_EQ(Q.Back(), 99);
            } else {
                ASSERT_TRUE(Q.Empty());
            }
        }
        TIntV Vec;
        Vec.Add(1);
        Vec.Add(2);
        Vec.Add(3);

        Q.PushV(Vec);
        ASSERT_EQ(Q.Front(), 1);
        ASSERT_EQ(Q.Back(), 3);
        ASSERT_EQ(Q.Len(), 3);
        TIntV Vec2; Q.GetSubValVec(0, 2, Vec2);
        ASSERT_EQ(Vec.Len(), Vec2.Len());
        for (int i = 0; i < 3; i++) {
            ASSERT_EQ(Vec[i], Vec2[i]);
            ASSERT_EQ(Q[i], Vec[i]);
        }

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

TEST(TQQueueTestUnlimited2) {
    try {
        TQQueue<TInt> Q(64, -1);
        ASSERT_TRUE(Q.Empty());
        TIntV Vec;
        Vec.Add(1);
        Vec.Add(2);
        Vec.Add(3);

        Q.PushV(Vec);
        ASSERT_EQ(Q.Front(), 1);
        ASSERT_EQ(Q.Back(), 3);
        ASSERT_EQ(Q.Len(), 3);
        TIntV Vec2; Q.GetSubValVec(0, 2, Vec2);
        ASSERT_EQ(Vec.Len(), Vec2.Len());
        for (int i = 0; i < 3; i++) {
            ASSERT_EQ(Vec[i], Vec2[i]);
            ASSERT_EQ(Q[i], Vec[i]);
        }

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

TEST(TQQueueTestUnlimited3) {
    try {
        TQQueue<TInt> Q(64, -1);
        ASSERT_TRUE(Q.Empty());
        TIntV Vec;
        TLinAlgTransform::RangeV(0, 99, Vec);

        Q.PushV(Vec);
        ASSERT_EQ(Q.Front(), 0);
        ASSERT_EQ(Q.Back(), 99);
        ASSERT_EQ(Q.Len(), 100);
        TIntV Vec2; Q.GetSubValVec(10, 20, Vec2);
        ASSERT_EQ(Vec2.Len(), 11);
        for (int i = 0; i < Vec2.Len(); i++) {
            ASSERT_EQ(Vec[i + 10], Vec2[i]);
            ASSERT_EQ(Q[i + 10], Vec2[i]);
        }

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}



TEST(TQQueueTestLimited1) {
    try {
        TQQueue<TInt> Q(64, 1);
        ASSERT_EQ(Q.Len(), 0);
        ASSERT_TRUE(Q.Empty());

        Q.Push(1);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(2);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 2);
        Q.Push(3);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 3);
        Q.Push(4);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 4);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_TRUE(Q.Empty());

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

TEST(TQQueueTestLimited2) {
    try {
        TQQueue<TInt> Q(64, 2);
        ASSERT_EQ(Q.Len(), 0);
        ASSERT_TRUE(Q.Empty());

        Q.Push(1);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(2);
        ASSERT_EQ(Q.Len(), 2);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(3);
        ASSERT_EQ(Q.Len(), 2);
        ASSERT_EQ(Q.Top(), 2);
        Q.Push(4);
        ASSERT_EQ(Q.Len(), 2);
        ASSERT_EQ(Q.Top(), 3);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 4);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_EQ(Q.Len(), 0);
        ASSERT_TRUE(Q.Empty());
    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

TEST(TQQueueTestLimited3) {
    try {
        TQQueue<TInt> Q(64, 3);
        ASSERT_EQ(Q.Len(), 0);
        ASSERT_TRUE(Q.Empty());

        Q.Push(1);
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(2);
        ASSERT_EQ(Q.Len(), 2);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(3);
        ASSERT_EQ(Q.Len(), 3);
        ASSERT_EQ(Q.Top(), 1);
        Q.Push(4);
        ASSERT_EQ(Q.Len(), 3);
        ASSERT_EQ(Q.Top(), 2);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_EQ(Q.Len(), 2);
        ASSERT_EQ(Q.Top(), 3);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_EQ(Q.Len(), 1);
        ASSERT_EQ(Q.Top(), 4);
        ASSERT_FALSE(Q.Empty());
        Q.Pop();
        ASSERT_EQ(Q.Len(), 0);
        ASSERT_TRUE(Q.Empty());

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}


TEST(TQQueueTestShuffle) {
    try {
        TQQueue<TInt> Q(64, -1);
        ASSERT_TRUE(Q.Empty());

        for (int i = 0; i < 200; i++) {
            Q.Push(i);
            ASSERT_EQ(Q.Len(), i + 1);
            ASSERT_EQ(Q.Front(), 0);
            ASSERT_EQ(Q.Back(), i);
            ASSERT_EQ(Q[i], i);
        }
        TRnd Rnd;
        Q.Shuffle(Rnd);
        TIntV QVec; Q.GetSubValVec(0, 199, QVec);
        QVec.Sort(true);
        ASSERT_EQ(QVec.Len(), 200);
        for (int i = 0; i < 200; i++) {
            ASSERT_EQ(QVec[i], i);
        }
    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}
