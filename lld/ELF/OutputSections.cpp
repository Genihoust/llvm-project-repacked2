//===- OutputSections.cpp -------------------------------------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "OutputSections.h"
#include "Config.h"
#include "EhFrame.h"
#include "GdbIndex.h"
#include "LinkerScript.h"
#include "Memory.h"
#include "Strings.h"
#include "SymbolTable.h"
#include "SyntheticSections.h"
#include "Target.h"
#include "lld/Core/Parallel.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/SHA1.h"

using namespace llvm;
using namespace llvm::dwarf;
using namespace llvm::object;
using namespace llvm::support::endian;
using namespace llvm::ELF;

using namespace lld;
using namespace lld::elf;

template <class ELFT>
OutputSectionBase<ELFT>::OutputSectionBase(StringRef Name, uint32_t Type,
                                           uintX_t Flags)
    : Name(Name) {
  memset(&Header, 0, sizeof(Elf_Shdr));
  Header.sh_type = Type;
  Header.sh_flags = Flags;
  Header.sh_addralign = 1;
}

template <class ELFT> uint32_t OutputSectionBase<ELFT>::getPhdrFlags() const {
  uintX_t Flags = getFlags();
  uint32_t Ret = PF_R;
  if (Flags & SHF_WRITE)
    Ret |= PF_W;
  if (Flags & SHF_EXECINSTR)
    Ret |= PF_X;
  return Ret;
}

template <class ELFT>
void OutputSectionBase<ELFT>::writeHeaderTo(Elf_Shdr *Shdr) {
  *Shdr = Header;
}

template <class ELFT>
GdbIndexSection<ELFT>::GdbIndexSection()
    : OutputSectionBase<ELFT>(".gdb_index", SHT_PROGBITS, 0) {}

template <class ELFT> void GdbIndexSection<ELFT>::parseDebugSections() {
  std::vector<InputSection<ELFT> *> &IS =
      static_cast<OutputSection<ELFT> *>(Out<ELFT>::DebugInfo)->Sections;

  for (InputSection<ELFT> *I : IS)
    readDwarf(I);
}

template <class ELFT>
void GdbIndexSection<ELFT>::readDwarf(InputSection<ELFT> *I) {
  std::vector<std::pair<uintX_t, uintX_t>> CuList = readCuList(I);
  CompilationUnits.insert(CompilationUnits.end(), CuList.begin(), CuList.end());
}

template <class ELFT> void GdbIndexSection<ELFT>::finalize() {
  parseDebugSections();

  // GdbIndex header consist from version fields
  // and 5 more fields with different kinds of offsets.
  CuTypesOffset = CuListOffset + CompilationUnits.size() * CompilationUnitSize;
  this->Header.sh_size = CuTypesOffset;
}

template <class ELFT> void GdbIndexSection<ELFT>::writeTo(uint8_t *Buf) {
  write32le(Buf, 7);                  // Write Version
  write32le(Buf + 4, CuListOffset);   // CU list offset
  write32le(Buf + 8, CuTypesOffset);  // Types CU list offset
  write32le(Buf + 12, CuTypesOffset); // Address area offset
  write32le(Buf + 16, CuTypesOffset); // Symbol table offset
  write32le(Buf + 20, CuTypesOffset); // Constant pool offset
  Buf += 24;

  // Write the CU list.
  for (std::pair<uintX_t, uintX_t> CU : CompilationUnits) {
    write64le(Buf, CU.first);
    write64le(Buf + 8, CU.second);
    Buf += 16;
  }
}

template <class ELFT>
GotPltSection<ELFT>::GotPltSection()
    : OutputSectionBase<ELFT>(".got.plt", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE) {
  this->Header.sh_addralign = Target->GotPltEntrySize;
}

template <class ELFT> void GotPltSection<ELFT>::addEntry(SymbolBody &Sym) {
  Sym.GotPltIndex = Target->GotPltHeaderEntriesNum + Entries.size();
  Entries.push_back(&Sym);
}

template <class ELFT> bool GotPltSection<ELFT>::empty() const {
  return Entries.empty();
}

template <class ELFT> void GotPltSection<ELFT>::finalize() {
  this->Header.sh_size = (Target->GotPltHeaderEntriesNum + Entries.size()) *
                         Target->GotPltEntrySize;
}

template <class ELFT> void GotPltSection<ELFT>::writeTo(uint8_t *Buf) {
  Target->writeGotPltHeader(Buf);
  Buf += Target->GotPltHeaderEntriesNum * Target->GotPltEntrySize;
  for (const SymbolBody *B : Entries) {
    Target->writeGotPlt(Buf, *B);
    Buf += sizeof(uintX_t);
  }
}

template <class ELFT>
GotSection<ELFT>::GotSection()
    : OutputSectionBase<ELFT>(".got", SHT_PROGBITS, SHF_ALLOC | SHF_WRITE) {
  if (Config->EMachine == EM_MIPS)
    this->Header.sh_flags |= SHF_MIPS_GPREL;
  this->Header.sh_addralign = Target->GotEntrySize;
}

template <class ELFT> void GotSection<ELFT>::addEntry(SymbolBody &Sym) {
  Sym.GotIndex = Entries.size();
  Entries.push_back(&Sym);
}

template <class ELFT>
void GotSection<ELFT>::addMipsEntry(SymbolBody &Sym, uintX_t Addend,
                                    RelExpr Expr) {
  // For "true" local symbols which can be referenced from the same module
  // only compiler creates two instructions for address loading:
  //
  // lw   $8, 0($gp) # R_MIPS_GOT16
  // addi $8, $8, 0  # R_MIPS_LO16
  //
  // The first instruction loads high 16 bits of the symbol address while
  // the second adds an offset. That allows to reduce number of required
  // GOT entries because only one global offset table entry is necessary
  // for every 64 KBytes of local data. So for local symbols we need to
  // allocate number of GOT entries to hold all required "page" addresses.
  //
  // All global symbols (hidden and regular) considered by compiler uniformly.
  // It always generates a single `lw` instruction and R_MIPS_GOT16 relocation
  // to load address of the symbol. So for each such symbol we need to
  // allocate dedicated GOT entry to store its address.
  //
  // If a symbol is preemptible we need help of dynamic linker to get its
  // final address. The corresponding GOT entries are allocated in the
  // "global" part of GOT. Entries for non preemptible global symbol allocated
  // in the "local" part of GOT.
  //
  // See "Global Offset Table" in Chapter 5:
  // ftp://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf
  if (Expr == R_MIPS_GOT_LOCAL_PAGE) {
    // At this point we do not know final symbol value so to reduce number
    // of allocated GOT entries do the following trick. Save all output
    // sections referenced by GOT relocations. Then later in the `finalize`
    // method calculate number of "pages" required to cover all saved output
    // section and allocate appropriate number of GOT entries.
    auto *OutSec = cast<DefinedRegular<ELFT>>(&Sym)->Section->OutSec;
    MipsOutSections.insert(OutSec);
    return;
  }
  if (Sym.isTls()) {
    // GOT entries created for MIPS TLS relocations behave like
    // almost GOT entries from other ABIs. They go to the end
    // of the global offset table.
    Sym.GotIndex = Entries.size();
    Entries.push_back(&Sym);
    return;
  }
  auto AddEntry = [&](SymbolBody &S, uintX_t A, MipsGotEntries &Items) {
    if (S.isInGot() && !A)
      return;
    size_t NewIndex = Items.size();
    if (!MipsGotMap.insert({{&S, A}, NewIndex}).second)
      return;
    Items.emplace_back(&S, A);
    if (!A)
      S.GotIndex = NewIndex;
  };
  if (Sym.isPreemptible()) {
    // Ignore addends for preemptible symbols. They got single GOT entry anyway.
    AddEntry(Sym, 0, MipsGlobal);
    Sym.IsInGlobalMipsGot = true;
  } else if (Expr == R_MIPS_GOT_OFF32) {
    AddEntry(Sym, Addend, MipsLocal32);
    Sym.Is32BitMipsGot = true;
  } else {
    // Hold local GOT entries accessed via a 16-bit index separately.
    // That allows to write them in the beginning of the GOT and keep
    // their indexes as less as possible to escape relocation's overflow.
    AddEntry(Sym, Addend, MipsLocal);
  }
}

template <class ELFT> bool GotSection<ELFT>::addDynTlsEntry(SymbolBody &Sym) {
  if (Sym.GlobalDynIndex != -1U)
    return false;
  Sym.GlobalDynIndex = Entries.size();
  // Global Dynamic TLS entries take two GOT slots.
  Entries.push_back(nullptr);
  Entries.push_back(&Sym);
  return true;
}

