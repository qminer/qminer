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
