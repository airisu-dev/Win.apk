// context.h -- pc and sp of a Windows CONTEXT on either host.

#pragma once

#include "winapk/arch.h"

#include <cstdint>

#include <windows.h>

namespace winapk {

inline uint64_t context_pc(const CONTEXT& c) {
#if WINAPK_HOST_X64
    return c.Rip;
#else
    return c.Pc;
#endif
}

inline uint64_t context_sp(const CONTEXT& c) {
#if WINAPK_HOST_X64
    return c.Rsp;
#else
    return c.Sp;
#endif
}

}  // namespace winapk
