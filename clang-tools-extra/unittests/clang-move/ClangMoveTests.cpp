//===-- ClangMoveTest.cpp - clang-move unit tests -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ClangMove.h"
#include "unittests/Tooling/RewriterTestContext.h"
#include "clang/Format/Format.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>

namespace clang {
namespace move {
namespace {

const char TestHeaderName[] = "foo.h";

const char TestCCName[] = "foo.cc";

const char TestHeader[] = "namespace a {\n"
                          "class C1; // test\n"
                          "template <typename T> class C2;\n"
                          "namespace b {\n"
                          "// This is a Foo class\n"
                          "// which is used in\n"
                          "// test.\n"
                          "class Foo {\n"
                          "public:\n"
                          "  void f();\n"
                          "\n"
                          "private:\n"
                          "  C1 *c1;\n"
                          "  static int b;\n"
                          "}; // abc\n"
                          "\n"
                          "class Foo2 {\n"
                          "public:\n"
                          "  int f();\n"
                          "};\n"
                          "} // namespace b\n"
                          "} // namespace a\n";

const char TestCC[] = "#include \"foo.h\"\n"
                      "namespace a {\n"
                      "namespace b {\n"
                      "namespace {\n"
                      "// comment1.\n"
                      "void f1() {}\n"
                      "/// comment2.\n"
                      "int kConstInt1 = 0;\n"
                      "} // namespace\n"
                      "\n"
                      "/* comment 3*/\n"
                      "static int kConstInt2 = 1;\n"
                      "\n"
                      "/** comment4\n"
                      "*/\n"
                      "static int help() {\n"
                      "  int a = 0;\n"
                      "  return a;\n"
                      "}\n"
                      "\n"
                      "// comment5\n"
                      "// comment5\n"
                      "void Foo::f() { f1(); }\n"
                      "\n"
                      "/////////////\n"
                      "// comment //\n"
                      "/////////////\n"
                      "int Foo::b = 2;\n"
                      "int Foo2::f() {\n"
                      "  f1();\n"
                      "  return 1;\n"
                      "}\n"
                      "} // namespace b\n"
                      "} // namespace a\n";

const char ExpectedTestHeader[] = "namespace a {\n"
                                  "class C1; // test\n"
                                  "template <typename T> class C2;\n"
                                  "namespace b {\n"
                                  "\n"
                                  "class Foo2 {\n"
                                  "public:\n"
                                  "  int f();\n"
                                  "};\n"
                                  "} // namespace b\n"
                                  "} // namespace a\n";

const char ExpectedTestCC[] = "#include \"foo.h\"\n"
                              "namespace a {\n"
                              "namespace b {\n"
                              "namespace {\n"
                              "// comment1.\n"
                              "void f1() {}\n"
                              "/// comment2.\n"
                              "int kConstInt1 = 0;\n"
                              "} // namespace\n"
                              "\n"
                              "/* comment 3*/\n"
                              "static int kConstInt2 = 1;\n"
                              "\n"
                              "/** comment4\n"
                              "*/\n"
                              "static int help() {\n"
                              "  int a = 0;\n"
                              "  return a;\n"
                              "}\n"
                              "\n"
                              "int Foo2::f() {\n"
                              "  f1();\n"
                              "  return 1;\n"
                              "}\n"
                              "} // namespace b\n"
                              "} // namespace a\n";

const char ExpectedNewHeader[] = "#ifndef NEW_FOO_H\n"
                                 "#define NEW_FOO_H\n"
                                 "namespace a {\n"
                                 "class C1; // test\n"
                                 "template <typename T> class C2;\n"
                                 "namespace b {\n"
                                 "// This is a Foo class\n"
                                 "// which is used in\n"
                                 "// test.\n"
                                 "class Foo {\n"
                                 "public:\n"
                                 "  void f();\n"
                                 "\n"
                                 "private:\n"
                                 "  C1 *c1;\n"
                                 "  static int b;\n"
                                 "}; // abc\n"
                                 "} // namespace b\n"
                                 "} // namespace a\n"
                                 "#endif // NEW_FOO_H\n";

const char ExpectedNewCC[] = "namespace a {\n"
                             "namespace b {\n"
                             "namespace {\n"
                             "// comment1.\n"
                             "void f1() {}\n"
                             "/// comment2.\n"
                             "int kConstInt1 = 0;\n"
                             "} // namespace\n"
                             "/* comment 3*/\n"
                             "static int kConstInt2 = 1;\n"
                             "/** comment4\n"
                             "*/\n"
                             "static int help() {\n"
                             "  int a = 0;\n"
                             "  return a;\n"
                             "}\n"
                             "// comment5\n"
                             "// comment5\n"
                             "void Foo::f() { f1(); }\n"
                             "/////////////\n"
                             "// comment //\n"
                             "/////////////\n"
                             "int Foo::b = 2;\n"
                             "} // namespace b\n"
                             "} // namespace a\n";

std::map<std::string, std::string>
runClangMoveOnCode(const move::ClangMoveTool::MoveDefinitionSpec &Spec,
                   const char *const Header = TestHeader,
                   const char *const CC = TestCC) {
  clang::RewriterTestContext Context;

  std::map<llvm::StringRef, clang::FileID> FileToFileID;
  std::vector<std::pair<std::string, std::string>> FileToSourceText = {
      {TestHeaderName, Header}, {TestCCName, CC}};

  auto CreateFiles = [&FileToSourceText, &Context, &FileToFileID](
      llvm::StringRef Name, llvm::StringRef Code) {
    if (!Name.empty()) {
      FileToFileID[Name] = Context.createInMemoryFile(Name, Code);
    }
  };
  CreateFiles(Spec.NewCC, "");
  CreateFiles(Spec.NewHeader, "");
  CreateFiles(Spec.OldHeader, Header);
  CreateFiles(Spec.OldCC, CC);

  std::map<std::string, tooling::Replacements> FileToReplacements;
  llvm::SmallString<128> InitialDirectory;
  std::error_code EC = llvm::sys::fs::current_path(InitialDirectory);
  assert(!EC);
  (void)EC;
  auto Factory = llvm::make_unique<clang::move::ClangMoveActionFactory>(
      Spec, FileToReplacements, InitialDirectory.str(), "LLVM");

  tooling::runToolOnCodeWithArgs(
      Factory->create(), CC, {"-std=c++11", "-fparse-all-comments"},
      TestCCName, "clang-move", std::make_shared<PCHContainerOperations>(),
      FileToSourceText);
  formatAndApplyAllReplacements(FileToReplacements, Context.Rewrite, "llvm");
  // The Key is file name, value is the new code after moving the class.
  std::map<std::string, std::string> Results;
  for (const auto &It : FileToReplacements) {
    StringRef FilePath = It.first;
    Results[FilePath] = Context.getRewrittenText(FileToFileID[FilePath]);
  }
  return Results;
}

TEST(ClangMove, MoveHeaderAndCC) {
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names = {std::string("a::b::Foo")};
  Spec.OldHeader = "foo.h";
  Spec.OldCC = "foo.cc";
  Spec.NewHeader = "new_foo.h";
  Spec.NewCC = "new_foo.cc";
  std::string ExpectedHeader = "#include \"" + Spec.NewHeader + "\"\n\n";
  auto Results = runClangMoveOnCode(Spec);
  EXPECT_EQ(ExpectedTestHeader, Results[Spec.OldHeader]);
  EXPECT_EQ(ExpectedTestCC, Results[Spec.OldCC]);
  EXPECT_EQ(ExpectedNewHeader, Results[Spec.NewHeader]);
  EXPECT_EQ(ExpectedHeader + ExpectedNewCC, Results[Spec.NewCC]);
}

TEST(ClangMove, MoveHeaderOnly) {
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names = {std::string("a::b::Foo")};
  Spec.OldHeader = "foo.h";
  Spec.NewHeader = "new_foo.h";
  auto Results = runClangMoveOnCode(Spec);
  EXPECT_EQ(2u, Results.size());
  EXPECT_EQ(ExpectedTestHeader, Results[Spec.OldHeader]);
  EXPECT_EQ(ExpectedNewHeader, Results[Spec.NewHeader]);
}

TEST(ClangMove, MoveCCOnly) {
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names = {std::string("a::b::Foo")};
  Spec.OldCC = "foo.cc";
  Spec.NewCC = "new_foo.cc";
  std::string ExpectedHeader = "#include \"foo.h\"\n\n";
  auto Results = runClangMoveOnCode(Spec);
  EXPECT_EQ(2u, Results.size());
  EXPECT_EQ(ExpectedTestCC, Results[Spec.OldCC]);
  EXPECT_EQ(ExpectedHeader + ExpectedNewCC, Results[Spec.NewCC]);
}

TEST(ClangMove, MoveNonExistClass) {
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names = {std::string("NonExistFoo")};
  Spec.OldHeader = "foo.h";
  Spec.OldCC = "foo.cc";
  Spec.NewHeader = "new_foo.h";
  Spec.NewCC = "new_foo.cc";
  auto Results = runClangMoveOnCode(Spec);
  EXPECT_EQ(0u, Results.size());
}

TEST(ClangMove, MoveAll) {
  std::vector<std::string> TestHeaders = {
    "class A {\npublic:\n  int f();\n};",
    // forward declaration.
    "class B;\nclass A {\npublic:\n  int f();\n};",
    // template forward declaration.
    "template <typename T> class B;\nclass A {\npublic:\n  int f();\n};",
    "namespace a {}\nclass A {\npublic:\n  int f();\n};",
    "namespace a {}\nusing namespace a;\nclass A {\npublic:\n  int f();\n};",
  };
  const char Code[] = "#include \"foo.h\"\nint A::f() { return 0; }";
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names.push_back("A");
  Spec.OldHeader = "foo.h";
  Spec.OldCC = "foo.cc";
  Spec.NewHeader = "new_foo.h";
  Spec.NewCC = "new_foo.cc";
  for (const auto& Header : TestHeaders) {
    auto Results = runClangMoveOnCode(Spec, Header.c_str(), Code);
    EXPECT_EQ(Header, Results[Spec.NewHeader]);
    EXPECT_EQ("", Results[Spec.OldHeader]);
    EXPECT_EQ("", Results[Spec.OldCC]);
  }
}

TEST(ClangMove, MoveAllMultipleClasses) {
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  std::vector<std::string> TestHeaders = {
    "class C;\nclass A {\npublic:\n  int f();\n};\nclass B {};",
    "class C;\nclass B;\nclass A {\npublic:\n  int f();\n};\nclass B {};",
  };
  const char Code[] = "#include \"foo.h\"\nint A::f() { return 0; }";
  Spec.Names = {std::string("A"), std::string("B")};
  Spec.OldHeader = "foo.h";
  Spec.OldCC = "foo.cc";
  Spec.NewHeader = "new_foo.h";
  Spec.NewCC = "new_foo.cc";
  for (const auto& Header : TestHeaders) {
    auto Results = runClangMoveOnCode(Spec, Header.c_str(), Code);
    EXPECT_EQ(Header, Results[Spec.NewHeader]);
    EXPECT_EQ("", Results[Spec.OldHeader]);
    EXPECT_EQ("", Results[Spec.OldCC]);
  }
}

TEST(ClangMove, DontMoveAll) {
  const char ExpectedHeader[] = "#ifndef NEW_FOO_H\n"
                                "#define NEW_FOO_H\n"
                                "class A {\npublic:\n  int f();\n};\n"
                                "#endif // NEW_FOO_H\n";
  const char Code[] = "#include \"foo.h\"\nint A::f() { return 0; }";
  std::vector<std::string> TestHeaders = {
    "typedef int Int;\nclass A {\npublic:\n  int f();\n};",
    "using Int=int;\nclass A {\npublic:\n  int f();\n};",
    "class B {};\nclass A {\npublic:\n  int f();\n};",
    "void f() {};\nclass A {\npublic:\n  int f();\n};",
    "enum Color { RED };\nclass A {\npublic:\n  int f();\n};",
  };
  move::ClangMoveTool::MoveDefinitionSpec Spec;
  Spec.Names.push_back("A");
  Spec.OldHeader = "foo.h";
  Spec.OldCC = "foo.cc";
  Spec.NewHeader = "new_foo.h";
  Spec.NewCC = "new_foo.cc";
  for (const auto& Header : TestHeaders) {
    auto Results = runClangMoveOnCode(Spec, Header.c_str(), Code);
    EXPECT_EQ(ExpectedHeader, Results[Spec.NewHeader]);
    // The expected old header should not contain class A definition.
    std::string ExpectedOldHeader = Header.substr(0, Header.size() - 31);
    EXPECT_EQ(ExpectedOldHeader, Results[Spec.OldHeader]);
  }
}

} // namespace
} // namespce move
} // namespace clang