// Reserves TLS entries for a TLS module ID and a TLS block offset.
// In total it takes two GOT slots.
template <class ELFT> bool GotSection<ELFT>::addTlsIndex() {
  if (TlsIndexOff != uint32_t(-1))
    return false;
  TlsIndexOff = Entries.size() * sizeof(uintX_t);
  Entries.push_back(nullptr);
  Entries.push_back(nullptr);
  return true;
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t
GotSection<ELFT>::getMipsLocalPageOffset(uintX_t EntryValue) {
  // Initialize the entry by the %hi(EntryValue) expression
  // but without right-shifting.
  EntryValue = (EntryValue + 0x8000) & ~0xffff;
  // Take into account MIPS GOT header.
  // See comment in the GotSection::writeTo.
  size_t NewIndex = MipsLocalGotPos.size() + 2;
  auto P = MipsLocalGotPos.insert(std::make_pair(EntryValue, NewIndex));
  assert(!P.second || MipsLocalGotPos.size() <= MipsPageEntries);
  return (uintX_t)P.first->second * sizeof(uintX_t) - MipsGPOffset;
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t
GotSection<ELFT>::getMipsGotOffset(const SymbolBody &B, uintX_t Addend) const {
  // Calculate offset of the GOT entries block: TLS, global, local.
  uintX_t GotBlockOff;
  if (B.isTls())
    GotBlockOff = getMipsTlsOffset();
  else if (B.IsInGlobalMipsGot)
    GotBlockOff = getMipsLocalEntriesNum() * sizeof(uintX_t);
  else if (B.Is32BitMipsGot)
    GotBlockOff = (MipsPageEntries + MipsLocal.size()) * sizeof(uintX_t);
  else
    GotBlockOff = MipsPageEntries * sizeof(uintX_t);
  // Calculate index of the GOT entry in the block.
  uintX_t GotIndex;
  if (B.isInGot())
    GotIndex = B.GotIndex;
  else {
    auto It = MipsGotMap.find({&B, Addend});
    assert(It != MipsGotMap.end());
    GotIndex = It->second;
  }
  return GotBlockOff + GotIndex * sizeof(uintX_t) - MipsGPOffset;
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t GotSection<ELFT>::getMipsTlsOffset() const {
  return (getMipsLocalEntriesNum() + MipsGlobal.size()) * sizeof(uintX_t);
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t
GotSection<ELFT>::getGlobalDynAddr(const SymbolBody &B) const {
  return this->getVA() + B.GlobalDynIndex * sizeof(uintX_t);
}

template <class ELFT>
typename GotSection<ELFT>::uintX_t
GotSection<ELFT>::getGlobalDynOffset(const SymbolBody &B) const {
  return B.GlobalDynIndex * sizeof(uintX_t);
}

template <class ELFT>
const SymbolBody *GotSection<ELFT>::getMipsFirstGlobalEntry() const {
  return MipsGlobal.empty() ? nullptr : MipsGlobal.front().first;
}

template <class ELFT>
unsigned GotSection<ELFT>::getMipsLocalEntriesNum() const {
  return MipsPageEntries + MipsLocal.size() + MipsLocal32.size();
}

template <class ELFT> void GotSection<ELFT>::finalize() {
  size_t EntriesNum = Entries.size();
  if (Config->EMachine == EM_MIPS) {
    // Take into account MIPS GOT header.
    // See comment in the GotSection::writeTo.
    MipsPageEntries += 2;
    for (const OutputSectionBase<ELFT> *OutSec : MipsOutSections) {
      // Calculate an upper bound of MIPS GOT entries required to store page
      // addresses of local symbols. We assume the worst case - each 64kb
      // page of the output section has at least one GOT relocation against it.
      // Add 0x8000 to the section's size because the page address stored
      // in the GOT entry is calculated as (value + 0x8000) & ~0xffff.
      MipsPageEntries += (OutSec->getSize() + 0x8000 + 0xfffe) / 0xffff;
    }
    EntriesNum += getMipsLocalEntriesNum() + MipsGlobal.size();
  }
  this->Header.sh_size = EntriesNum * sizeof(uintX_t);
}

template <class ELFT>
static void writeUint(uint8_t *Buf, typename ELFT::uint Val) {
  typedef typename ELFT::uint uintX_t;
  write<uintX_t, ELFT::TargetEndianness, sizeof(uintX_t)>(Buf, Val);
}

template <class ELFT> void GotSection<ELFT>::writeMipsGot(uint8_t *Buf) {
  // Set the MSB of the second GOT slot. This is not required by any
  // MIPS ABI documentation, though.
  //
  // There is a comment in glibc saying that "The MSB of got[1] of a
  // gnu object is set to identify gnu objects," and in GNU gold it
  // says "the second entry will be used by some runtime loaders".
  // But how this field is being used is unclear.
  //
  // We are not really willing to mimic other linkers behaviors
  // without understanding why they do that, but because all files
  // generated by GNU tools have this special GOT value, and because
  // we've been doing this for years, it is probably a safe bet to
  // keep doing this for now. We really need to revisit this to see
  // if we had to do this.
  auto *P = reinterpret_cast<typename ELFT::Off *>(Buf);
  P[1] = uintX_t(1) << (ELFT::Is64Bits ? 63 : 31);
  // Write 'page address' entries to the local part of the GOT.
  for (std::pair<uintX_t, size_t> &L : MipsLocalGotPos) {
    uint8_t *Entry = Buf + L.second * sizeof(uintX_t);
    writeUint<ELFT>(Entry, L.first);
  }
  Buf += MipsPageEntries * sizeof(uintX_t);
  auto AddEntry = [&](const MipsGotEntry &SA) {
    uint8_t *Entry = Buf;
    Buf += sizeof(uintX_t);
    const SymbolBody *Body = SA.first;
    uintX_t VA = Body->template getVA<ELFT>(SA.second);
    writeUint<ELFT>(Entry, VA);
  };
  std::for_each(std::begin(MipsLocal), std::end(MipsLocal), AddEntry);
  std::for_each(std::begin(MipsLocal32), std::end(MipsLocal32), AddEntry);
  std::for_each(std::begin(MipsGlobal), std::end(MipsGlobal), AddEntry);
  // Initialize TLS-related GOT entries. If the entry has a corresponding
  // dynamic relocations, leave it initialized by zero. Write down adjusted
  // TLS symbol's values otherwise. To calculate the adjustments use offsets
  // for thread-local storage.
  // https://www.linux-mips.org/wiki/NPTL
  if (TlsIndexOff != -1U && !Config->Pic)
    writeUint<ELFT>(Buf + TlsIndexOff, 1);
  for (const SymbolBody *B : Entries) {
    if (!B || B->isPreemptible())
      continue;
    uintX_t VA = B->getVA<ELFT>();
    if (B->GotIndex != -1U) {
      uint8_t *Entry = Buf + B->GotIndex * sizeof(uintX_t);
      writeUint<ELFT>(Entry, VA - 0x7000);
    }
    if (B->GlobalDynIndex != -1U) {
      uint8_t *Entry = Buf + B->GlobalDynIndex * sizeof(uintX_t);
      writeUint<ELFT>(Entry, 1);
      Entry += sizeof(uintX_t);
      writeUint<ELFT>(Entry, VA - 0x8000);
    }
  }
}

template <class ELFT> void GotSection<ELFT>::writeTo(uint8_t *Buf) {
  if (Config->EMachine == EM_MIPS) {
    writeMipsGot(Buf);
    return;
  }
  for (const SymbolBody *B : Entries) {
    uint8_t *Entry = Buf;
    Buf += sizeof(uintX_t);
    if (!B)
      continue;
    if (B->isPreemptible())
      continue; // The dynamic linker will take care of it.
    uintX_t VA = B->getVA<ELFT>();
    writeUint<ELFT>(Entry, VA);
  }
}

template <class ELFT>
PltSection<ELFT>::PltSection()
    : OutputSectionBase<ELFT>(".plt", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR) {
  this->Header.sh_addralign = 16;
}

template <class ELFT> void PltSection<ELFT>::writeTo(uint8_t *Buf) {
  // At beginning of PLT, we have code to call the dynamic linker
  // to resolve dynsyms at runtime. Write such code.
  Target->writePltHeader(Buf);
  size_t Off = Target->PltHeaderSize;

  for (auto &I : Entries) {
    const SymbolBody *B = I.first;
    unsigned RelOff = I.second;
    uint64_t Got = B->getGotPltVA<ELFT>();
    uint64_t Plt = this->getVA() + Off;
    Target->writePlt(Buf + Off, Got, Plt, B->PltIndex, RelOff);
    Off += Target->PltEntrySize;
  }
}

template <class ELFT> void PltSection<ELFT>::addEntry(SymbolBody &Sym) {
  Sym.PltIndex = Entries.size();
  unsigned RelOff = Out<ELFT>::RelaPlt->getRelocOffset();
  Entries.push_back(std::make_pair(&Sym, RelOff));
}

template <class ELFT> void PltSection<ELFT>::finalize() {
  this->Header.sh_size =
      Target->PltHeaderSize + Entries.size() * Target->PltEntrySize;
}

template <class ELFT>
RelocationSection<ELFT>::RelocationSection(StringRef Name, bool Sort)
    : OutputSectionBase<ELFT>(Name, Config->Rela ? SHT_RELA : SHT_REL,
                              SHF_ALLOC),
      Sort(Sort) {
  this->Header.sh_entsize = Config->Rela ? sizeof(Elf_Rela) : sizeof(Elf_Rel);
  this->Header.sh_addralign = sizeof(uintX_t);
}

template <class ELFT>
void RelocationSection<ELFT>::addReloc(const DynamicReloc<ELFT> &Reloc) {
  if (Reloc.Type == Target->RelativeRel)
    ++NumRelativeRelocs;
  Relocs.push_back(Reloc);
}

template <class ELFT, class RelTy>
static bool compRelocations(const RelTy &A, const RelTy &B) {
  bool AIsRel = A.getType(Config->Mips64EL) == Target->RelativeRel;
  bool BIsRel = B.getType(Config->Mips64EL) == Target->RelativeRel;
  if (AIsRel != BIsRel)
    return AIsRel;

  return A.getSymbol(Config->Mips64EL) < B.getSymbol(Config->Mips64EL);
}

template <class ELFT> void RelocationSection<ELFT>::writeTo(uint8_t *Buf) {
  uint8_t *BufBegin = Buf;
  for (const DynamicReloc<ELFT> &Rel : Relocs) {
    auto *P = reinterpret_cast<Elf_Rela *>(Buf);
    Buf += Config->Rela ? sizeof(Elf_Rela) : sizeof(Elf_Rel);

    if (Config->Rela)
      P->r_addend = Rel.getAddend();
    P->r_offset = Rel.getOffset();
    if (Config->EMachine == EM_MIPS && Rel.getOutputSec() == Out<ELFT>::Got)
      // Dynamic relocation against MIPS GOT section make deal TLS entries
      // allocated in the end of the GOT. We need to adjust the offset to take
      // in account 'local' and 'global' GOT entries.
      P->r_offset += Out<ELFT>::Got->getMipsTlsOffset();
    P->setSymbolAndType(Rel.getSymIndex(), Rel.Type, Config->Mips64EL);
  }

  if (Sort) {
    if (Config->Rela)
      std::stable_sort((Elf_Rela *)BufBegin,
                       (Elf_Rela *)BufBegin + Relocs.size(),
                       compRelocations<ELFT, Elf_Rela>);
    else
      std::stable_sort((Elf_Rel *)BufBegin, (Elf_Rel *)BufBegin + Relocs.size(),
                       compRelocations<ELFT, Elf_Rel>);
  }
}

template <class ELFT> unsigned RelocationSection<ELFT>::getRelocOffset() {
  return this->Header.sh_entsize * Relocs.size();
}

template <class ELFT> void RelocationSection<ELFT>::finalize() {
  this->Header.sh_link = Out<ELFT>::DynSymTab
                             ? Out<ELFT>::DynSymTab->SectionIndex
                             : Out<ELFT>::SymTab->SectionIndex;
  this->Header.sh_size = Relocs.size() * this->Header.sh_entsize;
}

template <class ELFT>
HashTableSection<ELFT>::HashTableSection()
    : OutputSectionBase<ELFT>(".hash", SHT_HASH, SHF_ALLOC) {
  this->Header.sh_entsize = sizeof(Elf_Word);
  this->Header.sh_addralign = sizeof(Elf_Word);
}

template <class ELFT> void HashTableSection<ELFT>::finalize() {
  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;

  unsigned NumEntries = 2;                             // nbucket and nchain.
  NumEntries += Out<ELFT>::DynSymTab->getNumSymbols(); // The chain entries.

  // Create as many buckets as there are symbols.
  // FIXME: This is simplistic. We can try to optimize it, but implementing
  // support for SHT_GNU_HASH is probably even more profitable.
  NumEntries += Out<ELFT>::DynSymTab->getNumSymbols();
  this->Header.sh_size = NumEntries * sizeof(Elf_Word);
}

template <class ELFT> void HashTableSection<ELFT>::writeTo(uint8_t *Buf) {
  unsigned NumSymbols = Out<ELFT>::DynSymTab->getNumSymbols();
  auto *P = reinterpret_cast<Elf_Word *>(Buf);
  *P++ = NumSymbols; // nbucket
  *P++ = NumSymbols; // nchain

  Elf_Word *Buckets = P;
  Elf_Word *Chains = P + NumSymbols;

  for (const SymbolTableEntry &S : Out<ELFT>::DynSymTab->getSymbols()) {
    SymbolBody *Body = S.Symbol;
    StringRef Name = Body->getName();
    unsigned I = Body->DynsymIndex;
    uint32_t Hash = hashSysV(Name) % NumSymbols;
    Chains[I] = Buckets[Hash];
    Buckets[Hash] = I;
  }
}

static uint32_t hashGnu(StringRef Name) {
  uint32_t H = 5381;
  for (uint8_t C : Name)
    H = (H << 5) + H + C;
  return H;
}

template <class ELFT>
GnuHashTableSection<ELFT>::GnuHashTableSection()
    : OutputSectionBase<ELFT>(".gnu.hash", SHT_GNU_HASH, SHF_ALLOC) {
  this->Header.sh_entsize = ELFT::Is64Bits ? 0 : 4;
  this->Header.sh_addralign = sizeof(uintX_t);
}

template <class ELFT>
unsigned GnuHashTableSection<ELFT>::calcNBuckets(unsigned NumHashed) {
  if (!NumHashed)
    return 0;

  // These values are prime numbers which are not greater than 2^(N-1) + 1.
  // In result, for any particular NumHashed we return a prime number
  // which is not greater than NumHashed.
  static const unsigned Primes[] = {
      1,   1,    3,    3,    7,    13,    31,    61,    127,   251,
      509, 1021, 2039, 4093, 8191, 16381, 32749, 65521, 131071};

  return Primes[std::min<unsigned>(Log2_32_Ceil(NumHashed),
                                   array_lengthof(Primes) - 1)];
}

// Bloom filter estimation: at least 8 bits for each hashed symbol.
// GNU Hash table requirement: it should be a power of 2,
//   the minimum value is 1, even for an empty table.
// Expected results for a 32-bit target:
//   calcMaskWords(0..4)   = 1
//   calcMaskWords(5..8)   = 2
//   calcMaskWords(9..16)  = 4
// For a 64-bit target:
//   calcMaskWords(0..8)   = 1
//   calcMaskWords(9..16)  = 2
//   calcMaskWords(17..32) = 4
template <class ELFT>
unsigned GnuHashTableSection<ELFT>::calcMaskWords(unsigned NumHashed) {
  if (!NumHashed)
    return 1;
  return NextPowerOf2((NumHashed - 1) / sizeof(Elf_Off));
}

template <class ELFT> void GnuHashTableSection<ELFT>::finalize() {
  unsigned NumHashed = Symbols.size();
  NBuckets = calcNBuckets(NumHashed);
  MaskWords = calcMaskWords(NumHashed);
  // Second hash shift estimation: just predefined values.
  Shift2 = ELFT::Is64Bits ? 6 : 5;

  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;
  this->Header.sh_size = sizeof(Elf_Word) * 4            // Header
                         + sizeof(Elf_Off) * MaskWords   // Bloom Filter
                         + sizeof(Elf_Word) * NBuckets   // Hash Buckets
                         + sizeof(Elf_Word) * NumHashed; // Hash Values
}

template <class ELFT> void GnuHashTableSection<ELFT>::writeTo(uint8_t *Buf) {
  writeHeader(Buf);
  if (Symbols.empty())
    return;
  writeBloomFilter(Buf);
  writeHashTable(Buf);
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeHeader(uint8_t *&Buf) {
  auto *P = reinterpret_cast<Elf_Word *>(Buf);
  *P++ = NBuckets;
  *P++ = Out<ELFT>::DynSymTab->getNumSymbols() - Symbols.size();
  *P++ = MaskWords;
  *P++ = Shift2;
  Buf = reinterpret_cast<uint8_t *>(P);
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeBloomFilter(uint8_t *&Buf) {
  unsigned C = sizeof(Elf_Off) * 8;

  auto *Masks = reinterpret_cast<Elf_Off *>(Buf);
  for (const SymbolData &Sym : Symbols) {
    size_t Pos = (Sym.Hash / C) & (MaskWords - 1);
    uintX_t V = (uintX_t(1) << (Sym.Hash % C)) |
                (uintX_t(1) << ((Sym.Hash >> Shift2) % C));
    Masks[Pos] |= V;
  }
  Buf += sizeof(Elf_Off) * MaskWords;
}

template <class ELFT>
void GnuHashTableSection<ELFT>::writeHashTable(uint8_t *Buf) {
  Elf_Word *Buckets = reinterpret_cast<Elf_Word *>(Buf);
  Elf_Word *Values = Buckets + NBuckets;

  int PrevBucket = -1;
  int I = 0;
  for (const SymbolData &Sym : Symbols) {
    int Bucket = Sym.Hash % NBuckets;
    assert(PrevBucket <= Bucket);
    if (Bucket != PrevBucket) {
      Buckets[Bucket] = Sym.Body->DynsymIndex;
      PrevBucket = Bucket;
      if (I > 0)
        Values[I - 1] |= 1;
    }
    Values[I] = Sym.Hash & ~1;
    ++I;
  }
  if (I > 0)
    Values[I - 1] |= 1;
}

// Add symbols to this symbol hash table. Note that this function
// destructively sort a given vector -- which is needed because
// GNU-style hash table places some sorting requirements.
template <class ELFT>
void GnuHashTableSection<ELFT>::addSymbols(std::vector<SymbolTableEntry> &V) {
  // Ideally this will just be 'auto' but GCC 6.1 is not able
  // to deduce it correctly.
  std::vector<SymbolTableEntry>::iterator Mid =
      std::stable_partition(V.begin(), V.end(), [](const SymbolTableEntry &S) {
        return S.Symbol->isUndefined();
      });
  if (Mid == V.end())
    return;
  for (auto I = Mid, E = V.end(); I != E; ++I) {
    SymbolBody *B = I->Symbol;
    size_t StrOff = I->StrTabOffset;
    Symbols.push_back({B, StrOff, hashGnu(B->getName())});
  }

  unsigned NBuckets = calcNBuckets(Symbols.size());
  std::stable_sort(Symbols.begin(), Symbols.end(),
                   [&](const SymbolData &L, const SymbolData &R) {
                     return L.Hash % NBuckets < R.Hash % NBuckets;
                   });

  V.erase(Mid, V.end());
  for (const SymbolData &Sym : Symbols)
    V.push_back({Sym.Body, Sym.STName});
}

// Returns the number of version definition entries. Because the first entry
// is for the version definition itself, it is the number of versioned symbols
// plus one. Note that we don't support multiple versions yet.
static unsigned getVerDefNum() { return Config->VersionDefinitions.size() + 1; }

template <class ELFT>
DynamicSection<ELFT>::DynamicSection()
    : OutputSectionBase<ELFT>(".dynamic", SHT_DYNAMIC, SHF_ALLOC | SHF_WRITE) {
  Elf_Shdr &Header = this->Header;
  Header.sh_addralign = sizeof(uintX_t);
  Header.sh_entsize = ELFT::Is64Bits ? 16 : 8;

  // .dynamic section is not writable on MIPS.
  // See "Special Section" in Chapter 4 in the following document:
  // ftp://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf
  if (Config->EMachine == EM_MIPS)
    Header.sh_flags = SHF_ALLOC;

  addEntries();
}

// There are some dynamic entries that don't depend on other sections.
// Such entries can be set early.
template <class ELFT> void DynamicSection<ELFT>::addEntries() {
  // Add strings to .dynstr early so that .dynstr's size will be
  // fixed early.
  for (StringRef S : Config->AuxiliaryList)
    Add({DT_AUXILIARY, Out<ELFT>::DynStrTab->addString(S)});
  if (!Config->RPath.empty())
    Add({Config->EnableNewDtags ? DT_RUNPATH : DT_RPATH,
         Out<ELFT>::DynStrTab->addString(Config->RPath)});
  for (SharedFile<ELFT> *F : Symtab<ELFT>::X->getSharedFiles())
    if (F->isNeeded())
      Add({DT_NEEDED, Out<ELFT>::DynStrTab->addString(F->getSoName())});
  if (!Config->SoName.empty())
    Add({DT_SONAME, Out<ELFT>::DynStrTab->addString(Config->SoName)});

  // Set DT_FLAGS and DT_FLAGS_1.
  uint32_t DtFlags = 0;
  uint32_t DtFlags1 = 0;
  if (Config->Bsymbolic)
    DtFlags |= DF_SYMBOLIC;
  if (Config->ZNodelete)
    DtFlags1 |= DF_1_NODELETE;
  if (Config->ZNow) {
    DtFlags |= DF_BIND_NOW;
    DtFlags1 |= DF_1_NOW;
  }
  if (Config->ZOrigin) {
    DtFlags |= DF_ORIGIN;
    DtFlags1 |= DF_1_ORIGIN;
  }

  if (DtFlags)
    Add({DT_FLAGS, DtFlags});
  if (DtFlags1)
    Add({DT_FLAGS_1, DtFlags1});

  if (!Config->Entry.empty())
    Add({DT_DEBUG, (uint64_t)0});
}

// Add remaining entries to complete .dynamic contents.
template <class ELFT> void DynamicSection<ELFT>::finalize() {
  if (this->Header.sh_size)
    return; // Already finalized.

  this->Header.sh_link = Out<ELFT>::DynStrTab->SectionIndex;

  if (Out<ELFT>::RelaDyn->hasRelocs()) {
    bool IsRela = Config->Rela;
    Add({IsRela ? DT_RELA : DT_REL, Out<ELFT>::RelaDyn});
    Add({IsRela ? DT_RELASZ : DT_RELSZ, Out<ELFT>::RelaDyn->getSize()});
    Add({IsRela ? DT_RELAENT : DT_RELENT,
         uintX_t(IsRela ? sizeof(Elf_Rela) : sizeof(Elf_Rel))});

    // MIPS dynamic loader does not support RELCOUNT tag.
    // The problem is in the tight relation between dynamic
    // relocations and GOT. So do not emit this tag on MIPS.
    if (Config->EMachine != EM_MIPS) {
      size_t NumRelativeRels = Out<ELFT>::RelaDyn->getRelativeRelocCount();
      if (Config->ZCombreloc && NumRelativeRels)
        Add({IsRela ? DT_RELACOUNT : DT_RELCOUNT, NumRelativeRels});
    }
  }
  if (Out<ELFT>::RelaPlt && Out<ELFT>::RelaPlt->hasRelocs()) {
    Add({DT_JMPREL, Out<ELFT>::RelaPlt});
    Add({DT_PLTRELSZ, Out<ELFT>::RelaPlt->getSize()});
    Add({Config->EMachine == EM_MIPS ? DT_MIPS_PLTGOT : DT_PLTGOT,
         Out<ELFT>::GotPlt});
    Add({DT_PLTREL, uint64_t(Config->Rela ? DT_RELA : DT_REL)});
  }

  Add({DT_SYMTAB, Out<ELFT>::DynSymTab});
  Add({DT_SYMENT, sizeof(Elf_Sym)});
  Add({DT_STRTAB, Out<ELFT>::DynStrTab});
  Add({DT_STRSZ, Out<ELFT>::DynStrTab->getSize()});
  if (Out<ELFT>::GnuHashTab)
    Add({DT_GNU_HASH, Out<ELFT>::GnuHashTab});
  if (Out<ELFT>::HashTab)
    Add({DT_HASH, Out<ELFT>::HashTab});

  if (Out<ELFT>::PreinitArray) {
    Add({DT_PREINIT_ARRAY, Out<ELFT>::PreinitArray});
    Add({DT_PREINIT_ARRAYSZ, Out<ELFT>::PreinitArray, Entry::SecSize});
  }
  if (Out<ELFT>::InitArray) {
    Add({DT_INIT_ARRAY, Out<ELFT>::InitArray});
    Add({DT_INIT_ARRAYSZ, Out<ELFT>::InitArray, Entry::SecSize});
  }
  if (Out<ELFT>::FiniArray) {
    Add({DT_FINI_ARRAY, Out<ELFT>::FiniArray});
    Add({DT_FINI_ARRAYSZ, Out<ELFT>::FiniArray, Entry::SecSize});
  }

  if (SymbolBody *B = Symtab<ELFT>::X->find(Config->Init))
    Add({DT_INIT, B});
  if (SymbolBody *B = Symtab<ELFT>::X->find(Config->Fini))
    Add({DT_FINI, B});

  bool HasVerNeed = Out<ELFT>::VerNeed->getNeedNum() != 0;
  if (HasVerNeed || Out<ELFT>::VerDef)
    Add({DT_VERSYM, Out<ELFT>::VerSym});
  if (Out<ELFT>::VerDef) {
    Add({DT_VERDEF, Out<ELFT>::VerDef});
    Add({DT_VERDEFNUM, getVerDefNum()});
  }
  if (HasVerNeed) {
    Add({DT_VERNEED, Out<ELFT>::VerNeed});
    Add({DT_VERNEEDNUM, Out<ELFT>::VerNeed->getNeedNum()});
  }

  if (Config->EMachine == EM_MIPS) {
    Add({DT_MIPS_RLD_VERSION, 1});
    Add({DT_MIPS_FLAGS, RHF_NOTPOT});
    Add({DT_MIPS_BASE_ADDRESS, Config->ImageBase});
    Add({DT_MIPS_SYMTABNO, Out<ELFT>::DynSymTab->getNumSymbols()});
    Add({DT_MIPS_LOCAL_GOTNO, Out<ELFT>::Got->getMipsLocalEntriesNum()});
    if (const SymbolBody *B = Out<ELFT>::Got->getMipsFirstGlobalEntry())
      Add({DT_MIPS_GOTSYM, B->DynsymIndex});
    else
      Add({DT_MIPS_GOTSYM, Out<ELFT>::DynSymTab->getNumSymbols()});
    Add({DT_PLTGOT, Out<ELFT>::Got});
    if (Out<ELFT>::MipsRldMap)
      Add({DT_MIPS_RLD_MAP, Out<ELFT>::MipsRldMap});
  }

  // +1 for DT_NULL
  this->Header.sh_size = (Entries.size() + 1) * this->Header.sh_entsize;
}

template <class ELFT> void DynamicSection<ELFT>::writeTo(uint8_t *Buf) {
  auto *P = reinterpret_cast<Elf_Dyn *>(Buf);

  for (const Entry &E : Entries) {
    P->d_tag = E.Tag;
    switch (E.Kind) {
    case Entry::SecAddr:
      P->d_un.d_ptr = E.OutSec->getVA();
      break;
    case Entry::SecSize:
      P->d_un.d_val = E.OutSec->getSize();
      break;
    case Entry::SymAddr:
      P->d_un.d_ptr = E.Sym->template getVA<ELFT>();
      break;
    case Entry::PlainInt:
      P->d_un.d_val = E.Val;
      break;
    }
    ++P;
  }
}

template <class ELFT>
EhFrameHeader<ELFT>::EhFrameHeader()
    : OutputSectionBase<ELFT>(".eh_frame_hdr", SHT_PROGBITS, SHF_ALLOC) {}

// .eh_frame_hdr contains a binary search table of pointers to FDEs.
// Each entry of the search table consists of two values,
// the starting PC from where FDEs covers, and the FDE's address.
// It is sorted by PC.
template <class ELFT> void EhFrameHeader<ELFT>::writeTo(uint8_t *Buf) {
  const endianness E = ELFT::TargetEndianness;

  // Sort the FDE list by their PC and uniqueify. Usually there is only
  // one FDE for a PC (i.e. function), but if ICF merges two functions
  // into one, there can be more than one FDEs pointing to the address.
  auto Less = [](const FdeData &A, const FdeData &B) { return A.Pc < B.Pc; };
  std::stable_sort(Fdes.begin(), Fdes.end(), Less);
  auto Eq = [](const FdeData &A, const FdeData &B) { return A.Pc == B.Pc; };
  Fdes.erase(std::unique(Fdes.begin(), Fdes.end(), Eq), Fdes.end());

  Buf[0] = 1;
  Buf[1] = DW_EH_PE_pcrel | DW_EH_PE_sdata4;
  Buf[2] = DW_EH_PE_udata4;
  Buf[3] = DW_EH_PE_datarel | DW_EH_PE_sdata4;
  write32<E>(Buf + 4, Out<ELFT>::EhFrame->getVA() - this->getVA() - 4);
  write32<E>(Buf + 8, Fdes.size());
  Buf += 12;

  uintX_t VA = this->getVA();
  for (FdeData &Fde : Fdes) {
    write32<E>(Buf, Fde.Pc - VA);
    write32<E>(Buf + 4, Fde.FdeVA - VA);
    Buf += 8;
  }
}

template <class ELFT> void EhFrameHeader<ELFT>::finalize() {
  // .eh_frame_hdr has a 12 bytes header followed by an array of FDEs.
  this->Header.sh_size = 12 + Out<ELFT>::EhFrame->NumFdes * 8;
}

template <class ELFT>
void EhFrameHeader<ELFT>::addFde(uint32_t Pc, uint32_t FdeVA) {
  Fdes.push_back({Pc, FdeVA});
}

template <class ELFT>
OutputSection<ELFT>::OutputSection(StringRef Name, uint32_t Type, uintX_t Flags)
    : OutputSectionBase<ELFT>(Name, Type, Flags) {
  if (Type == SHT_RELA)
    this->Header.sh_entsize = sizeof(Elf_Rela);
  else if (Type == SHT_REL)
    this->Header.sh_entsize = sizeof(Elf_Rel);
}

template <class ELFT> void OutputSection<ELFT>::finalize() {
  uint32_t Type = this->Header.sh_type;
  if (this->Header.sh_flags & SHF_LINK_ORDER) {
    if (!Config->Relocatable) {
      // SHF_LINK_ORDER only has meaning in relocatable objects
      this->Header.sh_flags &= ~SHF_LINK_ORDER;
    }
    else if (!this->Sections.empty()) {
      // When doing a relocatable link we must preserve the link order
      // dependency of sections with the SHF_LINK_ORDER flag. The dependency
      // is indicated by the sh_link field. We need to translate the
      // InputSection sh_link to the OutputSection sh_link, all InputSections
      // in the OutputSection have the same dependency.
      if (auto *D = this->Sections.front()->getLinkOrderDep())
        this->Header.sh_link = D->OutSec->SectionIndex;
    }
  }
  if (Type != SHT_RELA && Type != SHT_REL)
    return;
  this->Header.sh_link = Out<ELFT>::SymTab->SectionIndex;
  // sh_info for SHT_REL[A] sections should contain the section header index of
  // the section to which the relocation applies.
  InputSectionBase<ELFT> *S = Sections[0]->getRelocatedSection();
  this->Header.sh_info = S->OutSec->SectionIndex;
}

template <class ELFT>
void OutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  assert(C->Live);
  auto *S = cast<InputSection<ELFT>>(C);
  Sections.push_back(S);
  S->OutSec = this;
  this->updateAlignment(S->Alignment);
  // Keep sh_entsize value of the input section to be able to perform merging
  // later during a final linking using the generated relocatable object.
  if (Config->Relocatable && (S->Flags & SHF_MERGE))
    this->Header.sh_entsize = S->Entsize;
}

// This function is called after we sort input sections
// and scan relocations to setup sections' offsets.
template <class ELFT> void OutputSection<ELFT>::assignOffsets() {
  uintX_t Off = this->Header.sh_size;
  for (InputSection<ELFT> *S : Sections) {
    Off = alignTo(Off, S->Alignment);
    S->OutSecOff = Off;
    Off += S->getSize();
  }
  this->Header.sh_size = Off;
}

// Sorts input sections by section name suffixes, so that .foo.N comes
// before .foo.M if N < M. Used to sort .{init,fini}_array.N sections.
// We want to keep the original order if the priorities are the same
// because the compiler keeps the original initialization order in a
// translation unit and we need to respect that.
// For more detail, read the section of the GCC's manual about init_priority.
template <class ELFT> void OutputSection<ELFT>::sortInitFini() {
  // Sort sections by priority.
  typedef std::pair<int, InputSection<ELFT> *> Pair;
  auto Comp = [](const Pair &A, const Pair &B) { return A.first < B.first; };

  std::vector<Pair> V;
  for (InputSection<ELFT> *S : Sections)
    V.push_back({getPriority(S->Name), S});
  std::stable_sort(V.begin(), V.end(), Comp);
  Sections.clear();
  for (Pair &P : V)
    Sections.push_back(P.second);
}

// Returns true if S matches /Filename.?\.o$/.
static bool isCrtBeginEnd(StringRef S, StringRef Filename) {
  if (!S.endswith(".o"))
    return false;
  S = S.drop_back(2);
  if (S.endswith(Filename))
    return true;
  return !S.empty() && S.drop_back().endswith(Filename);
}

static bool isCrtbegin(StringRef S) { return isCrtBeginEnd(S, "crtbegin"); }
static bool isCrtend(StringRef S) { return isCrtBeginEnd(S, "crtend"); }

// .ctors and .dtors are sorted by this priority from highest to lowest.
//
//  1. The section was contained in crtbegin (crtbegin contains
//     some sentinel value in its .ctors and .dtors so that the runtime
//     can find the beginning of the sections.)
//
//  2. The section has an optional priority value in the form of ".ctors.N"
//     or ".dtors.N" where N is a number. Unlike .{init,fini}_array,
//     they are compared as string rather than number.
//
//  3. The section is just ".ctors" or ".dtors".
//
//  4. The section was contained in crtend, which contains an end marker.
//
// In an ideal world, we don't need this function because .init_array and
// .ctors are duplicate features (and .init_array is newer.) However, there
// are too many real-world use cases of .ctors, so we had no choice to
// support that with this rather ad-hoc semantics.
template <class ELFT>
static bool compCtors(const InputSection<ELFT> *A,
                      const InputSection<ELFT> *B) {
  bool BeginA = isCrtbegin(A->getFile()->getName());
  bool BeginB = isCrtbegin(B->getFile()->getName());
  if (BeginA != BeginB)
    return BeginA;
  bool EndA = isCrtend(A->getFile()->getName());
  bool EndB = isCrtend(B->getFile()->getName());
  if (EndA != EndB)
    return EndB;
  StringRef X = A->Name;
  StringRef Y = B->Name;
  assert(X.startswith(".ctors") || X.startswith(".dtors"));
  assert(Y.startswith(".ctors") || Y.startswith(".dtors"));
  X = X.substr(6);
  Y = Y.substr(6);
  if (X.empty() && Y.empty())
    return false;
  return X < Y;
}

// Sorts input sections by the special rules for .ctors and .dtors.
// Unfortunately, the rules are different from the one for .{init,fini}_array.
// Read the comment above.
template <class ELFT> void OutputSection<ELFT>::sortCtorsDtors() {
  std::stable_sort(Sections.begin(), Sections.end(), compCtors<ELFT>);
}

static void fill(uint8_t *Buf, size_t Size, ArrayRef<uint8_t> A) {
  size_t I = 0;
  for (; I + A.size() < Size; I += A.size())
    memcpy(Buf + I, A.data(), A.size());
  memcpy(Buf + I, A.data(), Size - I);
}

template <class ELFT> void OutputSection<ELFT>::writeTo(uint8_t *Buf) {
  ArrayRef<uint8_t> Filler = Script<ELFT>::X->getFiller(this->Name);
  if (!Filler.empty())
    fill(Buf, this->getSize(), Filler);
  if (Config->Threads) {
    parallel_for_each(Sections.begin(), Sections.end(),
                      [=](InputSection<ELFT> *C) { C->writeTo(Buf); });
  } else {
    for (InputSection<ELFT> *C : Sections)
      C->writeTo(Buf);
  }
  // Linker scripts may have BYTE()-family commands with which you
  // can write arbitrary bytes to the output. Process them if any.
  Script<ELFT>::X->writeDataBytes(this->Name, Buf);
}

template <class ELFT>
EhOutputSection<ELFT>::EhOutputSection()
    : OutputSectionBase<ELFT>(".eh_frame", SHT_PROGBITS, SHF_ALLOC) {}

// Search for an existing CIE record or create a new one.
// CIE records from input object files are uniquified by their contents
// and where their relocations point to.
template <class ELFT>
template <class RelTy>
CieRecord *EhOutputSection<ELFT>::addCie(EhSectionPiece &Piece,
                                         EhInputSection<ELFT> *Sec,
                                         ArrayRef<RelTy> Rels) {
  const endianness E = ELFT::TargetEndianness;
  if (read32<E>(Piece.data().data() + 4) != 0)
    fatal("CIE expected at beginning of .eh_frame: " + Sec->Name);

  SymbolBody *Personality = nullptr;
  unsigned FirstRelI = Piece.FirstRelocation;
  if (FirstRelI != (unsigned)-1)
    Personality = &Sec->getFile()->getRelocTargetSym(Rels[FirstRelI]);

  // Search for an existing CIE by CIE contents/relocation target pair.
  CieRecord *Cie = &CieMap[{Piece.data(), Personality}];

  // If not found, create a new one.
  if (Cie->Piece == nullptr) {
    Cie->Piece = &Piece;
    Cies.push_back(Cie);
  }
  return Cie;
}

// There is one FDE per function. Returns true if a given FDE
// points to a live function.
template <class ELFT>
template <class RelTy>
bool EhOutputSection<ELFT>::isFdeLive(EhSectionPiece &Piece,
                                      EhInputSection<ELFT> *Sec,
                                      ArrayRef<RelTy> Rels) {
  unsigned FirstRelI = Piece.FirstRelocation;
  if (FirstRelI == (unsigned)-1)
    fatal("FDE doesn't reference another section");
  const RelTy &Rel = Rels[FirstRelI];
  SymbolBody &B = Sec->getFile()->getRelocTargetSym(Rel);
  auto *D = dyn_cast<DefinedRegular<ELFT>>(&B);
  if (!D || !D->Section)
    return false;
  InputSectionBase<ELFT> *Target = D->Section->Repl;
  return Target && Target->Live;
}

// .eh_frame is a sequence of CIE or FDE records. In general, there
// is one CIE record per input object file which is followed by
// a list of FDEs. This function searches an existing CIE or create a new
// one and associates FDEs to the CIE.
template <class ELFT>
template <class RelTy>
void EhOutputSection<ELFT>::addSectionAux(EhInputSection<ELFT> *Sec,
                                          ArrayRef<RelTy> Rels) {
  const endianness E = ELFT::TargetEndianness;

  DenseMap<size_t, CieRecord *> OffsetToCie;
  for (EhSectionPiece &Piece : Sec->Pieces) {
    // The empty record is the end marker.
    if (Piece.size() == 4)
      return;

    size_t Offset = Piece.InputOff;
    uint32_t ID = read32<E>(Piece.data().data() + 4);
    if (ID == 0) {
      OffsetToCie[Offset] = addCie(Piece, Sec, Rels);
      continue;
    }

    uint32_t CieOffset = Offset + 4 - ID;
    CieRecord *Cie = OffsetToCie[CieOffset];
    if (!Cie)
      fatal("invalid CIE reference");

    if (!isFdeLive(Piece, Sec, Rels))
      continue;
    Cie->FdePieces.push_back(&Piece);
    NumFdes++;
  }
}

template <class ELFT>
void EhOutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  auto *Sec = cast<EhInputSection<ELFT>>(C);
  Sec->OutSec = this;
  this->updateAlignment(Sec->Alignment);
  Sections.push_back(Sec);

  // .eh_frame is a sequence of CIE or FDE records. This function
  // splits it into pieces so that we can call
  // SplitInputSection::getSectionPiece on the section.
  Sec->split();
  if (Sec->Pieces.empty())
    return;

  if (const Elf_Shdr *RelSec = Sec->RelocSection) {
    ELFFile<ELFT> Obj = Sec->getFile()->getObj();
    if (RelSec->sh_type == SHT_RELA)
      addSectionAux(Sec, check(Obj.relas(RelSec)));
    else
      addSectionAux(Sec, check(Obj.rels(RelSec)));
    return;
  }
  addSectionAux(Sec, makeArrayRef<Elf_Rela>(nullptr, nullptr));
}

template <class ELFT>
static void writeCieFde(uint8_t *Buf, ArrayRef<uint8_t> D) {
  memcpy(Buf, D.data(), D.size());

  // Fix the size field. -4 since size does not include the size field itself.
  const endianness E = ELFT::TargetEndianness;
  write32<E>(Buf, alignTo(D.size(), sizeof(typename ELFT::uint)) - 4);
}

template <class ELFT> void EhOutputSection<ELFT>::finalize() {
  if (this->Header.sh_size)
    return; // Already finalized.

  size_t Off = 0;
  for (CieRecord *Cie : Cies) {
    Cie->Piece->OutputOff = Off;
    Off += alignTo(Cie->Piece->size(), sizeof(uintX_t));

    for (EhSectionPiece *Fde : Cie->FdePieces) {
      Fde->OutputOff = Off;
      Off += alignTo(Fde->size(), sizeof(uintX_t));
    }
  }
  this->Header.sh_size = Off;
}

template <class ELFT> static uint64_t readFdeAddr(uint8_t *Buf, int Size) {
  const endianness E = ELFT::TargetEndianness;
  switch (Size) {
  case DW_EH_PE_udata2:
    return read16<E>(Buf);
  case DW_EH_PE_udata4:
    return read32<E>(Buf);
  case DW_EH_PE_udata8:
    return read64<E>(Buf);
  case DW_EH_PE_absptr:
    if (ELFT::Is64Bits)
      return read64<E>(Buf);
    return read32<E>(Buf);
  }
  fatal("unknown FDE size encoding");
}

// Returns the VA to which a given FDE (on a mmap'ed buffer) is applied to.
// We need it to create .eh_frame_hdr section.
template <class ELFT>
typename ELFT::uint EhOutputSection<ELFT>::getFdePc(uint8_t *Buf, size_t FdeOff,
                                                    uint8_t Enc) {
  // The starting address to which this FDE applies is
  // stored at FDE + 8 byte.
  size_t Off = FdeOff + 8;
  uint64_t Addr = readFdeAddr<ELFT>(Buf + Off, Enc & 0x7);
  if ((Enc & 0x70) == DW_EH_PE_absptr)
    return Addr;
  if ((Enc & 0x70) == DW_EH_PE_pcrel)
    return Addr + this->getVA() + Off;
  fatal("unknown FDE size relative encoding");
}

template <class ELFT> void EhOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  const endianness E = ELFT::TargetEndianness;
  for (CieRecord *Cie : Cies) {
    size_t CieOffset = Cie->Piece->OutputOff;
    writeCieFde<ELFT>(Buf + CieOffset, Cie->Piece->data());

    for (EhSectionPiece *Fde : Cie->FdePieces) {
      size_t Off = Fde->OutputOff;
      writeCieFde<ELFT>(Buf + Off, Fde->data());

      // FDE's second word should have the offset to an associated CIE.
      // Write it.
      write32<E>(Buf + Off + 4, Off + 4 - CieOffset);
    }
  }

  for (EhInputSection<ELFT> *S : Sections)
    S->relocate(Buf, nullptr);

  // Construct .eh_frame_hdr. .eh_frame_hdr is a binary search table
  // to get a FDE from an address to which FDE is applied. So here
  // we obtain two addresses and pass them to EhFrameHdr object.
  if (Out<ELFT>::EhFrameHdr) {
    for (CieRecord *Cie : Cies) {
      uint8_t Enc = getFdeEncoding<ELFT>(Cie->Piece->data());
      for (SectionPiece *Fde : Cie->FdePieces) {
        uintX_t Pc = getFdePc(Buf, Fde->OutputOff, Enc);
        uintX_t FdeVA = this->getVA() + Fde->OutputOff;
        Out<ELFT>::EhFrameHdr->addFde(Pc, FdeVA);
      }
    }
  }
}

template <class ELFT>
MergeOutputSection<ELFT>::MergeOutputSection(StringRef Name, uint32_t Type,
                                             uintX_t Flags, uintX_t Alignment)
    : OutputSectionBase<ELFT>(Name, Type, Flags),
      Builder(StringTableBuilder::RAW, Alignment) {}

template <class ELFT> void MergeOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  Builder.write(Buf);
}

template <class ELFT>
void MergeOutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  auto *Sec = cast<MergeInputSection<ELFT>>(C);
  Sec->OutSec = this;
  this->updateAlignment(Sec->Alignment);
  this->Header.sh_entsize = Sec->Entsize;
  Sections.push_back(Sec);

  auto HashI = Sec->Hashes.begin();
  for (auto I = Sec->Pieces.begin(), E = Sec->Pieces.end(); I != E; ++I) {
    SectionPiece &Piece = *I;
    uint32_t Hash = *HashI;
    ++HashI;
    if (!Piece.Live)
      continue;
    StringRef Data = toStringRef(Sec->getData(I));
    CachedHashStringRef V(Data, Hash);
    uintX_t OutputOffset = Builder.add(V);
    if (!shouldTailMerge())
      Piece.OutputOff = OutputOffset;
  }
}

template <class ELFT>
unsigned MergeOutputSection<ELFT>::getOffset(CachedHashStringRef Val) {
  return Builder.getOffset(Val);
}

template <class ELFT> bool MergeOutputSection<ELFT>::shouldTailMerge() const {
  return Config->Optimize >= 2 && this->Header.sh_flags & SHF_STRINGS;
}

template <class ELFT> void MergeOutputSection<ELFT>::finalize() {
  if (shouldTailMerge())
    Builder.finalize();
  else
    Builder.finalizeInOrder();
  this->Header.sh_size = Builder.getSize();
}

template <class ELFT> void MergeOutputSection<ELFT>::finalizePieces() {
  for (MergeInputSection<ELFT> *Sec : Sections)
    Sec->finalizePieces();
}

template <class ELFT>
StringTableSection<ELFT>::StringTableSection(StringRef Name, bool Dynamic)
    : OutputSectionBase<ELFT>(Name, SHT_STRTAB,
                              Dynamic ? (uintX_t)SHF_ALLOC : 0),
      Dynamic(Dynamic) {
  // ELF string tables start with a NUL byte, so 1.
  this->setSize(1);
}

// Adds a string to the string table. If HashIt is true we hash and check for
// duplicates. It is optional because the name of global symbols are already
// uniqued and hashing them again has a big cost for a small value: uniquing
// them with some other string that happens to be the same.
template <class ELFT>
unsigned StringTableSection<ELFT>::addString(StringRef S, bool HashIt) {
  if (HashIt) {
    auto R = StringMap.insert(std::make_pair(S, this->getSize()));
    if (!R.second)
      return R.first->second;
  }
  unsigned Ret = this->getSize();
  this->setSize(this->getSize() + S.size() + 1);
  Strings.push_back(S);
  return Ret;
}

template <class ELFT> void StringTableSection<ELFT>::writeTo(uint8_t *Buf) {
  // ELF string tables start with NUL byte, so advance the pointer by one.
  ++Buf;
  for (StringRef S : Strings) {
    memcpy(Buf, S.data(), S.size());
    Buf += S.size() + 1;
  }
}

template <class ELFT>
typename ELFT::uint DynamicReloc<ELFT>::getOffset() const {
  if (OutputSec)
    return OutputSec->getVA() + OffsetInSec;
  return InputSec->OutSec->getVA() + InputSec->getOffset(OffsetInSec);
}

template <class ELFT>
typename ELFT::uint DynamicReloc<ELFT>::getAddend() const {
  if (UseSymVA)
    return Sym->getVA<ELFT>(Addend);
  return Addend;
}

template <class ELFT> uint32_t DynamicReloc<ELFT>::getSymIndex() const {
  if (Sym && !UseSymVA)
    return Sym->DynsymIndex;
  return 0;
}

template <class ELFT>
SymbolTableSection<ELFT>::SymbolTableSection(
    StringTableSection<ELFT> &StrTabSec)
    : OutputSectionBase<ELFT>(StrTabSec.isDynamic() ? ".dynsym" : ".symtab",
                              StrTabSec.isDynamic() ? SHT_DYNSYM : SHT_SYMTAB,
                              StrTabSec.isDynamic() ? (uintX_t)SHF_ALLOC : 0),
      StrTabSec(StrTabSec) {
  this->Header.sh_entsize = sizeof(Elf_Sym);
  this->Header.sh_addralign = sizeof(uintX_t);
}

// Orders symbols according to their positions in the GOT,
// in compliance with MIPS ABI rules.
// See "Global Offset Table" in Chapter 5 in the following document
// for detailed description:
// ftp://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf
static bool sortMipsSymbols(const SymbolBody *L, const SymbolBody *R) {
  // Sort entries related to non-local preemptible symbols by GOT indexes.
  // All other entries go to the first part of GOT in arbitrary order.
  bool LIsInLocalGot = !L->IsInGlobalMipsGot;
  bool RIsInLocalGot = !R->IsInGlobalMipsGot;
  if (LIsInLocalGot || RIsInLocalGot)
    return !RIsInLocalGot;
  return L->GotIndex < R->GotIndex;
}

static uint8_t getSymbolBinding(SymbolBody *Body) {
  Symbol *S = Body->symbol();
  if (Config->Relocatable)
    return S->Binding;
  uint8_t Visibility = S->Visibility;
  if (Visibility != STV_DEFAULT && Visibility != STV_PROTECTED)
    return STB_LOCAL;
  if (Config->NoGnuUnique && S->Binding == STB_GNU_UNIQUE)
    return STB_GLOBAL;
  return S->Binding;
}

template <class ELFT> void SymbolTableSection<ELFT>::finalize() {
  if (this->Header.sh_size)
    return; // Already finalized.

  this->Header.sh_size = getNumSymbols() * sizeof(Elf_Sym);
  this->Header.sh_link = StrTabSec.SectionIndex;
  this->Header.sh_info = NumLocals + 1;

  if (Config->Relocatable) {
    size_t I = NumLocals;
    for (const SymbolTableEntry &S : Symbols)
      S.Symbol->DynsymIndex = ++I;
    return;
  }

  if (!StrTabSec.isDynamic()) {
    std::stable_sort(Symbols.begin(), Symbols.end(),
                     [](const SymbolTableEntry &L, const SymbolTableEntry &R) {
                       return getSymbolBinding(L.Symbol) == STB_LOCAL &&
                              getSymbolBinding(R.Symbol) != STB_LOCAL;
                     });
    return;
  }
  if (Out<ELFT>::GnuHashTab)
    // NB: It also sorts Symbols to meet the GNU hash table requirements.
    Out<ELFT>::GnuHashTab->addSymbols(Symbols);
  else if (Config->EMachine == EM_MIPS)
    std::stable_sort(Symbols.begin(), Symbols.end(),
                     [](const SymbolTableEntry &L, const SymbolTableEntry &R) {
                       return sortMipsSymbols(L.Symbol, R.Symbol);
                     });
  size_t I = 0;
  for (const SymbolTableEntry &S : Symbols)
    S.Symbol->DynsymIndex = ++I;
}

template <class ELFT> void SymbolTableSection<ELFT>::addSymbol(SymbolBody *B) {
  Symbols.push_back({B, StrTabSec.addString(B->getName(), false)});
}

template <class ELFT> void SymbolTableSection<ELFT>::writeTo(uint8_t *Buf) {
  Buf += sizeof(Elf_Sym);

  // All symbols with STB_LOCAL binding precede the weak and global symbols.
  // .dynsym only contains global symbols.
  if (Config->Discard != DiscardPolicy::All && !StrTabSec.isDynamic())
    writeLocalSymbols(Buf);

  writeGlobalSymbols(Buf);
}

template <class ELFT>
void SymbolTableSection<ELFT>::writeLocalSymbols(uint8_t *&Buf) {
  // Iterate over all input object files to copy their local symbols
  // to the output symbol table pointed by Buf.
  for (ObjectFile<ELFT> *File : Symtab<ELFT>::X->getObjectFiles()) {
    for (const std::pair<const DefinedRegular<ELFT> *, size_t> &P :
         File->KeptLocalSyms) {
      const DefinedRegular<ELFT> &Body = *P.first;
      InputSectionBase<ELFT> *Section = Body.Section;
      auto *ESym = reinterpret_cast<Elf_Sym *>(Buf);

      if (!Section) {
        ESym->st_shndx = SHN_ABS;
        ESym->st_value = Body.Value;
      } else {
        const OutputSectionBase<ELFT> *OutSec = Section->OutSec;
        ESym->st_shndx = OutSec->SectionIndex;
        ESym->st_value = OutSec->getVA() + Section->getOffset(Body);
      }
      ESym->st_name = P.second;
      ESym->st_size = Body.template getSize<ELFT>();
      ESym->setBindingAndType(STB_LOCAL, Body.Type);
      Buf += sizeof(*ESym);
    }
  }
}

template <class ELFT>
void SymbolTableSection<ELFT>::writeGlobalSymbols(uint8_t *Buf) {
  // Write the internal symbol table contents to the output symbol table
  // pointed by Buf.
  auto *ESym = reinterpret_cast<Elf_Sym *>(Buf);
  for (const SymbolTableEntry &S : Symbols) {
    SymbolBody *Body = S.Symbol;
    size_t StrOff = S.StrTabOffset;

    uint8_t Type = Body->Type;
    uintX_t Size = Body->getSize<ELFT>();

    ESym->setBindingAndType(getSymbolBinding(Body), Type);
    ESym->st_size = Size;
    ESym->st_name = StrOff;
    ESym->setVisibility(Body->symbol()->Visibility);
    ESym->st_value = Body->getVA<ELFT>();

    if (const OutputSectionBase<ELFT> *OutSec = getOutputSection(Body))
      ESym->st_shndx = OutSec->SectionIndex;
    else if (isa<DefinedRegular<ELFT>>(Body))
      ESym->st_shndx = SHN_ABS;

    if (Config->EMachine == EM_MIPS) {
      // On MIPS we need to mark symbol which has a PLT entry and requires
      // pointer equality by STO_MIPS_PLT flag. That is necessary to help
      // dynamic linker distinguish such symbols and MIPS lazy-binding stubs.
      // https://sourceware.org/ml/binutils/2008-07/txt00000.txt
      if (Body->isInPlt() && Body->NeedsCopyOrPltAddr)
        ESym->st_other |= STO_MIPS_PLT;
      if (Config->Relocatable) {
        auto *D = dyn_cast<DefinedRegular<ELFT>>(Body);
        if (D && D->isMipsPIC())
          ESym->st_other |= STO_MIPS_PIC;
      }
    }
    ++ESym;
  }
}

template <class ELFT>
const OutputSectionBase<ELFT> *
SymbolTableSection<ELFT>::getOutputSection(SymbolBody *Sym) {
  switch (Sym->kind()) {
  case SymbolBody::DefinedSyntheticKind:
    return cast<DefinedSynthetic<ELFT>>(Sym)->Section;
  case SymbolBody::DefinedRegularKind: {
    auto &D = cast<DefinedRegular<ELFT>>(*Sym);
    if (D.Section)
      return D.Section->OutSec;
    break;
  }
  case SymbolBody::DefinedCommonKind:
    return In<ELFT>::Common->OutSec;
  case SymbolBody::SharedKind:
    if (cast<SharedSymbol<ELFT>>(Sym)->needsCopy())
      return Out<ELFT>::Bss;
    break;
  case SymbolBody::UndefinedKind:
  case SymbolBody::LazyArchiveKind:
  case SymbolBody::LazyObjectKind:
    break;
  }
  return nullptr;
}

template <class ELFT>
VersionDefinitionSection<ELFT>::VersionDefinitionSection()
    : OutputSectionBase<ELFT>(".gnu.version_d", SHT_GNU_verdef, SHF_ALLOC) {
  this->Header.sh_addralign = sizeof(uint32_t);
}

static StringRef getFileDefName() {
  if (!Config->SoName.empty())
    return Config->SoName;
  return Config->OutputFile;
}

template <class ELFT> void VersionDefinitionSection<ELFT>::finalize() {
  FileDefNameOff = Out<ELFT>::DynStrTab->addString(getFileDefName());
  for (VersionDefinition &V : Config->VersionDefinitions)
    V.NameOff = Out<ELFT>::DynStrTab->addString(V.Name);

  this->Header.sh_size =
      (sizeof(Elf_Verdef) + sizeof(Elf_Verdaux)) * getVerDefNum();
  this->Header.sh_link = Out<ELFT>::DynStrTab->SectionIndex;

  // sh_info should be set to the number of definitions. This fact is missed in
  // documentation, but confirmed by binutils community:
  // https://sourceware.org/ml/binutils/2014-11/msg00355.html
  this->Header.sh_info = getVerDefNum();
}

template <class ELFT>
void VersionDefinitionSection<ELFT>::writeOne(uint8_t *Buf, uint32_t Index,
                                              StringRef Name, size_t NameOff) {
  auto *Verdef = reinterpret_cast<Elf_Verdef *>(Buf);
  Verdef->vd_version = 1;
  Verdef->vd_cnt = 1;
  Verdef->vd_aux = sizeof(Elf_Verdef);
  Verdef->vd_next = sizeof(Elf_Verdef) + sizeof(Elf_Verdaux);
  Verdef->vd_flags = (Index == 1 ? VER_FLG_BASE : 0);
  Verdef->vd_ndx = Index;
  Verdef->vd_hash = hashSysV(Name);

  auto *Verdaux = reinterpret_cast<Elf_Verdaux *>(Buf + sizeof(Elf_Verdef));
  Verdaux->vda_name = NameOff;
  Verdaux->vda_next = 0;
}

template <class ELFT>
void VersionDefinitionSection<ELFT>::writeTo(uint8_t *Buf) {
  writeOne(Buf, 1, getFileDefName(), FileDefNameOff);

  for (VersionDefinition &V : Config->VersionDefinitions) {
    Buf += sizeof(Elf_Verdef) + sizeof(Elf_Verdaux);
    writeOne(Buf, V.Id, V.Name, V.NameOff);
  }

  // Need to terminate the last version definition.
  Elf_Verdef *Verdef = reinterpret_cast<Elf_Verdef *>(Buf);
  Verdef->vd_next = 0;
}

template <class ELFT>
VersionTableSection<ELFT>::VersionTableSection()
    : OutputSectionBase<ELFT>(".gnu.version", SHT_GNU_versym, SHF_ALLOC) {
  this->Header.sh_addralign = sizeof(uint16_t);
}

template <class ELFT> void VersionTableSection<ELFT>::finalize() {
  this->Header.sh_size =
      sizeof(Elf_Versym) * (Out<ELFT>::DynSymTab->getSymbols().size() + 1);
  this->Header.sh_entsize = sizeof(Elf_Versym);
  // At the moment of june 2016 GNU docs does not mention that sh_link field
  // should be set, but Sun docs do. Also readelf relies on this field.
  this->Header.sh_link = Out<ELFT>::DynSymTab->SectionIndex;
}

template <class ELFT> void VersionTableSection<ELFT>::writeTo(uint8_t *Buf) {
  auto *OutVersym = reinterpret_cast<Elf_Versym *>(Buf) + 1;
  for (const SymbolTableEntry &S : Out<ELFT>::DynSymTab->getSymbols()) {
    OutVersym->vs_index = S.Symbol->symbol()->VersionId;
    ++OutVersym;
  }
}

template <class ELFT>
VersionNeedSection<ELFT>::VersionNeedSection()
    : OutputSectionBase<ELFT>(".gnu.version_r", SHT_GNU_verneed, SHF_ALLOC) {
  this->Header.sh_addralign = sizeof(uint32_t);

  // Identifiers in verneed section start at 2 because 0 and 1 are reserved
  // for VER_NDX_LOCAL and VER_NDX_GLOBAL.
  // First identifiers are reserved by verdef section if it exist.
  NextIndex = getVerDefNum() + 1;
}

template <class ELFT>
void VersionNeedSection<ELFT>::addSymbol(SharedSymbol<ELFT> *SS) {
  if (!SS->Verdef) {
    SS->symbol()->VersionId = VER_NDX_GLOBAL;
    return;
  }
  SharedFile<ELFT> *F = SS->file();
  // If we don't already know that we need an Elf_Verneed for this DSO, prepare
  // to create one by adding it to our needed list and creating a dynstr entry
  // for the soname.
  if (F->VerdefMap.empty())
    Needed.push_back({F, Out<ELFT>::DynStrTab->addString(F->getSoName())});
  typename SharedFile<ELFT>::NeededVer &NV = F->VerdefMap[SS->Verdef];
  // If we don't already know that we need an Elf_Vernaux for this Elf_Verdef,
  // prepare to create one by allocating a version identifier and creating a
  // dynstr entry for the version name.
  if (NV.Index == 0) {
    NV.StrTab = Out<ELFT>::DynStrTab->addString(
        SS->file()->getStringTable().data() + SS->Verdef->getAux()->vda_name);
    NV.Index = NextIndex++;
  }
  SS->symbol()->VersionId = NV.Index;
}

template <class ELFT> void VersionNeedSection<ELFT>::writeTo(uint8_t *Buf) {
  // The Elf_Verneeds need to appear first, followed by the Elf_Vernauxs.
  auto *Verneed = reinterpret_cast<Elf_Verneed *>(Buf);
  auto *Vernaux = reinterpret_cast<Elf_Vernaux *>(Verneed + Needed.size());

  for (std::pair<SharedFile<ELFT> *, size_t> &P : Needed) {
    // Create an Elf_Verneed for this DSO.
    Verneed->vn_version = 1;
    Verneed->vn_cnt = P.first->VerdefMap.size();
    Verneed->vn_file = P.second;
    Verneed->vn_aux =
        reinterpret_cast<char *>(Vernaux) - reinterpret_cast<char *>(Verneed);
    Verneed->vn_next = sizeof(Elf_Verneed);
    ++Verneed;

    // Create the Elf_Vernauxs for this Elf_Verneed. The loop iterates over
    // VerdefMap, which will only contain references to needed version
    // definitions. Each Elf_Vernaux is based on the information contained in
    // the Elf_Verdef in the source DSO. This loop iterates over a std::map of
    // pointers, but is deterministic because the pointers refer to Elf_Verdef
    // data structures within a single input file.
    for (auto &NV : P.first->VerdefMap) {
      Vernaux->vna_hash = NV.first->vd_hash;
      Vernaux->vna_flags = 0;
      Vernaux->vna_other = NV.second.Index;
      Vernaux->vna_name = NV.second.StrTab;
      Vernaux->vna_next = sizeof(Elf_Vernaux);
      ++Vernaux;
    }

    Vernaux[-1].vna_next = 0;
  }
  Verneed[-1].vn_next = 0;
}

template <class ELFT> void VersionNeedSection<ELFT>::finalize() {
  this->Header.sh_link = Out<ELFT>::DynStrTab->SectionIndex;
  this->Header.sh_info = Needed.size();
  unsigned Size = Needed.size() * sizeof(Elf_Verneed);
  for (std::pair<SharedFile<ELFT> *, size_t> &P : Needed)
    Size += P.first->VerdefMap.size() * sizeof(Elf_Vernaux);
  this->Header.sh_size = Size;
}

template <class ELFT>
MipsReginfoOutputSection<ELFT>::MipsReginfoOutputSection()
    : OutputSectionBase<ELFT>(".reginfo", SHT_MIPS_REGINFO, SHF_ALLOC) {
  this->Header.sh_addralign = 4;
  this->Header.sh_entsize = sizeof(Elf_Mips_RegInfo);
  this->Header.sh_size = sizeof(Elf_Mips_RegInfo);
}

template <class ELFT>
void MipsReginfoOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  auto *R = reinterpret_cast<Elf_Mips_RegInfo *>(Buf);
  if (Config->Relocatable)
    R->ri_gp_value = 0;
  else
    R->ri_gp_value = Out<ELFT>::Got->getVA() + MipsGPOffset;
  R->ri_gprmask = GprMask;
}

