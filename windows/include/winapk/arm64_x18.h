// arm64_x18.h -- does an arm64 library's code use register x18?
//
// Windows on Arm keeps the TEB in x18; an exception taken while it holds
// anything else ends the process. NDK code leaves x18 alone, but some code uses
// it as scratch (the shadow call stack: `str x30, [x18], #8`; hand-written
// assembly), and such a library cannot run natively. This finds it at load.
//
// uses_x18 decodes the general-register fields of one A64 instruction (31 is
// SP/XZR; v18 is not x18). Exact for what compilers emit, "yes" when unsure.

#pragma once

#include "winapk/elf64_defs.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace winapk::arm64 {

namespace x18_detail {

constexpr uint32_t kX18 = 18;
inline constexpr uint32_t bits(uint32_t v, unsigned hi, unsigned lo) { return (v >> lo) & ((1u << (hi - lo + 1)) - 1); }
inline constexpr bool is18(uint32_t r) { return r == kX18; }

}  // namespace x18_detail

inline bool uses_x18(uint32_t i) {
    using namespace x18_detail;
    const uint32_t rd = bits(i, 4, 0), rn = bits(i, 9, 5), rm = bits(i, 20, 16), ra = bits(i, 14, 10);
    const uint32_t op0 = bits(i, 28, 25);
    const bool any3 = is18(rd) || is18(rn) || is18(rm);  // for the classes below that are decoded only loosely

    // Data processing, immediate: 100x.
    if ((op0 & 0xe) == 0x8) {
        if (bits(i, 28, 24) == 0x10) return is18(rd);  // ADR, ADRP: Rn is an immediate
        switch (bits(i, 25, 23)) {
            case 0x5: return is18(rd);                         // move wide
            case 0x7: return is18(rd) || is18(rn) || is18(rm);  // extract
            default: return is18(rd) || is18(rn);               // add/sub (with tags), logical, bitfield
        }
    }

    // Branches, system: 101x.
    if ((op0 & 0xe) == 0xa) {
        if (bits(i, 30, 26) == 0x05) return false;                                 // B, BL
        if (bits(i, 31, 25) == 0x2a) return false;                                 // B.cond
        if (bits(i, 31, 24) == 0xd4) return false;                                 // exceptions
        if (bits(i, 30, 25) == 0x1a || bits(i, 30, 25) == 0x1b) return is18(rd);   // CBZ/CBNZ, TBZ/TBNZ: Rt
        if (bits(i, 31, 25) == 0x6b) return is18(rn) || is18(rd);                  // BR, BLR, RET, BRAA..
        if (bits(i, 31, 22) == 0x354) return is18(rd);                             // MSR, MRS, SYS, SYSL, hints: Rt
        if (bits(i, 31, 22) == 0x355) return is18(rd) || bits(i, 4, 0) == 17;      // MRRS, MSRR, SYSP: Rt and Rt+1
        return any3;                                                               // newer branch forms (compare and branch)
    }

    // Loads and stores: x1x0.
    if ((op0 & 0x5) == 0x4) {
        const bool v = bits(i, 26, 26) != 0;  // a SIMD/FP register is the data register
        const uint32_t cls = bits(i, 29, 27);
        const bool data = !v && is18(rd);
        // PRFM (immediate, register, unscaled) and its literal form: Rt is the prefetch operation.
        const uint32_t op4 = bits(i, 11, 10);
        const bool prfm = bits(i, 31, 30) == 3 && !v &&
                          (cls == 3 ? bits(i, 25, 24) == 0
                                    : (cls == 7 && bits(i, 23, 22) == 2 &&
                                       (bits(i, 24, 24) || (!bits(i, 21, 21) && op4 == 0) || (bits(i, 21, 21) && op4 == 2))));
        // Memory copy and set (FEAT_MOPS): Xd, Xs, Xn are all general registers.
        if (cls == 0x3 && bits(i, 25, 24) == 1 && !bits(i, 21, 21) && op4 == 1 && !bits(i, 31, 31)) return any3;
        if (cls == 0x5) return data || (!v && is18(ra)) || is18(rn);  // pairs: Rt, Rt2, Rn
        if (cls == 0x3) return (prfm ? false : data) || (bits(i, 25, 24) == 0 ? false : is18(rn));  // literal, and LDAPUR forms
        if (cls == 0x1) {
            if (v) return is18(rn) || (bits(i, 23, 23) && is18(rm));  // SIMD structures: base, post-index register
            return data || is18(rm) || is18(ra) || is18(rn);           // exclusive and ordered: Rs, Rt2, Rn, Rt
        }
        if (cls == 0x7) {
            const bool rt = prfm ? false : data;
            if (bits(i, 24, 24)) return rt || is18(rn);  // unsigned offset
            if (bits(i, 21, 21)) {                       // register offset, atomics, pointer-authenticated
                if (op4 == 0x2) return rt || is18(rn) || is18(rm);           // register offset: Rm is always a general register
                if (op4 == 0x0) return rt || is18(rn) || (!v && is18(rm));   // atomic: Rs
                return rt || is18(rn);                                         // LDRAA, LDRAB
            }
            return rt || is18(rn);  // immediate offset, pre/post index, unprivileged, ordered
        }
        return data || is18(rn);  // anything else in the class (tags, ...): the data register and the base
    }

    // Data processing, register: x101.
    if ((op0 & 0x7) == 0x5) {
        const uint32_t top = bits(i, 28, 21);
        if (top == 0xd2) return is18(rn) || (!bits(i, 11, 11) && is18(rm));  // conditional compare: imm5 or Rm
        bool use_rm = true;
        if (bits(i, 30, 30) && !bits(i, 29, 29) && top == 0xd6) use_rm = false;  // one source: opcode2
        const bool three = bits(i, 28, 24) == 0x1b;                              // three source: Ra
        return is18(rd) || is18(rn) || (use_rm && is18(rm)) || (three && is18(ra));
    }

    // Data processing, SIMD and FP: x111. Only the moves between the general
    // and the vector registers name a general register.
    if ((op0 & 0x7) == 0x7) {
        if (bits(i, 28, 24) == 0x1e && bits(i, 30, 29) == 0 && (bits(i, 21, 21) == 0 || bits(i, 15, 10) == 0)) {
            // Conversions between FP and integer (bit 21 set, bits 15..10 clear) and fixed point
            // (bit 21 clear): the general register is Rn for SCVTF, UCVTF and FMOV to FP (opcodes
            // 010, 011, 111), else Rd.
            const uint32_t opcode = bits(i, 18, 16);
            const bool gpr_in_rn = opcode == 2 || opcode == 3 || (bits(i, 21, 21) && opcode == 7);
            return gpr_in_rn ? is18(rn) : is18(rd);
        }
        if (bits(i, 31, 31) == 0 && bits(i, 29, 29) == 0 && bits(i, 28, 21) == 0x70 && bits(i, 15, 15) == 0 &&
            bits(i, 10, 10) == 1) {  // SIMD copy
            const uint32_t imm4 = bits(i, 14, 11);
            if (imm4 == 1 || imm4 == 3) return is18(rn);  // DUP, INS from a general register
            if (imm4 == 5 || imm4 == 7) return is18(rd);  // SMOV, UMOV to a general register
        }
        return false;
    }

    // SVE and SME (op0 0010, and 0000 with bit 31 set) name general registers
    // as bases, indexes and counters in more forms than are worth decoding:
    // any of the three register fields, unless the word is plain data. No
    // Android toolchain emits these without being asked for them.
    if (op0 == 0x2 || (op0 == 0x0 && bits(i, 31, 31))) return any3;

    return false;  // reserved
}
struct X18Scan {
    size_t uses = 0;          // instructions that name x18
    size_t first_offset = 0;  // byte offset in the scanned range of the first
    bool found() const { return uses != 0; }
};

