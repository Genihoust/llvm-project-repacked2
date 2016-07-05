; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+avx512bw | FileCheck %s

declare <64 x i8> @llvm.x86.avx512.mask.pshuf.b.512(<64 x i8>, <64 x i8>, <64 x i8>, i64)

declare <8 x double> @llvm.x86.avx512.mask.permvar.df.512(<8 x double>, <8 x i64>, <8 x double>, i8)
declare <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64>, <8 x i64>, <8 x i64>, i8)
declare <16 x i32> @llvm.x86.avx512.mask.permvar.si.512(<16 x i32>, <16 x i32>, <16 x i32>, i16)
declare <32 x i16> @llvm.x86.avx512.mask.permvar.hi.512(<32 x i16>, <32 x i16>, <32 x i16>, i32)

declare <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64>, <8 x double>, <8 x double>, i8)
declare <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32>, <16 x float>, <16 x float>, i16)

declare <8 x i64> @llvm.x86.avx512.maskz.vpermt2var.q.512(<8 x i64>, <8 x i64>, <8 x i64>, i8)
declare <16 x i32> @llvm.x86.avx512.maskz.vpermt2var.d.512(<16 x i32>, <16 x i32>, <16 x i32>, i16)
declare <32 x i16> @llvm.x86.avx512.maskz.vpermt2var.hi.512(<32 x i16>, <32 x i16>, <32 x i16>, i32)

define <8 x double> @combine_permvar_8f64_identity(<8 x double> %x0, <8 x double> %x1) {
; CHECK-LABEL: combine_permvar_8f64_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.mask.permvar.df.512(<8 x double> %x0, <8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x double> %x1, i8 -1)
  %res1 = call <8 x double> @llvm.x86.avx512.mask.permvar.df.512(<8 x double> %res0, <8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x double> %res0, i8 -1)
  ret <8 x double> %res1
}
define <8 x double> @combine_permvar_8f64_identity_mask(<8 x double> %x0, <8 x double> %x1, i8 %m) {
; CHECK-LABEL: combine_permvar_8f64_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm2 = [7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermpd %zmm0, %zmm2, %zmm1 {%k1}
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm0 = [7,14,5,12,3,10,1,8]
; CHECK-NEXT:    vpermpd %zmm1, %zmm0, %zmm1 {%k1}
; CHECK-NEXT:    vmovaps %zmm1, %zmm0
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.mask.permvar.df.512(<8 x double> %x0, <8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x double> %x1, i8 %m)
  %res1 = call <8 x double> @llvm.x86.avx512.mask.permvar.df.512(<8 x double> %res0, <8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x double> %res0, i8 %m)
  ret <8 x double> %res1
}

define <8 x i64> @combine_permvar_8i64_identity(<8 x i64> %x0, <8 x i64> %x1) {
; CHECK-LABEL: combine_permvar_8i64_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64> %x0, <8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x i64> %x1, i8 -1)
  %res1 = call <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64> %res0, <8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x i64> %res0, i8 -1)
  ret <8 x i64> %res1
}
define <8 x i64> @combine_permvar_8i64_identity_mask(<8 x i64> %x0, <8 x i64> %x1, i8 %m) {
; CHECK-LABEL: combine_permvar_8i64_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm2 = [7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermq %zmm0, %zmm2, %zmm1 {%k1}
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm0 = [7,14,5,12,3,10,1,8]
; CHECK-NEXT:    vpermq %zmm1, %zmm0, %zmm1 {%k1}
; CHECK-NEXT:    vmovaps %zmm1, %zmm0
; CHECK-NEXT:    retq
  %res0 = call <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64> %x0, <8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x i64> %x1, i8 %m)
  %res1 = call <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64> %res0, <8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x i64> %res0, i8 %m)
  ret <8 x i64> %res1
}

define <8 x double> @combine_vpermt2var_8f64_identity(<8 x double> %x0, <8 x double> %x1) {
; CHECK-LABEL: combine_vpermt2var_8f64_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x double> %x0, <8 x double> %x1, i8 -1)
  %res1 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x double> %res0, <8 x double> %res0, i8 -1)
  ret <8 x double> %res1
}
define <8 x double> @combine_vpermt2var_8f64_identity_mask(<8 x double> %x0, <8 x double> %x1, i8 %m) {
; CHECK-LABEL: combine_vpermt2var_8f64_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm2 = [7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermt2pd %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm1 = [7,14,5,12,3,10,1,8]
; CHECK-NEXT:    vpermt2pd %zmm0, %zmm1, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x double> %x0, <8 x double> %x1, i8 %m)
  %res1 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x double> %res0, <8 x double> %res0, i8 %m)
  ret <8 x double> %res1
}