template <class ELFT>
void MipsReginfoOutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  // Copy input object file's .reginfo gprmask to output.
  auto *S = cast<MipsReginfoInputSection<ELFT>>(C);
  GprMask |= S->Reginfo->ri_gprmask;
  S->OutSec = this;
}

template <class ELFT>
MipsOptionsOutputSection<ELFT>::MipsOptionsOutputSection()
    : OutputSectionBase<ELFT>(".MIPS.options", SHT_MIPS_OPTIONS,
                              SHF_ALLOC | SHF_MIPS_NOSTRIP) {
  this->Header.sh_addralign = 8;
  this->Header.sh_entsize = 1;
  this->Header.sh_size = sizeof(Elf_Mips_Options) + sizeof(Elf_Mips_RegInfo);
}

template <class ELFT>
void MipsOptionsOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  auto *Opt = reinterpret_cast<Elf_Mips_Options *>(Buf);
  Opt->kind = ODK_REGINFO;
  Opt->size = this->Header.sh_size;
  Opt->section = 0;
  Opt->info = 0;
  auto *Reg = reinterpret_cast<Elf_Mips_RegInfo *>(Buf + sizeof(*Opt));
  if (Config->Relocatable)
    Reg->ri_gp_value = 0;
  else
    Reg->ri_gp_value = Out<ELFT>::Got->getVA() + MipsGPOffset;
  Reg->ri_gprmask = GprMask;
}

