#include <base.h>
#include <mine.h>
#include <thread.h>
#include "microtest.h"

#ifdef GLib_UNIX

class TestRunnable {

};

TEST(constructor) {
    TThreadPool ThreadPool(1);
}

#endif