define <8 x double> @combine_vpermt2var_8f64_movddup(<8 x double> %x0, <8 x double> %x1) {
; CHECK-LABEL: combine_vpermt2var_8f64_movddup:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovddup {{.*#+}} zmm0 = zmm0[0,0,2,2,4,4,6,6]
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 0, i64 0, i64 2, i64 2, i64 4, i64 4, i64 undef, i64 undef>, <8 x double> %x0, <8 x double> %x1, i8 -1)
  ret <8 x double> %res0
}
define <8 x double> @combine_vpermt2var_8f64_movddup_load(<8 x double> *%p0, <8 x double> %x1) {
; CHECK-LABEL: combine_vpermt2var_8f64_movddup_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovddup {{.*#+}} zmm0 = mem[0,0,2,2,4,4,6,6]
; CHECK-NEXT:    retq
  %x0 = load <8 x double>, <8 x double> *%p0
  %res0 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 0, i64 0, i64 2, i64 2, i64 4, i64 4, i64 6, i64 6>, <8 x double> %x0, <8 x double> %x1, i8 -1)
  ret <8 x double> %res0
}
define <8 x double> @combine_vpermt2var_8f64_movddup_mask(<8 x double> %x0, <8 x double> %x1, i8 %m) {
; CHECK-LABEL: combine_vpermt2var_8f64_movddup_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovddup {{.*#+}} zmm0 {%k1} {z} = zmm0[0,0,2,2,4,4,6,6]
; CHECK-NEXT:    retq
  %res0 = call <8 x double> @llvm.x86.avx512.maskz.vpermt2var.pd.512(<8 x i64> <i64 0, i64 0, i64 2, i64 2, i64 4, i64 4, i64 6, i64 6>, <8 x double> %x0, <8 x double> %x1, i8 %m)
  ret <8 x double> %res0
}

define <8 x i64> @combine_vpermt2var_8i64_identity(<8 x i64> %x0, <8 x i64> %x1) {
; CHECK-LABEL: combine_vpermt2var_8i64_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <8 x i64> @llvm.x86.avx512.maskz.vpermt2var.q.512(<8 x i64> <i64 undef, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x i64> %x0, <8 x i64> %x1, i8 -1)
  %res1 = call <8 x i64> @llvm.x86.avx512.maskz.vpermt2var.q.512(<8 x i64> <i64 undef, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x i64> %res0, <8 x i64> %res0, i8 -1)
  ret <8 x i64> %res1
}
define <8 x i64> @combine_vpermt2var_8i64_identity_mask(<8 x i64> %x0, <8 x i64> %x1, i8 %m) {
; CHECK-LABEL: combine_vpermt2var_8i64_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm2 = [7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermt2q %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    vmovdqa64 {{.*#+}} zmm1 = [7,14,5,12,3,10,1,8]
; CHECK-NEXT:    vpermt2q %zmm0, %zmm1, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <8 x i64> @llvm.x86.avx512.maskz.vpermt2var.q.512(<8 x i64> <i64 7, i64 6, i64 5, i64 4, i64 3, i64 2, i64 1, i64 0>, <8 x i64> %x0, <8 x i64> %x1, i8 %m)
  %res1 = call <8 x i64> @llvm.x86.avx512.maskz.vpermt2var.q.512(<8 x i64> <i64 7, i64 14, i64 5, i64 12, i64 3, i64 10, i64 1, i64 8>, <8 x i64> %res0, <8 x i64> %res0, i8 %m)
  ret <8 x i64> %res1
}

define <16 x float> @combine_vpermt2var_16f32_identity(<16 x float> %x0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  %res1 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 15, i32 30, i32 13, i32 28, i32 11, i32 26, i32 9, i32 24, i32 7, i32 22, i32 5, i32 20, i32 3, i32 18, i32 1, i32 16>, <16 x float> %res0, <16 x float> %res0, i16 -1)
  ret <16 x float> %res1
}
define <16 x float> @combine_vpermt2var_16f32_identity_mask(<16 x float> %x0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermt2ps %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm1 = [15,30,13,28,11,26,9,24,7,22,5,20,3,18,1,16]
; CHECK-NEXT:    vpermt2ps %zmm0, %zmm1, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  %res1 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 15, i32 30, i32 13, i32 28, i32 11, i32 26, i32 9, i32 24, i32 7, i32 22, i32 5, i32 20, i32 3, i32 18, i32 1, i32 16>, <16 x float> %res0, <16 x float> %res0, i16 %m)
  ret <16 x float> %res1
}

define <16 x float> @combine_vpermt2var_16f32_vmovddup(<16 x float> %x0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovddup:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [0,1,0,1,4,5,4,5,8,9,8,9,12,13,12,13]
; CHECK-NEXT:    vpermt2ps %zmm1, %zmm2, %zmm0
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5, i32 8, i32 9, i32 8, i32 9, i32 12, i32 13, i32 12, i32 13>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovddup_load(<16 x float> *%p0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovddup_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovaps (%rdi), %zmm1
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [0,1,0,1,4,5,4,5,8,9,8,9,12,13,12,13]
; CHECK-NEXT:    vpermt2ps %zmm0, %zmm2, %zmm1
; CHECK-NEXT:    vmovaps %zmm1, %zmm0
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5, i32 8, i32 9, i32 8, i32 9, i32 12, i32 13, i32 12, i32 13>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovddup_mask(<16 x float> %x0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovddup_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [0,1,0,1,4,5,4,5,8,9,8,9,12,13,12,13]
; CHECK-NEXT:    vpermt2ps %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5, i32 8, i32 9, i32 8, i32 9, i32 12, i32 13, i32 12, i32 13>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovddup_mask_load(<16 x float> *%p0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovddup_mask_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %esi, %k1
; CHECK-NEXT:    vmovaps (%rdi), %zmm1
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [0,1,0,1,4,5,4,5,8,9,8,9,12,13,12,13]
; CHECK-NEXT:    vpermt2ps %zmm0, %zmm2, %zmm1 {%k1} {z}
; CHECK-NEXT:    vmovaps %zmm1, %zmm0
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 1, i32 0, i32 1, i32 4, i32 5, i32 4, i32 5, i32 8, i32 9, i32 8, i32 9, i32 12, i32 13, i32 12, i32 13>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}

define <16 x float> @combine_vpermt2var_16f32_vmovshdup(<16 x float> %x0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovshdup:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovshdup {{.*#+}} zmm0 = zmm0[1,1,3,3,5,5,7,7,9,9,11,11,13,13,15,15]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 1, i32 1, i32 3, i32 3, i32 5, i32 5, i32 7, i32 7, i32 9, i32 9, i32 11, i32 11, i32 13, i32 13, i32 15, i32 15>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovshdup_load(<16 x float> *%p0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovshdup_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovshdup {{.*#+}} zmm0 = mem[1,1,3,3,5,5,7,7,9,9,11,11,13,13,15,15]
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 1, i32 1, i32 3, i32 3, i32 5, i32 5, i32 7, i32 7, i32 9, i32 9, i32 11, i32 11, i32 13, i32 13, i32 15, i32 15>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovshdup_mask(<16 x float> %x0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovshdup_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovshdup {{.*#+}} zmm0 {%k1} {z} = zmm0[1,1,3,3,5,5,7,7,9,9,11,11,13,13,15,15]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 1, i32 1, i32 3, i32 3, i32 5, i32 5, i32 7, i32 7, i32 9, i32 9, i32 11, i32 11, i32 13, i32 13, i32 15, i32 15>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}

define <16 x float> @combine_vpermt2var_16f32_vmovsldup(<16 x float> %x0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovsldup:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovsldup {{.*#+}} zmm0 = zmm0[0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 0, i32 2, i32 2, i32 4, i32 4, i32 6, i32 6, i32 8, i32 8, i32 10, i32 10, i32 12, i32 12, i32 14, i32 14>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovsldup_load(<16 x float> *%p0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovsldup_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    vmovsldup {{.*#+}} zmm0 = mem[0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14]
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 0, i32 0, i32 2, i32 2, i32 4, i32 4, i32 6, i32 6, i32 8, i32 8, i32 10, i32 10, i32 12, i32 12, i32 14, i32 14>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovsldup_mask(<16 x float> %x0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovsldup_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovsldup {{.*#+}} zmm0 {%k1} {z} = zmm0[0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 undef, i32 0, i32 undef, i32 2, i32 4, i32 4, i32 6, i32 6, i32 8, i32 8, i32 10, i32 10, i32 12, i32 12, i32 14, i32 14>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vmovsldup_mask_load(<16 x float> *%p0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vmovsldup_mask_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %esi, %k1
; CHECK-NEXT:    vmovsldup {{.*#+}} zmm0 {%k1} {z} = mem[0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14]
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 undef, i32 0, i32 undef, i32 2, i32 4, i32 4, i32 6, i32 6, i32 8, i32 8, i32 10, i32 10, i32 12, i32 12, i32 14, i32 14>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}

define <16 x float> @combine_vpermt2var_16f32_vpermilps(<16 x float> %x0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vpermilps:
; CHECK:       # BB#0:
; CHECK-NEXT:    vpermilps {{.*#+}} zmm0 = zmm0[3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 3, i32 2, i32 1, i32 0, i32 7, i32 6, i32 5, i32 4, i32 11, i32 10, i32 9, i32 8, i32 15, i32 14, i32 13, i32 12>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vpermilps_load(<16 x float> *%p0, <16 x float> %x1) {
; CHECK-LABEL: combine_vpermt2var_16f32_vpermilps_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    vpermilps {{.*#+}} zmm0 = mem[3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12]
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 3, i32 2, i32 1, i32 0, i32 7, i32 6, i32 5, i32 4, i32 11, i32 10, i32 9, i32 8, i32 15, i32 14, i32 13, i32 12>, <16 x float> %x0, <16 x float> %x1, i16 -1)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vpermilps_mask(<16 x float> %x0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vpermilps_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vpermilps {{.*#+}} zmm0 {%k1} {z} = zmm0[3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12]
; CHECK-NEXT:    retq
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 3, i32 2, i32 1, i32 0, i32 7, i32 6, i32 5, i32 4, i32 11, i32 10, i32 9, i32 8, i32 15, i32 14, i32 13, i32 12>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}
define <16 x float> @combine_vpermt2var_16f32_vpermilps_mask_load(<16 x float> *%p0, <16 x float> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16f32_vpermilps_mask_load:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %esi, %k1
; CHECK-NEXT:    vpermilps {{.*#+}} zmm0 {%k1} {z} = mem[3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12]
; CHECK-NEXT:    retq
  %x0 = load <16 x float>, <16 x float> *%p0
  %res0 = call <16 x float> @llvm.x86.avx512.maskz.vpermt2var.ps.512(<16 x i32> <i32 3, i32 2, i32 1, i32 0, i32 7, i32 6, i32 5, i32 4, i32 11, i32 10, i32 9, i32 8, i32 15, i32 14, i32 13, i32 12>, <16 x float> %x0, <16 x float> %x1, i16 %m)
  ret <16 x float> %res0
}

define <16 x i32> @combine_vpermt2var_16i32_identity(<16 x i32> %x0, <16 x i32> %x1) {
; CHECK-LABEL: combine_vpermt2var_16i32_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <16 x i32> @llvm.x86.avx512.maskz.vpermt2var.d.512(<16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 undef>, <16 x i32> %x0, <16 x i32> %x1, i16 -1)
  %res1 = call <16 x i32> @llvm.x86.avx512.maskz.vpermt2var.d.512(<16 x i32> <i32 15, i32 30, i32 13, i32 28, i32 undef, i32 26, i32 9, i32 24, i32 7, i32 22, i32 5, i32 20, i32 3, i32 18, i32 1, i32 16>, <16 x i32> %res0, <16 x i32> %res0, i16 -1)
  ret <16 x i32> %res1
}
define <16 x i32> @combine_vpermt2var_16i32_identity_mask(<16 x i32> %x0, <16 x i32> %x1, i16 %m) {
; CHECK-LABEL: combine_vpermt2var_16i32_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovw %edi, %k1
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm2 = [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermt2d %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    vmovdqa32 {{.*#+}} zmm1 = [15,30,13,28,11,26,9,24,7,22,5,20,3,18,1,16]
; CHECK-NEXT:    vpermt2d %zmm0, %zmm1, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <16 x i32> @llvm.x86.avx512.maskz.vpermt2var.d.512(<16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>, <16 x i32> %x0, <16 x i32> %x1, i16 %m)
  %res1 = call <16 x i32> @llvm.x86.avx512.maskz.vpermt2var.d.512(<16 x i32> <i32 15, i32 30, i32 13, i32 28, i32 11, i32 26, i32 9, i32 24, i32 7, i32 22, i32 5, i32 20, i32 3, i32 18, i32 1, i32 16>, <16 x i32> %res0, <16 x i32> %res0, i16 %m)
  ret <16 x i32> %res1
}

define <32 x i16> @combine_vpermt2var_32i16_identity(<32 x i16> %x0, <32 x i16> %x1) {
; CHECK-LABEL: combine_vpermt2var_32i16_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %res0 = call <32 x i16> @llvm.x86.avx512.maskz.vpermt2var.hi.512(<32 x i16> <i16 31, i16 30, i16 29, i16 28, i16 27, i16 26, i16 25, i16 24, i16 23, i16 22, i16 21, i16 20, i16 19, i16 18, i16 17, i16 16, i16 15, i16 14, i16 13, i16 12, i16 11, i16 10, i16 9, i16 8, i16 7, i16 6, i16 5, i16 4, i16 3, i16 2, i16 1, i16 0>, <32 x i16> %x0, <32 x i16> %x1, i32 -1)
  %res1 = call <32 x i16> @llvm.x86.avx512.maskz.vpermt2var.hi.512(<32 x i16> <i16 63, i16 30, i16 61, i16 28, i16 59, i16 26, i16 57, i16 24, i16 55, i16 22, i16 53, i16 20, i16 51, i16 18, i16 49, i16 16, i16 47, i16 46, i16 13, i16 44, i16 11, i16 42, i16 9, i16 40, i16 7, i16 38, i16 5, i16 36, i16 3, i16 34, i16 1, i16 32>, <32 x i16> %res0, <32 x i16> %res0, i32 -1)
  ret <32 x i16> %res1
}
define <32 x i16> @combine_vpermt2var_32i16_identity_mask(<32 x i16> %x0, <32 x i16> %x1, i32 %m) {
; CHECK-LABEL: combine_vpermt2var_32i16_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovd %edi, %k1
; CHECK-NEXT:    vmovdqu16 {{.*#+}} zmm2 = [31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vpermt2w %zmm1, %zmm2, %zmm0 {%k1} {z}
; CHECK-NEXT:    vmovdqu16 {{.*#+}} zmm1 = [63,30,61,28,59,26,57,24,55,22,53,20,51,18,49,16,47,46,13,44,11,42,9,40,7,38,5,36,3,34,1,32]
; CHECK-NEXT:    vpermt2w %zmm0, %zmm1, %zmm0 {%k1} {z}
; CHECK-NEXT:    retq
  %res0 = call <32 x i16> @llvm.x86.avx512.maskz.vpermt2var.hi.512(<32 x i16> <i16 31, i16 30, i16 29, i16 28, i16 27, i16 26, i16 25, i16 24, i16 23, i16 22, i16 21, i16 20, i16 19, i16 18, i16 17, i16 16, i16 15, i16 14, i16 13, i16 12, i16 11, i16 10, i16 9, i16 8, i16 7, i16 6, i16 5, i16 4, i16 3, i16 2, i16 1, i16 0>, <32 x i16> %x0, <32 x i16> %x1, i32 %m)
  %res1 = call <32 x i16> @llvm.x86.avx512.maskz.vpermt2var.hi.512(<32 x i16> <i16 63, i16 30, i16 61, i16 28, i16 59, i16 26, i16 57, i16 24, i16 55, i16 22, i16 53, i16 20, i16 51, i16 18, i16 49, i16 16, i16 47, i16 46, i16 13, i16 44, i16 11, i16 42, i16 9, i16 40, i16 7, i16 38, i16 5, i16 36, i16 3, i16 34, i16 1, i16 32>, <32 x i16> %res0, <32 x i16> %res0, i32 %m)
  ret <32 x i16> %res1
}

define <64 x i8> @combine_pshufb_identity(<64 x i8> %x0) {
; CHECK-LABEL: combine_pshufb_identity:
; CHECK:       # BB#0:
; CHECK-NEXT:    retq
  %select = bitcast <8 x i64> <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1> to <64 x i8>
  %mask = bitcast <16 x i32> <i32 202182159, i32 134810123, i32 67438087, i32 66051, i32 202182159, i32 undef, i32 67438087, i32 66051, i32 202182159, i32 134810123, i32 67438087, i32 66051, i32 202182159, i32 134810123, i32 67438087, i32 66051> to <64 x i8>
  %res0 = call <64 x i8> @llvm.x86.avx512.mask.pshuf.b.512(<64 x i8> %x0, <64 x i8> %mask, <64 x i8> %select, i64 -1)
  %res1 = call <64 x i8> @llvm.x86.avx512.mask.pshuf.b.512(<64 x i8> %res0, <64 x i8> %mask, <64 x i8> %select, i64 -1)
  ret <64 x i8> %res1
}
define <64 x i8> @combine_pshufb_identity_mask(<64 x i8> %x0, i64 %m) {
; CHECK-LABEL: combine_pshufb_identity_mask:
; CHECK:       # BB#0:
; CHECK-NEXT:    kmovq %rdi, %k1
; CHECK-NEXT:    vmovdqu8 {{.*#+}} zmm1 = [255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255]
; CHECK-NEXT:    vmovdqu8 {{.*#+}} zmm2 = [15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0]
; CHECK-NEXT:    vmovaps %zmm1, %zmm3
; CHECK-NEXT:    vpshufb %zmm2, %zmm0, %zmm3 {%k1}
; CHECK-NEXT:    vpshufb %zmm2, %zmm3, %zmm1 {%k1}
; CHECK-NEXT:    vmovaps %zmm1, %zmm0
; CHECK-NEXT:    retq
  %select = bitcast <8 x i64> <i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1, i64 -1> to <64 x i8>
  %mask = bitcast <16 x i32> <i32 202182159, i32 134810123, i32 67438087, i32 66051, i32 202182159, i32 134810123, i32 67438087, i32 66051, i32 202182159, i32 134810123, i32 67438087, i32 66051, i32 202182159, i32 134810123, i32 67438087, i32 66051> to <64 x i8>
  %res0 = call <64 x i8> @llvm.x86.avx512.mask.pshuf.b.512(<64 x i8> %x0, <64 x i8> %mask, <64 x i8> %select, i64 %m)
  %res1 = call <64 x i8> @llvm.x86.avx512.mask.pshuf.b.512(<64 x i8> %res0, <64 x i8> %mask, <64 x i8> %select, i64 %m)
  ret <64 x i8> %res1
}

define <32 x i16> @combine_permvar_as_vpbroadcastw512(<32 x i16> %x0) {
; CHECK-LABEL: combine_permvar_as_vpbroadcastw512:
; CHECK:       # BB#0:
; CHECK-NEXT:    vpbroadcastw %xmm0, %zmm0
; CHECK-NEXT:    retq
  %1 = call <32 x i16> @llvm.x86.avx512.mask.permvar.hi.512(<32 x i16> %x0, <32 x i16> zeroinitializer, <32 x i16> undef, i32 -1)
  ret <32 x i16> %1
}

define <16 x i32> @combine_permvar_as_vpbroadcastd512(<16 x i32> %x0) {
; CHECK-LABEL: combine_permvar_as_vpbroadcastd512:
; CHECK:       # BB#0:
; CHECK-NEXT:    vpbroadcastd %xmm0, %zmm0
; CHECK-NEXT:    retq
  %1 = call <16 x i32> @llvm.x86.avx512.mask.permvar.si.512(<16 x i32> %x0, <16 x i32> zeroinitializer, <16 x i32> undef, i16 -1)
  ret <16 x i32> %1
}

define <8 x i64> @combine_permvar_as_vpbroadcastq512(<8 x i64> %x0) {
; CHECK-LABEL: combine_permvar_as_vpbroadcastq512:
; CHECK:       # BB#0:
; CHECK-NEXT:    vpxord %zmm1, %zmm1, %zmm1
; CHECK-NEXT:    vpermq %zmm0, %zmm1, %zmm0
; CHECK-NEXT:    retq
  %1 = call <8 x i64> @llvm.x86.avx512.mask.permvar.di.512(<8 x i64> %x0, <8 x i64> zeroinitializer, <8 x i64> undef, i8 -1)
  ret <8 x i64> %1
}
