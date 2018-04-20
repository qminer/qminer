#include <base.h>
#include <mine.h>

#include <qminer.h>

#include "microtest.h"



/////////////////////////////////////////////////////////////////////////////////////////////
// WARNING: These tests use TZipIn, which assumes 7Zip is installed on the machine
/////////////////////////////////////////////////////////////////////////////////////////////




//// Test an empty archive
//
//TEST(TZipInEmptyArchiveFLen) {
//    TStr FNm = "../../test/cpp/files/empty.7z";
//    ASSERT_EQ(0, TZipIn::GetFLen(FNm));
//}
//
//TEST(TZipInEmptyArchiveNew) {
//    TStr FNm = "../../test/cpp/files/empty.7z";
//    PSIn SIn = TZipIn::New(FNm);
//
//    ASSERT_EQ(0, SIn->Len());
//    ASSERT_TRUE(SIn->Eof());
//}
//
//// Test archive with single, empty file
//
//TEST(TZipInEmptyFileFLen) {
//    TStr FNm = "../../test/cpp/files/zero.7z";
//    ASSERT_EQ(0, TZipIn::GetFLen(FNm));
//}
//
//TEST(TZipInEmptyFileNew) {
//    TStr FNm = "../../test/cpp/files/zero.7z";
//    PSIn SIn = TZipIn::New(FNm);
//
//    ASSERT_EQ(0, SIn->Len());
//    ASSERT_TRUE(SIn->Eof());
//}
//
//// Test archive with single file containing "1234567\n"
//
//TEST(TZipInEightFileFLen) {
//    TStr FNm = "../../test/cpp/files/eight.7z";
//    ASSERT_EQ(8, TZipIn::GetFLen(FNm));
//}
//
//TEST(TZipInEightFileNew) {
//    TStr FNm = "../../test/cpp/files/eight.7z";
//    PSIn SIn = TZipIn::New(FNm);
//
//    ASSERT_EQ(8, SIn->Len());
//    ASSERT_FALSE(SIn->Eof());
//}
