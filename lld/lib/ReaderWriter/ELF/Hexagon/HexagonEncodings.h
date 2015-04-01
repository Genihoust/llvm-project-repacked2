//===- lib/ReaderWriter/ELF/Hexagon/HexagonEncodings.h -------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
Instruction insn_encodings[] = {
  // InsnMask   CompareMask BitMask IsDuplexInstruction
  { 0xffe00004, 0x40000000, 0x20f8, 0x0 },
  { 0xffe03080, 0x9ca03080, 0xf60, 0x0 },
  { 0xf9e00000, 0x48c00000, 0x61f20ff, 0x0 },
  { 0xf7c02300, 0x13802100, 0x3000fe, 0x0 },
  { 0xffe00000, 0x60c00000, 0x1f18, 0x0 },
  { 0xffe00000, 0x69c00000, 0x1f18, 0x0 },
  { 0xffe02000, 0x43000000, 0x7e0, 0x0 },
  { 0xff602060, 0x3e000060, 0x1f80, 0x0 },
  { 0xffe03000, 0x9ae01000, 0xf60, 0x0 },
  { 0xf9e00000, 0x91600000, 0x6003fe0, 0x0 },
  { 0xffe02084, 0xaf000084, 0x30078, 0x0 },
  { 0xff602060, 0x3e000020, 0x1f80, 0x0 },
  { 0xff602060, 0x3e200040, 0x1f80, 0x0 },
  { 0xf7c02000, 0x10c02000, 0x3000fe, 0x0 },
  { 0xffe00000, 0x60200000, 0x1f18, 0x0 },
  { 0xffe00000, 0x69200000, 0x1f18, 0x0 },
  { 0xffe038c0, 0xada00880, 0x3f, 0x0 },
  { 0xff602000, 0x73002000, 0x1fe0, 0x0 },
  { 0xf7c02000, 0x26c02000, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9f403880, 0x1f0100, 0x0 },
  { 0xf9e00000, 0x48400000, 0x61f20ff, 0x0 },
  { 0xffe02000, 0x41600000, 0x7e0, 0x0 },
  { 0xffe02084, 0xaf000080, 0x30078, 0x0 },
  { 0xf7c02300, 0x13800100, 0x3000fe, 0x0 },
  { 0xffe01804, 0x46a00000, 0x20f8, 0x0 },
  { 0xffe00004, 0x42400000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x22400000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x12402000, 0x3000fe, 0x0 },
  { 0xfc003d18, 0x28003c18, 0x3f00000, 0x1 },
  { 0xffe00000, 0x39000000, 0x201f, 0x0 },
  { 0xff601018, 0xdd400008, 0xfe0, 0x0 },
  { 0xffc0001c, 0x75400000, 0x203fe0, 0x0 },
  { 0xfc003fc7, 0x48003f47, 0x3f00000, 0x1 },
  { 0xffe03080, 0x9ca03000, 0xf60, 0x0 },
  { 0xf9e00000, 0x90800000, 0x6003fe0, 0x0 },
  { 0xf8003fc7, 0x40003fc4, 0x7f00000, 0x1 },
  { 0xfc003e00, 0x68003c00, 0x3f00000, 0x1 },
  { 0xf8003fc7, 0x40003fc5, 0x7f00000, 0x1 },
  { 0xf9e00000, 0x91800000, 0x6003fe0, 0x0 },
  { 0xff602060, 0x3e400060, 0x1f80, 0x0 },
  { 0xff602060, 0x3e000000, 0x1f80, 0x0 },
  { 0xf8003d18, 0x20003c18, 0x7f00000, 0x1 },
  { 0xf8003f00, 0x20003800, 0x7f00000, 0x1 },
  { 0xf8003d18, 0x20003c10, 0x7f00000, 0x1 },
  { 0xff602000, 0x73602000, 0x1fe0, 0x0 },
  { 0xffe03880, 0x9f002080, 0x1f0100, 0x0 },
  { 0xffe02000, 0x47000000, 0x7e0, 0x0 },
  { 0xf9e00000, 0x91400000, 0x6003fe0, 0x0 },
  { 0xffe02080, 0xabc00080, 0x3f, 0x0 },
  { 0xf7c02000, 0x20802000, 0x3000fe, 0x0 },
  { 0xf8003fc7, 0x40003f44, 0x7f00000, 0x1 },
  { 0xffe03884, 0xafa03084, 0x30078, 0x0 },
  { 0xffe03000, 0x9b001000, 0xf60, 0x0 },
  { 0xffe01804, 0x42a00800, 0x20f8, 0x0 },
  { 0xfc003f00, 0x28003100, 0x3f00000, 0x1 },
  { 0xffe02080, 0xab800080, 0x3f, 0x0 },
  { 0xf7c02000, 0x24c00000, 0x3000fe, 0x0 },
  { 0xffe00000, 0x39a00000, 0x201f, 0x0 },
  { 0xf7c02300, 0x13802300, 0x3000fe, 0x0 },
  { 0xffe01804, 0x46a00800, 0x20f8, 0x0 },
  { 0xffe020c0, 0xad602080, 0x3f, 0x0 },
  { 0xfc003f00, 0x28003500, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x28003400, 0x3f00000, 0x1 },
  { 0xffe020c0, 0xad6000c0, 0x3f, 0x0 },
  { 0xffe00000, 0x60000000, 0x1f18, 0x0 },
  { 0xf8003000, 0x40000000, 0x7f00000, 0x1 },
  { 0xffe00000, 0x69000000, 0x1f18, 0x0 },
  { 0xffe03080, 0x9c601080, 0xf60, 0x0 },
  { 0xffe03080, 0x9ce01000, 0xf60, 0x0 },
  { 0xffe03080, 0x9c601000, 0xf60, 0x0 },
  { 0xf7c02000, 0x13402000, 0x3000fe, 0x0 },
  { 0xffe03080, 0x9c603000, 0xf60, 0x0 },
  { 0xf7c02000, 0x21c00000, 0x3000fe, 0x0 },
  { 0xfc003000, 0x68000000, 0x3f00000, 0x1 },
  { 0xf8003800, 0x60002000, 0x7f00000, 0x1 },
  { 0xffe02084, 0xaf802084, 0x30078, 0x0 },
  { 0xfc003000, 0x48000000, 0x3f00000, 0x1 },
  { 0xf7c02300, 0x11c02100, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x12800000, 0x3000fe, 0x0 },
  { 0xfc003e70, 0x28003a40, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x28003300, 0x3f00000, 0x1 },
  { 0xff800000, 0xe0000000, 0x1fe0, 0x0 },
  { 0xff602060, 0x3f400000, 0x1f80, 0x0 },
  { 0xffe00004, 0x42000000, 0x20f8, 0x0 },
  { 0xf8003f00, 0x60003300, 0x7f00000, 0x1 },
  { 0xffe01804, 0x42a00000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x12c00000, 0x3000fe, 0x0 },
  { 0xf0000000, 0x0, 0xfff3fff, 0x0 },
  { 0xff000016, 0xde000016, 0xe020e8, 0x0 },
  { 0xffe03000, 0x9b201000, 0xf60, 0x0 },
  { 0xffe03880, 0xaba00880, 0x3f, 0x0 },
  { 0xf8003e00, 0x40003c00, 0x7f00000, 0x1 },
  { 0xff602060, 0x3f200040, 0x1f80, 0x0 },
  { 0xffe03880, 0x9f203880, 0x1f0100, 0x0 },
  { 0xf7c02000, 0x20c00000, 0x3000fe, 0x0 },
  { 0xf9e01800, 0x48a00800, 0x61f20ff, 0x0 },
  { 0xf9e00000, 0x90a00000, 0x6003fe0, 0x0 },
  { 0xff802000, 0x74802000, 0x1fe0, 0x0 },
  { 0xffe03000, 0x9a401000, 0xf60, 0x0 },
  { 0xf7c02000, 0x10002000, 0x3000fe, 0x0 },
  { 0xf7c03000, 0x14803000, 0x3000fe, 0x0 },
  { 0xffe020c0, 0xad0020c0, 0x3f, 0x0 },
  { 0xffe0001c, 0x75800000, 0x3fe0, 0x0 },
  { 0xf9e01800, 0x48a01000, 0x61f20ff, 0x0 },
  { 0xffe03080, 0x9dc03000, 0xf60, 0x0 },
  { 0xffe03080, 0x9dc03080, 0xf60, 0x0 },
  { 0xffe03080, 0x9dc01000, 0xf60, 0x0 },
  { 0xffe03080, 0x9dc01080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d601000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d601080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d603000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d603080, 0xf60, 0x0 },
  { 0xfc003e00, 0x48003c00, 0x3f00000, 0x1 },
  { 0xffe02084, 0xaf402084, 0x30078, 0x0 },
  { 0xffe00004, 0x46600000, 0x20f8, 0x0 },
  { 0xffe03880, 0x9f203080, 0x1f0100, 0x0 },
  { 0xf8003f00, 0x20003100, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x11402000, 0x3000fe, 0x0 },
  { 0xf8003d08, 0x20003d00, 0x7f00000, 0x1 },
  { 0xffe03080, 0x9ca01080, 0xf60, 0x0 },
  { 0xffe03080, 0x9ca01000, 0xf60, 0x0 },
  { 0xffe00000, 0x38a00000, 0x201f, 0x0 },
  { 0xf7c02300, 0x11800000, 0x3000fe, 0x0 },
  { 0xf7c02300, 0x13c02300, 0x3000fe, 0x0 },
  { 0xffe03080, 0x9ce03000, 0xf60, 0x0 },
  { 0xf9e00000, 0x90e00000, 0x6003fe0, 0x0 },
  { 0xffe02084, 0xaf400080, 0x30078, 0x0 },
  { 0xffe03080, 0x9ce03080, 0xf60, 0x0 },
  { 0xff000000, 0x78000000, 0xdf3fe0, 0x0 },
  { 0xffe03080, 0x9ce01080, 0xf60, 0x0 },
  { 0xffe03880, 0xaba01080, 0x3f, 0x0 },
  { 0xffe020c0, 0xad002080, 0x3f, 0x0 },
  { 0xffe020c0, 0xad0000c0, 0x3f, 0x0 },
  { 0xffe020c0, 0xad000080, 0x3f, 0x0 },
  { 0xf7c02000, 0x25000000, 0x3000fe, 0x0 },
  { 0xff602060, 0x3f200020, 0x1f80, 0x0 },
  { 0xffe02084, 0xafc00084, 0x30078, 0x0 },
  { 0xf7c02000, 0x24400000, 0x3000fe, 0x0 },
  { 0xfc003000, 0x48001000, 0x3f00000, 0x1 },
  { 0xf9e01800, 0xa1a01000, 0x60020ff, 0x0 },
  { 0xff602060, 0x3f000040, 0x1f80, 0x0 },
  { 0xffe02084, 0xaf602084, 0x30078, 0x0 },
  { 0xf8003f00, 0x20003400, 0x7f00000, 0x1 },
  { 0xffe02084, 0xaf400084, 0x30078, 0x0 },
  { 0xffe01804, 0x44a01000, 0x20f8, 0x0 },
  { 0xff602060, 0x3e200000, 0x1f80, 0x0 },
  { 0xf8003e70, 0x20003a70, 0x7f00000, 0x1 },
  { 0xf8003f00, 0x40003e00, 0x7f00000, 0x1 },
  { 0xf8003f00, 0x20003300, 0x7f00000, 0x1 },
  { 0xf7c02300, 0x13800300, 0x3000fe, 0x0 },
  { 0xffe038c0, 0xada00080, 0x3f, 0x0 },
  { 0xf9e00000, 0x49400000, 0x61f3fe0, 0x0 },
  { 0xf8003800, 0x40002800, 0x7f00000, 0x1 },
  { 0xffe038c0, 0xada020c0, 0x3f, 0x0 },
  { 0xffe03884, 0xafa00880, 0x30078, 0x0 },
  { 0xf9e00000, 0x49000000, 0x61f3fe0, 0x0 },
  { 0xff800000, 0xd7000000, 0x6020e0, 0x0 },
  { 0xffc00000, 0xda000000, 0x203fe0, 0x0 },
  { 0xf7c02000, 0x12802000, 0x3000fe, 0x0 },
  { 0xf9e00000, 0x49600000, 0x61f3fe0, 0x0 },
  { 0xffe02000, 0x47400000, 0x7e0, 0x0 },
  { 0xf9e00000, 0x49c00000, 0x61f3fe0, 0x0 },
  { 0xffe03000, 0x9bc01000, 0xf60, 0x0 },
  { 0xf7c02300, 0x13c00100, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9f002880, 0x1f0100, 0x0 },
  { 0xffe03000, 0x9b601000, 0xf60, 0x0 },
  { 0xffe01804, 0x40a00800, 0x20f8, 0x0 },
  { 0xffe00004, 0x42800000, 0x20f8, 0x0 },
  { 0xf7c03000, 0x14800000, 0x3000fe, 0x0 },
  { 0xfc003000, 0x68001000, 0x3f00000, 0x1 },
  { 0xfc003fc7, 0x48003f44, 0x3f00000, 0x1 },
  { 0xfc003fc7, 0x48003f45, 0x3f00000, 0x1 },
  { 0xf7c02000, 0x10800000, 0x3000fe, 0x0 },
  { 0xf8003e70, 0x20003a50, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x21002000, 0x3000fe, 0x0 },
  { 0xf8003fc4, 0x40003fc0, 0x7f00000, 0x1 },
  { 0xf9e00000, 0x48000000, 0x61f20ff, 0x0 },
  { 0xffc0001c, 0x75000010, 0x203fe0, 0x0 },
  { 0xf8003f00, 0x20003800, 0x7f00000, 0x1 },
  { 0xf9e00000, 0xa1800000, 0x60020ff, 0x0 },
  { 0xffc01000, 0x61c00000, 0x202ffe, 0x0 },
  { 0xffe02084, 0xaf402080, 0x30078, 0x0 },
  { 0xffe03880, 0x9f602880, 0x1f0100, 0x0 },
  { 0xfc003f00, 0x68003000, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x68003100, 0x3f00000, 0x1 },
  { 0xff602060, 0x3f200000, 0x1f80, 0x0 },
  { 0xffe03000, 0x9a801000, 0xf60, 0x0 },
  { 0xf7c02000, 0x24802000, 0x3000fe, 0x0 },
  { 0xffe00004, 0x42c00000, 0x20f8, 0x0 },
  { 0xf7c02300, 0x11802000, 0x3000fe, 0x0 },
  { 0xffc01000, 0x61401000, 0x202ffe, 0x0 },
  { 0xffe02000, 0x43c00000, 0x7e0, 0x0 },
  { 0xf7c02000, 0x11400000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x21800000, 0x3000fe, 0x0 },
  { 0xfc003c00, 0x28002c00, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x28003200, 0x3f00000, 0x1 },
  { 0xffe03080, 0x9c803080, 0xf60, 0x0 },
  { 0xf7c03000, 0x14c03000, 0x3000fe, 0x0 },
  { 0xff800000, 0xdb800000, 0x6020e0, 0x0 },
  { 0xf7c02000, 0x22402000, 0x3000fe, 0x0 },
  { 0xffe00004, 0x46800000, 0x20f8, 0x0 },
  { 0xffe00000, 0x69a00000, 0x1f18, 0x0 },
  { 0xfc003e00, 0x68002a00, 0x3f00000, 0x1 },
  { 0xffe00000, 0x60a00000, 0x1f18, 0x0 },
  { 0xf7c02000, 0x25400000, 0x3000fe, 0x0 },
  { 0xfc003e70, 0x28003a70, 0x3f00000, 0x1 },
  { 0xffe03080, 0x9c803000, 0xf60, 0x0 },
  { 0xffc01000, 0x61400000, 0x202ffe, 0x0 },
  { 0xffe01804, 0x42a01000, 0x20f8, 0x0 },
  { 0xffc0001c, 0x75000000, 0x203fe0, 0x0 },
  { 0xffe02084, 0xafc02080, 0x30078, 0x0 },
  { 0xffe03884, 0xafa00884, 0x30078, 0x0 },
  { 0xffe03884, 0xafa02080, 0x30078, 0x0 },
  { 0xffe00000, 0x38c00000, 0x201f, 0x0 },
  { 0xffc01000, 0x61001000, 0x202ffe, 0x0 },
  { 0xf9e00000, 0x48800000, 0x61f20ff, 0x0 },
  { 0xf8003800, 0x40003000, 0x7f00000, 0x1 },
  { 0xf7c03000, 0x15403000, 0x3000fe, 0x0 },
  { 0xf7c03000, 0x15400000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x21000000, 0x3000fe, 0x0 },
  { 0xffe00004, 0x40c00000, 0x20f8, 0x0 },
  { 0xffe01804, 0x46a01000, 0x20f8, 0x0 },
  { 0xf8003d08, 0x20003d08, 0x7f00000, 0x1 },
  { 0xffe038c0, 0xada02080, 0x3f, 0x0 },
  { 0xffe03080, 0x9c203000, 0xf60, 0x0 },
  { 0xfc003800, 0x68002000, 0x3f00000, 0x1 },
  { 0xf9e00000, 0x90600000, 0x6003fe0, 0x0 },
  { 0xf7c03000, 0x14000000, 0x3000fe, 0x0 },
  { 0xf8003e70, 0x20003a40, 0x7f00000, 0x1 },
  { 0xff201800, 0x5c000800, 0xdf20fe, 0x0 },
  { 0xffe02000, 0x41800000, 0x7e0, 0x0 },
  { 0xff800000, 0xdb000000, 0x6020e0, 0x0 },
  { 0xfc003f00, 0x48003e00, 0x3f00000, 0x1 },
  { 0xf7c03000, 0x14002000, 0x3000fe, 0x0 },
  { 0xf7c02300, 0x11800100, 0x3000fe, 0x0 },
  { 0xfc003e00, 0x68002800, 0x3f00000, 0x1 },
  { 0xffe00004, 0x44c00000, 0x20f8, 0x0 },
  { 0xffe03880, 0x9f003880, 0x1f0100, 0x0 },
  { 0xff602000, 0x73402000, 0x1fe0, 0x0 },
  { 0xffe00000, 0x38200000, 0x201f, 0x0 },
  { 0xf7c02000, 0x24800000, 0x3000fe, 0x0 },
  { 0xf7c03000, 0x15001000, 0x3000fe, 0x0 },
  { 0xff800000, 0x7c800000, 0x1f2000, 0x0 },
  { 0xf8003fc7, 0x40003fc6, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x12000000, 0x3000fe, 0x0 },
  { 0xff602000, 0x73202000, 0x1fe0, 0x0 },
  { 0xf7c02300, 0x13c00000, 0x3000fe, 0x0 },
  { 0xff602060, 0x3f400040, 0x1f80, 0x0 },
  { 0xf7c02000, 0x24002000, 0x3000fe, 0x0 },
  { 0xffe02084, 0xaf800080, 0x30078, 0x0 },
  { 0xffe00000, 0x38800000, 0x201f, 0x0 },
  { 0xfc003f00, 0x28003800, 0x3f00000, 0x1 },
  { 0xffe03080, 0x9c801080, 0xf60, 0x0 },
  { 0xffe020c0, 0xad4000c0, 0x3f, 0x0 },
  { 0xffe00000, 0x39400000, 0x201f, 0x0 },
  { 0xf7c02300, 0x13c02100, 0x3000fe, 0x0 },
  { 0xffe020c0, 0xad400080, 0x3f, 0x0 },
  { 0xffe03880, 0x9f603880, 0x1f0100, 0x0 },
  { 0xff000016, 0xde000002, 0xe020e8, 0x0 },
  { 0xfc003d08, 0x28003d00, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x28003000, 0x3f00000, 0x1 },
  { 0xffe03080, 0x9c401000, 0xf60, 0x0 },
  { 0xf7c02000, 0x21402000, 0x3000fe, 0x0 },
  { 0xff201800, 0x5c200800, 0xdf20fe, 0x0 },
  { 0xffe01804, 0x40a01000, 0x20f8, 0x0 },
  { 0xfc003f00, 0x68003300, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x68003200, 0x3f00000, 0x1 },
  { 0xf7c03000, 0x15401000, 0x3000fe, 0x0 },
  { 0xffe01804, 0x44a00800, 0x20f8, 0x0 },
  { 0xf7c02000, 0x26000000, 0x3000fe, 0x0 },
  { 0xffc00000, 0xda400000, 0x203fe0, 0x0 },
  { 0xffe00004, 0x40600000, 0x20f8, 0x0 },
  { 0xffe02080, 0xab600080, 0x3f, 0x0 },
  { 0xf8003f00, 0x20003600, 0x7f00000, 0x1 },
  { 0xf7c02300, 0x11c00300, 0x3000fe, 0x0 },
  { 0xf8003f00, 0x20003700, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x25c00000, 0x3000fe, 0x0 },
  { 0xf7c02300, 0x11800300, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9f802880, 0x1f0100, 0x0 },
  { 0xfc003800, 0x48003000, 0x3f00000, 0x1 },
  { 0xf8003c00, 0x20002c00, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x10400000, 0x3000fe, 0x0 },
  { 0xff602060, 0x3f400060, 0x1f80, 0x0 },
  { 0xffe03080, 0x9c801000, 0xf60, 0x0 },
  { 0xff602060, 0x3e400040, 0x1f80, 0x0 },
  { 0xf7c03000, 0x14402000, 0x3000fe, 0x0 },
  { 0xffe0001c, 0x75800010, 0x3fe0, 0x0 },
  { 0xff000016, 0xde000014, 0xe020e8, 0x0 },
  { 0xf7c02300, 0x11c02000, 0x3000fe, 0x0 },
  { 0xff600018, 0xdd200008, 0x1fe0, 0x0 },
  { 0xff602060, 0x3e200060, 0x1f80, 0x0 },
  { 0xff000016, 0xde000006, 0xe020e8, 0x0 },
  { 0xffe00004, 0x44600000, 0x20f8, 0x0 },
  { 0xf8003e00, 0x60002800, 0x7f00000, 0x1 },
  { 0xfe600000, 0x3c000000, 0x207f, 0x0 },
  { 0xffe03884, 0xafa02884, 0x30078, 0x0 },
  { 0xf7c02300, 0x11802300, 0x3000fe, 0x0 },
  { 0xffe00000, 0x38000000, 0x201f, 0x0 },
  { 0xff200800, 0x5c000000, 0xdf20fe, 0x0 },
  { 0xf7c02000, 0x13400000, 0x3000fe, 0x0 },
  { 0xff200800, 0x5c200000, 0xdf20fe, 0x0 },
  { 0xffe02000, 0x41000000, 0x7e0, 0x0 },
  { 0xffe03880, 0x9fc02880, 0x1f0100, 0x0 },
  { 0xffe00004, 0x46000000, 0x20f8, 0x0 },
  { 0xff602060, 0x3f000020, 0x1f80, 0x0 },
  { 0xfc003d08, 0x28003d08, 0x3f00000, 0x1 },
  { 0xff602060, 0x3f200060, 0x1f80, 0x0 },
  { 0xffe038c0, 0xada028c0, 0x3f, 0x0 },
  { 0xffe038c0, 0xada008c0, 0x3f, 0x0 },
  { 0xf8003f00, 0x20003500, 0x7f00000, 0x1 },
  { 0xfc003fc4, 0x48003f40, 0x3f00000, 0x1 },
  { 0xf9e01800, 0x48a00000, 0x61f20ff, 0x0 },
  { 0xf7c03000, 0x14802000, 0x3000fe, 0x0 },
  { 0xfc003f00, 0x28003900, 0x3f00000, 0x1 },
  { 0xf8003fc7, 0x40003fc7, 0x7f00000, 0x1 },
  { 0xffe02000, 0x45400000, 0x7e0, 0x0 },
  { 0xffe038c0, 0xada02880, 0x3f, 0x0 },
  { 0xffe02084, 0xaf002080, 0x30078, 0x0 },
  { 0xffe03880, 0x9f803880, 0x1f0100, 0x0 },
  { 0xf7c03000, 0x15000000, 0x3000fe, 0x0 },
  { 0xfc003f00, 0x28003700, 0x3f00000, 0x1 },
  { 0xfc003f00, 0x28003600, 0x3f00000, 0x1 },
  { 0xffe02000, 0x47200000, 0x7e0, 0x0 },
  { 0xffe03880, 0xaba00080, 0x3f, 0x0 },
  { 0xffe02084, 0xafc00080, 0x30078, 0x0 },
  { 0xff802000, 0x73800000, 0x1fe0, 0x0 },
  { 0xffe03880, 0x9f202880, 0x1f0100, 0x0 },
  { 0xf8003d18, 0x20003c00, 0x7f00000, 0x1 },
  { 0xf9e00000, 0xa1600000, 0x60020ff, 0x0 },
  { 0xffe00004, 0x44800000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x21802000, 0x3000fe, 0x0 },
  { 0xff000000, 0xd8000000, 0x6020e0, 0x0 },
  { 0xf9e00000, 0xa1000000, 0x60020ff, 0x0 },
  { 0xffe03884, 0xafa00084, 0x30078, 0x0 },
  { 0xff201800, 0x5c201800, 0xdf20fe, 0x0 },
  { 0xff000016, 0xde000010, 0xe020e8, 0x0 },
  { 0xffe03880, 0x9f603080, 0x1f0100, 0x0 },
  { 0xffe02000, 0x41c00000, 0x7e0, 0x0 },
  { 0xf7c02000, 0x20402000, 0x3000fe, 0x0 },
  { 0xff800000, 0xe1000000, 0x1fe0, 0x0 },
  { 0xf9e00000, 0xa1400000, 0x60020ff, 0x0 },
  { 0xf7c03000, 0x14c00000, 0x3000fe, 0x0 },
  { 0xf8003fc7, 0x40003f47, 0x7f00000, 0x1 },
  { 0xffe00004, 0x40800000, 0x20f8, 0x0 },
  { 0xff800000, 0xe1800000, 0x1fe0, 0x0 },
  { 0xf7c02300, 0x11802100, 0x3000fe, 0x0 },
  { 0xf9e00000, 0x49800000, 0x61f3fe0, 0x0 },
  { 0xf7c02000, 0x26400000, 0x3000fe, 0x0 },
  { 0xf8003c00, 0x20002800, 0x7f00000, 0x1 },
  { 0xff902000, 0x7e002000, 0xf1fe0, 0x0 },
  { 0xff902000, 0x7e802000, 0xf1fe0, 0x0 },
  { 0xf9e00000, 0x91c00000, 0x6003fe0, 0x0 },
  { 0xffe03884, 0xafa02880, 0x30078, 0x0 },
  { 0xf7c02000, 0x22000000, 0x3000fe, 0x0 },
  { 0xffe03080, 0x9d203000, 0xf60, 0x0 },
  { 0xf7c02000, 0x26002000, 0x3000fe, 0x0 },
  { 0xff800000, 0xe2000000, 0x1fe0, 0x0 },
  { 0xf7c02000, 0x26c00000, 0x3000fe, 0x0 },
  { 0xff602060, 0x3e400000, 0x1f80, 0x0 },
  { 0xffe00000, 0x38400000, 0x201f, 0x0 },
  { 0xfc003800, 0x48002000, 0x3f00000, 0x1 },
  { 0xff000016, 0xde000000, 0xe020e8, 0x0 },
  { 0xf8003f00, 0x20003000, 0x7f00000, 0x1 },
  { 0xf8003e70, 0x20003a60, 0x7f00000, 0x1 },
  { 0xff902000, 0x7e800000, 0xf1fe0, 0x0 },
  { 0xffe020c0, 0xad6020c0, 0x3f, 0x0 },
  { 0xf7c02300, 0x13802000, 0x3000fe, 0x0 },
  { 0xffe020c0, 0xad600080, 0x3f, 0x0 },
  { 0xff902000, 0x7e000000, 0xf1fe0, 0x0 },
  { 0xf7000000, 0x17000000, 0x3000fe, 0x0 },
  { 0xf7000000, 0x16000000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x25002000, 0x3000fe, 0x0 },
  { 0xfc003fc7, 0x48003fc7, 0x3f00000, 0x1 },
  { 0xffc01000, 0x61801000, 0x202ffe, 0x0 },
  { 0xffe03884, 0xafa03080, 0x30078, 0x0 },
  { 0xf8003fc4, 0x40003f40, 0x7f00000, 0x1 },
  { 0xfc003e70, 0x28003a60, 0x3f00000, 0x1 },
  { 0xf7c02300, 0x13800000, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9f802080, 0x1f0100, 0x0 },
  { 0xf0000000, 0xb0000000, 0xfe03fe0, 0x0 },
  { 0xffe03880, 0x9f402080, 0x1f0100, 0x0 },
  { 0xffe02000, 0x43200000, 0x7e0, 0x0 },
  { 0xffe00000, 0x39800000, 0x201f, 0x0 },
  { 0xffe03880, 0x9fc03880, 0x1f0100, 0x0 },
  { 0xffe02000, 0x45600000, 0x7e0, 0x0 },
  { 0xf9e00000, 0x91200000, 0x6003fe0, 0x0 },
  { 0xffe02000, 0x43600000, 0x7e0, 0x0 },
  { 0xfc003f00, 0x28003800, 0x3f00000, 0x1 },
  { 0xff802000, 0x74000000, 0x1fe0, 0x0 },
  { 0xffe02084, 0xaf002084, 0x30078, 0x0 },
  { 0xff802000, 0x74800000, 0x1fe0, 0x0 },
  { 0xf7c03000, 0x14c02000, 0x3000fe, 0x0 },
  { 0xfe000001, 0x5a000000, 0x1ff3ffe, 0x0 },
  { 0xff602060, 0x3f400020, 0x1f80, 0x0 },
  { 0xf7c02000, 0x10802000, 0x3000fe, 0x0 },
  { 0xffe02084, 0xaf802080, 0x30078, 0x0 },
  { 0xffe00004, 0x46400000, 0x20f8, 0x0 },
  { 0xffe020c0, 0xad800080, 0x3f, 0x0 },
  { 0xffe020c0, 0xad8000c0, 0x3f, 0x0 },
  { 0xf8003fc7, 0x40003f45, 0x7f00000, 0x1 },
  { 0xf8003e00, 0x60002a00, 0x7f00000, 0x1 },
  { 0xffe02084, 0xaf600084, 0x30078, 0x0 },
  { 0xffe03080, 0x9c201000, 0xf60, 0x0 },
  { 0xffe02000, 0x43400000, 0x7e0, 0x0 },
  { 0xffe03080, 0x9c203080, 0xf60, 0x0 },
  { 0xffe02000, 0x41200000, 0x7e0, 0x0 },
  { 0xffe03080, 0x9c201080, 0xf60, 0x0 },
  { 0xf7c02300, 0x11c02300, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9fc03080, 0x1f0100, 0x0 },
  { 0xffe03880, 0x9f402880, 0x1f0100, 0x0 },
  { 0xf8003800, 0x40002000, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x24402000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x20c02000, 0x3000fe, 0x0 },
  { 0xf7c02300, 0x11c00000, 0x3000fe, 0x0 },
  { 0xffe02000, 0x45200000, 0x7e0, 0x0 },
  { 0xf8003f00, 0x20003900, 0x7f00000, 0x1 },
  { 0xf7c02300, 0x11c00100, 0x3000fe, 0x0 },
  { 0xffe02084, 0xaf800084, 0x30078, 0x0 },
  { 0xfe600000, 0x3c200000, 0x207f, 0x0 },
  { 0xf7c02000, 0x26800000, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9f003080, 0x1f0100, 0x0 },
  { 0xffe03884, 0xafa01084, 0x30078, 0x0 },
  { 0xffc00000, 0x76000000, 0x203fe0, 0x0 },
  { 0xff602060, 0x3e000040, 0x1f80, 0x0 },
  { 0xffe020c0, 0xadc020c0, 0x3f, 0x0 },
  { 0xffe00004, 0x44400000, 0x20f8, 0x0 },
  { 0xffe020c0, 0xadc02080, 0x3f, 0x0 },
  { 0xfe600000, 0x3c400000, 0x207f, 0x0 },
  { 0xf7c02000, 0x20400000, 0x3000fe, 0x0 },
  { 0xff800000, 0x7c000000, 0x1fe0, 0x0 },
  { 0xffe03884, 0xafa00080, 0x30078, 0x0 },
  { 0xff201800, 0x5c001800, 0xdf20fe, 0x0 },
  { 0xffe02000, 0x47800000, 0x7e0, 0x0 },
  { 0xff601018, 0xdd400000, 0xfe0, 0x0 },
  { 0xffe020c0, 0xad4020c0, 0x3f, 0x0 },
  { 0xffe020c0, 0xad402080, 0x3f, 0x0 },
  { 0xf8003000, 0x40001000, 0x7f00000, 0x1 },
  { 0xffe02084, 0xafc02084, 0x30078, 0x0 },
  { 0xffe03080, 0x9c403080, 0xf60, 0x0 },
  { 0xfc003e40, 0x28003a00, 0x3f00000, 0x1 },
  { 0xffe038c0, 0xada010c0, 0x3f, 0x0 },
  { 0xffe038c0, 0xada01080, 0x3f, 0x0 },
  { 0xffe038c0, 0xada030c0, 0x3f, 0x0 },
  { 0xffe038c0, 0xada03080, 0x3f, 0x0 },
  { 0xf7c02000, 0x20800000, 0x3000fe, 0x0 },
  { 0xfc003fc7, 0x48003f46, 0x3f00000, 0x1 },
  { 0xffe01804, 0x44a00000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x20002000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x12c02000, 0x3000fe, 0x0 },
  { 0xffe03000, 0x9a601000, 0xf60, 0x0 },
  { 0xffc00000, 0xda800000, 0x203fe0, 0x0 },
  { 0xf9e00000, 0x90400000, 0x6003fe0, 0x0 },
  { 0xffe02000, 0x47600000, 0x7e0, 0x0 },
  { 0xffe03080, 0x9d403000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d403080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d401000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d401080, 0xf60, 0x0 },
  { 0xffe02000, 0x41400000, 0x7e0, 0x0 },
  { 0xff800000, 0xdf800000, 0x6020e0, 0x0 },
  { 0xffc01000, 0x61000000, 0x202ffe, 0x0 },
  { 0xffe03880, 0x9f202080, 0x1f0100, 0x0 },
  { 0xfc003fc7, 0x48003fc6, 0x3f00000, 0x1 },
  { 0xfe000000, 0x7a000000, 0x1fe0, 0x0 },
  { 0xffff0000, 0x6a490000, 0x1f80, 0x0 },
  { 0xff802000, 0x73000000, 0x1fe0, 0x0 },
  { 0xff602060, 0x3e200020, 0x1f80, 0x0 },
  { 0xf7c02000, 0x24000000, 0x3000fe, 0x0 },
  { 0xf8003e40, 0x20003a00, 0x7f00000, 0x1 },
  { 0xf7c03000, 0x14401000, 0x3000fe, 0x0 },
  { 0xf8003f00, 0x20003200, 0x7f00000, 0x1 },
  { 0xffc00000, 0x76400000, 0x203fe0, 0x0 },
  { 0xf7c02000, 0x22002000, 0x3000fe, 0x0 },
  { 0xffc01000, 0x61c01000, 0x202ffe, 0x0 },
  { 0xf7c03000, 0x14801000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x12002000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x10402000, 0x3000fe, 0x0 },
  { 0xff201800, 0x5d200000, 0xdf20fe, 0x0 },
  { 0xf7c02000, 0x21400000, 0x3000fe, 0x0 },
  { 0xff201800, 0x5d000000, 0xdf20fe, 0x0 },
  { 0xffe02000, 0x45c00000, 0x7e0, 0x0 },
  { 0xf7c02000, 0x25802000, 0x3000fe, 0x0 },
  { 0xfc003e70, 0x28003a50, 0x3f00000, 0x1 },
  { 0xf7c02300, 0x13c00300, 0x3000fe, 0x0 },
  { 0xf9e01800, 0xa1a00800, 0x60020ff, 0x0 },
  { 0xffe02000, 0x43800000, 0x7e0, 0x0 },
  { 0xfc003fc4, 0x48003fc0, 0x3f00000, 0x1 },
  { 0xff800000, 0xe2800000, 0x1fe0, 0x0 },
  { 0xf7c02300, 0x13c02000, 0x3000fe, 0x0 },
  { 0xffe03080, 0x9d803080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d803000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d801080, 0xf60, 0x0 },
  { 0xf8003fc4, 0x40003f00, 0x7f00000, 0x1 },
  { 0xffe00000, 0x39c00000, 0x201f, 0x0 },
  { 0xffe03080, 0x9d203080, 0xf60, 0x0 },
  { 0xffe02080, 0xab000080, 0x3f, 0x0 },
  { 0xf8003e00, 0x60003c00, 0x7f00000, 0x1 },
  { 0xffe03880, 0x9f602080, 0x1f0100, 0x0 },
  { 0xffc00000, 0x76800000, 0x203fe0, 0x0 },
  { 0xffe03884, 0xafa02084, 0x30078, 0x0 },
  { 0xf7c02000, 0x13002000, 0x3000fe, 0x0 },
  { 0xf9e00000, 0x91000000, 0x6003fe0, 0x0 },
  { 0xffe03080, 0x9d201080, 0xf60, 0x0 },
  { 0xf7c03000, 0x15002000, 0x3000fe, 0x0 },
  { 0xf8003000, 0x60000000, 0x7f00000, 0x1 },
  { 0xffc01000, 0x61800000, 0x202ffe, 0x0 },
  { 0xf7c03000, 0x14400000, 0x3000fe, 0x0 },
  { 0xffe03000, 0x9b401000, 0xf60, 0x0 },
  { 0xf7c03000, 0x14003000, 0x3000fe, 0x0 },
  { 0xffe03880, 0x9fc02080, 0x1f0100, 0x0 },
  { 0xfc003fc4, 0x48003f00, 0x3f00000, 0x1 },
  { 0xffe02000, 0x45000000, 0x7e0, 0x0 },
  { 0xfc003800, 0x48002800, 0x3f00000, 0x1 },
  { 0xfc003fc7, 0x48003fc5, 0x3f00000, 0x1 },
  { 0xfc003d18, 0x28003c00, 0x3f00000, 0x1 },
  { 0xfc003fc7, 0x48003fc4, 0x3f00000, 0x1 },
  { 0xf8003f00, 0x60003200, 0x7f00000, 0x1 },
  { 0xffe02084, 0xaf600080, 0x30078, 0x0 },
  { 0xf9e01800, 0xa1a00000, 0x60020ff, 0x0 },
  { 0xf7c03000, 0x14001000, 0x3000fe, 0x0 },
  { 0xf7c03000, 0x14c01000, 0x3000fe, 0x0 },
  { 0xffe00004, 0x46c00000, 0x20f8, 0x0 },
  { 0xf7c03000, 0x15003000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x10000000, 0x3000fe, 0x0 },
  { 0xf8003d18, 0x20003c08, 0x7f00000, 0x1 },
  { 0xffc0001c, 0x75400010, 0x203fe0, 0x0 },
  { 0xf9e00000, 0x48600000, 0x61f20ff, 0x0 },
  { 0xffe03080, 0x9c603080, 0xf60, 0x0 },
  { 0xfe000000, 0x58000000, 0x1ff3ffe, 0x0 },
  { 0xffe03000, 0x9a201000, 0xf60, 0x0 },
  { 0xffe00000, 0x69e00000, 0x1f18, 0x0 },
  { 0xffe020c0, 0xad802080, 0x3f, 0x0 },
  { 0xffe02000, 0x47c00000, 0x7e0, 0x0 },
  { 0xffe00000, 0x60e00000, 0x1f18, 0x0 },
  { 0xf7c03000, 0x15402000, 0x3000fe, 0x0 },
  { 0xffe020c0, 0xad8020c0, 0x3f, 0x0 },
  { 0xff000016, 0xde000012, 0xe020e8, 0x0 },
  { 0xf7c02000, 0x25c02000, 0x3000fe, 0x0 },
  { 0xf8003f00, 0x60003100, 0x7f00000, 0x1 },
  { 0xf8003f00, 0x60003000, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x25800000, 0x3000fe, 0x0 },
  { 0xf7c03000, 0x14403000, 0x3000fe, 0x0 },
  { 0xfc003d18, 0x28003c08, 0x3f00000, 0x1 },
  { 0xffe03880, 0x9f403080, 0x1f0100, 0x0 },
  { 0xf7c02000, 0x25402000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x10c00000, 0x3000fe, 0x0 },
  { 0xffe02000, 0x45800000, 0x7e0, 0x0 },
  { 0xffe03880, 0x9f803080, 0x1f0100, 0x0 },
  { 0xffe03080, 0x9d001000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d001080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d003000, 0xf60, 0x0 },
  { 0xffe03080, 0x9d003080, 0xf60, 0x0 },
  { 0xffe03080, 0x9d801000, 0xf60, 0x0 },
  { 0xf9e00000, 0x49200000, 0x61f3fe0, 0x0 },
  { 0xf9e00000, 0xa1c00000, 0x60020ff, 0x0 },
  { 0xf9e00000, 0x90200000, 0x6003fe0, 0x0 },
  { 0xffe03080, 0x9d201000, 0xf60, 0x0 },
  { 0xffe03884, 0xafa01080, 0x30078, 0x0 },
  { 0xffe02084, 0xaf602080, 0x30078, 0x0 },
  { 0xffe038c0, 0xada000c0, 0x3f, 0x0 },
  { 0xffe02080, 0xab400080, 0x3f, 0x0 },
  { 0xff000016, 0xde000004, 0xe020e8, 0x0 },
  { 0xffe00004, 0x44000000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x20000000, 0x3000fe, 0x0 },
  { 0xfc003d18, 0x28003c10, 0x3f00000, 0x1 },
  { 0xff600018, 0xdd000008, 0x1fe0, 0x0 },
  { 0xffe020c0, 0xadc000c0, 0x3f, 0x0 },
  { 0xffe020c0, 0xadc00080, 0x3f, 0x0 },
  { 0xffe03000, 0x9b801000, 0xf60, 0x0 },
  { 0xf8003fc7, 0x40003f46, 0x7f00000, 0x1 },
  { 0xf7c02000, 0x21c02000, 0x3000fe, 0x0 },
  { 0xffe01804, 0x40a00000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x26402000, 0x3000fe, 0x0 },
  { 0xffe03080, 0x9c401080, 0xf60, 0x0 },
  { 0xffe00000, 0x39200000, 0x201f, 0x0 },
  { 0xffe03080, 0x9c403000, 0xf60, 0x0 },
  { 0xf7c02000, 0x11002000, 0x3000fe, 0x0 },
  { 0xfc003c00, 0x28002800, 0x3f00000, 0x1 },
  { 0xffe00004, 0x40400000, 0x20f8, 0x0 },
  { 0xf7c02000, 0x26802000, 0x3000fe, 0x0 },
  { 0xf7c02000, 0x13000000, 0x3000fe, 0x0 },
  { 0xffe00004, 0x42600000, 0x20f8, 0x0 },
  { 0xf8003000, 0x60001000, 0x7f00000, 0x1 },
  { 0xff602060, 0x3e400020, 0x1f80, 0x0 },
  { 0xff602060, 0x3f000000, 0x1f80, 0x0 },
  { 0xf7c02000, 0x24c02000, 0x3000fe, 0x0 },
  { 0xff802000, 0x74002000, 0x1fe0, 0x0 },
  { 0xf8003800, 0x20002000, 0x7f00000, 0x1 },
  { 0xffe03000, 0x9aa01000, 0xf60, 0x0 },
  { 0xf7c02000, 0x12400000, 0x3000fe, 0x0 },
  { 0xff602060, 0x3f000060, 0x1f80, 0x0 },
  { 0xf7c02000, 0x11000000, 0x3000fe, 0x0 },
};