template <class ELFT>
void MipsOptionsOutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  auto *S = cast<MipsOptionsInputSection<ELFT>>(C);
  if (S->Reginfo)
    GprMask |= S->Reginfo->ri_gprmask;
  S->OutSec = this;
}

template <class ELFT>
MipsAbiFlagsOutputSection<ELFT>::MipsAbiFlagsOutputSection()
    : OutputSectionBase<ELFT>(".MIPS.abiflags", SHT_MIPS_ABIFLAGS, SHF_ALLOC) {
  this->Header.sh_addralign = 8;
  this->Header.sh_entsize = sizeof(Elf_Mips_ABIFlags);
  this->Header.sh_size = sizeof(Elf_Mips_ABIFlags);
  memset(&Flags, 0, sizeof(Flags));
}

template <class ELFT>
void MipsAbiFlagsOutputSection<ELFT>::writeTo(uint8_t *Buf) {
  memcpy(Buf, &Flags, sizeof(Flags));
}

template <class ELFT>
void MipsAbiFlagsOutputSection<ELFT>::addSection(InputSectionBase<ELFT> *C) {
  // Check compatibility and merge fields from input .MIPS.abiflags
  // to the output one.
  auto *S = cast<MipsAbiFlagsInputSection<ELFT>>(C);
  S->OutSec = this;
  if (S->Flags->version != 0) {
    error(getFilename(S->getFile()) + ": unexpected .MIPS.abiflags version " +
          Twine(S->Flags->version));
    return;
  }
  // LLD checks ISA compatibility in getMipsEFlags(). Here we just
  // select the highest number of ISA/Rev/Ext.
  Flags.isa_level = std::max(Flags.isa_level, S->Flags->isa_level);
  Flags.isa_rev = std::max(Flags.isa_rev, S->Flags->isa_rev);
  Flags.isa_ext = std::max(Flags.isa_ext, S->Flags->isa_ext);
  Flags.gpr_size = std::max(Flags.gpr_size, S->Flags->gpr_size);
  Flags.cpr1_size = std::max(Flags.cpr1_size, S->Flags->cpr1_size);
  Flags.cpr2_size = std::max(Flags.cpr2_size, S->Flags->cpr2_size);
  Flags.ases |= S->Flags->ases;
  Flags.flags1 |= S->Flags->flags1;
  Flags.flags2 |= S->Flags->flags2;
  Flags.fp_abi = elf::getMipsFpAbiFlag(Flags.fp_abi, S->Flags->fp_abi,
                                       getFilename(S->getFile()));
}

