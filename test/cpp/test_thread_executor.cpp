#include <base.h>
#include <mine.h>
#include <thread.h>
#include "microtest.h"

#ifdef GLib_UNIX

class TestRunnable {

};

TEST(constructor) {
    //PNotify Notify = TStdNotify::New();
    printf("HEJ\n");
    TThreadExecutor ThreadExecutor(10);//, Notify);
}

#endif
