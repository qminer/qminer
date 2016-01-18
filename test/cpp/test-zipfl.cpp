/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <base.h>

// Test an empty archive

TEST(TZipIn, EmptyArchiveFLen) {
  TStr FNm = "files/empty.7z";
  EXPECT_EQ(0,TZipIn::GetFLen(FNm));
}

TEST(TZipIn, EmptyArchiveNew) {
  TStr FNm = "files/empty.7z";
  PSIn SIn = TZipIn::New(FNm);

  EXPECT_EQ(0,SIn->Len());
  EXPECT_TRUE(SIn->Eof());
}

// Test archive with single, empty file

TEST(TZipIn, EmptyFileFLen) {
  TStr FNm = "files/zero.7z";
  EXPECT_EQ(0,TZipIn::GetFLen(FNm));
}

TEST(TZipIn, EmptyFileNew) {
  TStr FNm = "files/zero.7z";
  PSIn SIn = TZipIn::New(FNm);

  EXPECT_EQ(0,SIn->Len());
  EXPECT_TRUE(SIn->Eof());
}

// Test archive with single file containing "1234567\n"

TEST(TZipIn, EightFileFLen) {
  TStr FNm = "files/eight.7z";
  EXPECT_EQ(8,TZipIn::GetFLen(FNm));
}

TEST(TZipIn, EightFileNew) {
  TStr FNm = "files/eight.7z";
  PSIn SIn = TZipIn::New(FNm);

  EXPECT_EQ(8,SIn->Len());
  EXPECT_FALSE(SIn->Eof());
}
