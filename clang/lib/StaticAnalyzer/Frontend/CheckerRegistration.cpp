//===--- CheckerRegistration.cpp - Registration for the Analyzer Checkers -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Defines the registration function for the analyzer checkers.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Frontend/CheckerRegistration.h"
#include "clang/StaticAnalyzer/Frontend/FrontendActions.h"
#include "clang/StaticAnalyzer/Checkers/ClangCheckers.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/CheckerOptInfo.h"
#include "clang/StaticAnalyzer/Core/CheckerRegistry.h"
#include "clang/Frontend/AnalyzerOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"

using namespace clang;
using namespace ento;

static void registerCheckers(CheckerRegistry &registry,
                             ArrayRef<std::string> plugins) {
  registerBuiltinCheckers(registry);

  // FIXME: register plugins.
}

CheckerManager *ento::createCheckerManager(const AnalyzerOptions &opts,
                                           const LangOptions &langOpts,
                                           ArrayRef<std::string> plugins,
                                           Diagnostic &diags) {
  llvm::OwningPtr<CheckerManager> checkerMgr(new CheckerManager(langOpts));

  SmallVector<CheckerOptInfo, 8> checkerOpts;
  for (unsigned i = 0, e = opts.CheckersControlList.size(); i != e; ++i) {
    const std::pair<std::string, bool> &opt = opts.CheckersControlList[i];
    checkerOpts.push_back(CheckerOptInfo(opt.first.c_str(), opt.second));
  }

  CheckerRegistry allCheckers;
  registerCheckers(allCheckers, plugins);
  allCheckers.initializeManager(*checkerMgr, checkerOpts);

  checkerMgr->finishedCheckerRegistration();

  for (unsigned i = 0, e = checkerOpts.size(); i != e; ++i) {
    if (checkerOpts[i].isUnclaimed())
      diags.Report(diag::warn_unkwown_analyzer_checker)
          << checkerOpts[i].getName();
  }

  return checkerMgr.take();
}

void ento::printCheckerHelp(raw_ostream &out, ArrayRef<std::string> plugins) {
  out << "OVERVIEW: Clang Static Analyzer Checkers List\n\n";
  out << "USAGE: -analyzer-checker <CHECKER or PACKAGE,...>\n\n";

  CheckerRegistry allCheckers;
  registerCheckers(allCheckers, plugins);
  allCheckers.printHelp(out);
}
