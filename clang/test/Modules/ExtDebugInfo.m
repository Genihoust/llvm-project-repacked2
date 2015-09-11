// RUN: rm -rf %t
// Test that only forward declarations are emitted for types dfined in modules.

// Modules:
// RUN: %clang_cc1 -x objective-c -g -dwarf-ext-refs -fmodules \
// RUN:     -fmodule-format=obj -fimplicit-module-maps -DMODULES \
// RUN:     -fmodules-cache-path=%t %s -I %S/Inputs -I %t -emit-llvm -o %t-mod.ll
// RUN: cat %t-mod.ll |  FileCheck %s

// PCH:
// RUN: %clang_cc1 -x objective-c -fmodule-format=obj -emit-pch -I%S/Inputs \
// RUN:     -o %t.pch %S/Inputs/DebugObjC.h
// RUN: %clang_cc1 -x objective-c -g -dwarf-ext-refs -fmodule-format=obj \
// RUN:     -include-pch %t.pch %s -emit-llvm -o %t-pch.ll %s
// RUN: cat %t-pch.ll |  FileCheck %s

#ifdef MODULES
@import DebugObjC;
#endif

int foo(ObjCClass *c) {
  [c instanceMethodWithInt: 0];
  return [c property];
}

// CHECK: !DICompositeType(tag: DW_TAG_structure_type,
// CHECK-SAME:             scope: ![[MOD:[0-9]+]],
// CHECK-SAME:             flags: DIFlagFwdDecl)
// CHECK: ![[MOD]] = !DIModule(scope: null, name: {{.*}}DebugObjC