template <class ELFT>
static typename ELFT::uint getOutFlags(InputSectionBase<ELFT> *S) {
  return S->Flags & ~SHF_GROUP & ~SHF_COMPRESSED;
}

template <class ELFT>
static SectionKey<ELFT::Is64Bits> createKey(InputSectionBase<ELFT> *C,
                                            StringRef OutsecName) {
  typedef typename ELFT::uint uintX_t;
  uintX_t Flags = getOutFlags(C);

  // For SHF_MERGE we create different output sections for each alignment.
  // This makes each output section simple and keeps a single level mapping from
  // input to output.
  // In case of relocatable object generation we do not try to perform merging
  // and treat SHF_MERGE sections as regular ones, but also create different
  // output sections for them to allow merging at final linking stage.
  uintX_t Alignment = 0;
  if (isa<MergeInputSection<ELFT>>(C) ||
      (Config->Relocatable && (C->Flags & SHF_MERGE)))
    Alignment = std::max<uintX_t>(C->Alignment, C->Entsize);

  return SectionKey<ELFT::Is64Bits>{OutsecName, C->Type, Flags, Alignment};
}

template <class ELFT>
std::pair<OutputSectionBase<ELFT> *, bool>
OutputSectionFactory<ELFT>::create(InputSectionBase<ELFT> *C,
                                   StringRef OutsecName) {
  SectionKey<ELFT::Is64Bits> Key = createKey(C, OutsecName);
  return create(Key, C);
}

