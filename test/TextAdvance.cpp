// Copyright (c) 2015 The Khronos Group Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and/or associated documentation files (the
// "Materials"), to deal in the Materials without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Materials, and to
// permit persons to whom the Materials are furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Materials.
//
// MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
// KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
// SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
//    https://www.khronos.org/registry/
//
// THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.

#include "UnitSPIRV.h"

namespace {

using libspirv::AssemblyContext;
using spvtest::AutoText;

TEST(TextAdvance, LeadingNewLines) {
  AutoText input("\n\nWord");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_SUCCESS, data.advance());
  ASSERT_EQ(0, data.position().column);
  ASSERT_EQ(2, data.position().line);
  ASSERT_EQ(2, data.position().index);
}

TEST(TextAdvance, LeadingSpaces) {
  AutoText input("    Word");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_SUCCESS, data.advance());
  ASSERT_EQ(4, data.position().column);
  ASSERT_EQ(0, data.position().line);
  ASSERT_EQ(4, data.position().index);
}

TEST(TextAdvance, LeadingTabs) {
  AutoText input("\t\t\tWord");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_SUCCESS, data.advance());
  ASSERT_EQ(3, data.position().column);
  ASSERT_EQ(0, data.position().line);
  ASSERT_EQ(3, data.position().index);
}

TEST(TextAdvance, LeadingNewLinesSpacesAndTabs) {
  AutoText input("\n\n\t  Word");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_SUCCESS, data.advance());
  ASSERT_EQ(3, data.position().column);
  ASSERT_EQ(2, data.position().line);
  ASSERT_EQ(5, data.position().index);
}

TEST(TextAdvance, LeadingWhitespaceAfterCommentLine) {
  AutoText input("; comment\n \t \tWord");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_SUCCESS, data.advance());
  ASSERT_EQ(4,  data.position().column);
  ASSERT_EQ(1,  data.position().line);
  ASSERT_EQ(14, data.position().index);
}

TEST(TextAdvance, EOFAfterCommentLine) {
  AutoText input("; comment");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_END_OF_STREAM, data.advance());
}

TEST(TextAdvance, NullTerminator) {
  AutoText input("");
  AssemblyContext data(input, nullptr);
  ASSERT_EQ(SPV_END_OF_STREAM, data.advance());
}

}  // anonymous namespace