// Scans `len` bytes of little-endian A64 code (4-byte aligned) at `code`.
inline X18Scan scan_x18(const uint8_t* code, size_t len) {
    X18Scan r;
    for (size_t at = 0; at + 4 <= len; at += 4) {
        const uint32_t w = static_cast<uint32_t>(code[at]) | static_cast<uint32_t>(code[at + 1]) << 8 |
                           static_cast<uint32_t>(code[at + 2]) << 16 | static_cast<uint32_t>(code[at + 3]) << 24;
        if (uses_x18(w)) {
            if (!r.uses) r.first_offset = at;
            ++r.uses;
        }
    }
    return r;
}

// The scan of an arm64 ELF file's code. An executable segment often starts at file offset 0 and holds
// the headers, symbol tables and strings along with the code (lld without -z separate-code), and
// those are not instructions; so the executable sections (SHF_EXECINSTR) are what is scanned when the
// file has section headers, and the whole executable segments when it has not.
// first_offset is a file offset. by_sections tells which of the two was done.
struct ElfX18Scan {
    X18Scan scan;
    bool by_sections = false;
    bool ok = false;  // false: not a readable arm64 ELF file
};

inline ElfX18Scan scan_elf_x18(const uint8_t* file, size_t size) {
    ElfX18Scan r;
    elf::Ehdr eh;
    if (size < sizeof eh) return r;
    std::memcpy(&eh, file, sizeof eh);
    if (eh.e_ident[0] != elf::kMag0 || eh.e_ident[1] != elf::kMag1 || eh.e_ident[2] != elf::kMag2 ||
        eh.e_ident[3] != elf::kMag3 || eh.e_machine != elf::kEmAarch64)
        return r;
    r.ok = true;
    auto add = [&](uint64_t off, uint64_t len) {
        if (off > size || len > size - off) return;
        X18Scan s = scan_x18(file + off, static_cast<size_t>(len));
        if (s.found() && !r.scan.uses) r.scan.first_offset = static_cast<size_t>(off) + s.first_offset;
        r.scan.uses += s.uses;
    };
    bool sections = false;
    if (eh.e_shoff && eh.e_shnum && eh.e_shentsize >= sizeof(elf::Shdr) && eh.e_shoff <= size &&
        static_cast<uint64_t>(eh.e_shnum) * eh.e_shentsize <= size - eh.e_shoff) {
        for (unsigned i = 0; i < eh.e_shnum; ++i) {
            elf::Shdr sh;
            std::memcpy(&sh, file + eh.e_shoff + static_cast<size_t>(i) * eh.e_shentsize, sizeof sh);
            if (sh.sh_type == elf::kShtProgbits && (sh.sh_flags & elf::kShfExecinstr)) {
                sections = true;
                add(sh.sh_offset, sh.sh_size);
            }
        }
    }
    r.by_sections = sections;
    if (!sections) {
        for (unsigned i = 0; i < eh.e_phnum; ++i) {
            elf::Phdr ph;
            const uint64_t at = eh.e_phoff + static_cast<uint64_t>(i) * eh.e_phentsize;
            if (at > size || size - at < sizeof ph) break;
            std::memcpy(&ph, file + at, sizeof ph);
            if (ph.p_type == elf::kPtLoad && (ph.p_flags & elf::kPfX)) add(ph.p_offset, ph.p_filesz);
        }
    }
    return r;
}

}  // namespace winapk::arm64