template <class ELFT>
std::pair<OutputSectionBase<ELFT> *, bool>
OutputSectionFactory<ELFT>::create(const SectionKey<ELFT::Is64Bits> &Key,
                                   InputSectionBase<ELFT> *C) {
  uintX_t Flags = getOutFlags(C);
  OutputSectionBase<ELFT> *&Sec = Map[Key];
  if (Sec) {
    Sec->updateFlags(Flags);
    return {Sec, false};
  }

  uint32_t Type = C->Type;
  switch (C->kind()) {
  case InputSectionBase<ELFT>::Regular:
    Sec = make<OutputSection<ELFT>>(Key.Name, Type, Flags);
    break;
  case InputSectionBase<ELFT>::EHFrame:
    return {Out<ELFT>::EhFrame, false};
  case InputSectionBase<ELFT>::Merge:
    Sec = make<MergeOutputSection<ELFT>>(Key.Name, Type, Flags, Key.Alignment);
    break;
  case InputSectionBase<ELFT>::MipsReginfo:
    Sec = make<MipsReginfoOutputSection<ELFT>>();
    break;
  case InputSectionBase<ELFT>::MipsOptions:
    Sec = make<MipsOptionsOutputSection<ELFT>>();
    break;
  case InputSectionBase<ELFT>::MipsAbiFlags:
    Sec = make<MipsAbiFlagsOutputSection<ELFT>>();
    break;
  }
  return {Sec, true};
}

