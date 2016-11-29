/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef WIN32
    // memory leaks
    //#define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
#ifdef WIN32	
    // memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif	
    ::testing::InitGoogleTest(&argc, argv);
    int output = RUN_ALL_TESTS();
    return output;
}