template <bool Is64Bits>
typename lld::elf::SectionKey<Is64Bits>
DenseMapInfo<lld::elf::SectionKey<Is64Bits>>::getEmptyKey() {
  return SectionKey<Is64Bits>{DenseMapInfo<StringRef>::getEmptyKey(), 0, 0, 0};
}

template <bool Is64Bits>
typename lld::elf::SectionKey<Is64Bits>
DenseMapInfo<lld::elf::SectionKey<Is64Bits>>::getTombstoneKey() {
  return SectionKey<Is64Bits>{DenseMapInfo<StringRef>::getTombstoneKey(), 0, 0,
                              0};
}

template <bool Is64Bits>
unsigned
DenseMapInfo<lld::elf::SectionKey<Is64Bits>>::getHashValue(const Key &Val) {
  return hash_combine(Val.Name, Val.Type, Val.Flags, Val.Alignment);
}

template <bool Is64Bits>
bool DenseMapInfo<lld::elf::SectionKey<Is64Bits>>::isEqual(const Key &LHS,
                                                           const Key &RHS) {
  return DenseMapInfo<StringRef>::isEqual(LHS.Name, RHS.Name) &&
         LHS.Type == RHS.Type && LHS.Flags == RHS.Flags &&
         LHS.Alignment == RHS.Alignment;
}

namespace llvm {
template struct DenseMapInfo<SectionKey<true>>;
template struct DenseMapInfo<SectionKey<false>>;
}

namespace lld {
namespace elf {
template class OutputSectionBase<ELF32LE>;
template class OutputSectionBase<ELF32BE>;
template class OutputSectionBase<ELF64LE>;
template class OutputSectionBase<ELF64BE>;

template class EhFrameHeader<ELF32LE>;
template class EhFrameHeader<ELF32BE>;
template class EhFrameHeader<ELF64LE>;
template class EhFrameHeader<ELF64BE>;

template class GotPltSection<ELF32LE>;
template class GotPltSection<ELF32BE>;
template class GotPltSection<ELF64LE>;
template class GotPltSection<ELF64BE>;

template class GotSection<ELF32LE>;
template class GotSection<ELF32BE>;
template class GotSection<ELF64LE>;
template class GotSection<ELF64BE>;

template class PltSection<ELF32LE>;
template class PltSection<ELF32BE>;
template class PltSection<ELF64LE>;
template class PltSection<ELF64BE>;

template class RelocationSection<ELF32LE>;
template class RelocationSection<ELF32BE>;
template class RelocationSection<ELF64LE>;
template class RelocationSection<ELF64BE>;

template class GnuHashTableSection<ELF32LE>;
template class GnuHashTableSection<ELF32BE>;
template class GnuHashTableSection<ELF64LE>;
template class GnuHashTableSection<ELF64BE>;

template class HashTableSection<ELF32LE>;
template class HashTableSection<ELF32BE>;
template class HashTableSection<ELF64LE>;
template class HashTableSection<ELF64BE>;

template class DynamicSection<ELF32LE>;
template class DynamicSection<ELF32BE>;
template class DynamicSection<ELF64LE>;
template class DynamicSection<ELF64BE>;

template class OutputSection<ELF32LE>;
template class OutputSection<ELF32BE>;
template class OutputSection<ELF64LE>;
template class OutputSection<ELF64BE>;

template class EhOutputSection<ELF32LE>;
template class EhOutputSection<ELF32BE>;
template class EhOutputSection<ELF64LE>;
template class EhOutputSection<ELF64BE>;

template class MipsReginfoOutputSection<ELF32LE>;
template class MipsReginfoOutputSection<ELF32BE>;
template class MipsReginfoOutputSection<ELF64LE>;
template class MipsReginfoOutputSection<ELF64BE>;

template class MipsOptionsOutputSection<ELF32LE>;
template class MipsOptionsOutputSection<ELF32BE>;
template class MipsOptionsOutputSection<ELF64LE>;
template class MipsOptionsOutputSection<ELF64BE>;

template class MipsAbiFlagsOutputSection<ELF32LE>;
template class MipsAbiFlagsOutputSection<ELF32BE>;
template class MipsAbiFlagsOutputSection<ELF64LE>;
template class MipsAbiFlagsOutputSection<ELF64BE>;

template class MergeOutputSection<ELF32LE>;
template class MergeOutputSection<ELF32BE>;
template class MergeOutputSection<ELF64LE>;
template class MergeOutputSection<ELF64BE>;

template class StringTableSection<ELF32LE>;
template class StringTableSection<ELF32BE>;
template class StringTableSection<ELF64LE>;
template class StringTableSection<ELF64BE>;

template class SymbolTableSection<ELF32LE>;
template class SymbolTableSection<ELF32BE>;
template class SymbolTableSection<ELF64LE>;
template class SymbolTableSection<ELF64BE>;

template class VersionTableSection<ELF32LE>;
template class VersionTableSection<ELF32BE>;
template class VersionTableSection<ELF64LE>;
template class VersionTableSection<ELF64BE>;

template class VersionNeedSection<ELF32LE>;
template class VersionNeedSection<ELF32BE>;
template class VersionNeedSection<ELF64LE>;
template class VersionNeedSection<ELF64BE>;

template class VersionDefinitionSection<ELF32LE>;
template class VersionDefinitionSection<ELF32BE>;
template class VersionDefinitionSection<ELF64LE>;
template class VersionDefinitionSection<ELF64BE>;

template class GdbIndexSection<ELF32LE>;
template class GdbIndexSection<ELF32BE>;
template class GdbIndexSection<ELF64LE>;
template class GdbIndexSection<ELF64BE>;

template class OutputSectionFactory<ELF32LE>;
template class OutputSectionFactory<ELF32BE>;
template class OutputSectionFactory<ELF64LE>;
template class OutputSectionFactory<ELF64BE>;
}
}
