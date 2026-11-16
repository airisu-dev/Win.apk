// arch.h -- which architecture the Win.apk host is built for.
//
//   WINAPK_HOST_X64: x86_64 guest code runs natively (System V calls through
//     sysv_abi); arm64 guest code on the interpreter, JIT or AOT translation.
//   WINAPK_HOST_ARM64: arm64 guest code runs natively; no System V thunks and
//     no %fs patch.
// Architecture-specific code tests these, not __x86_64__ or _M_ARM64.

#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#define WINAPK_HOST_X64 1
#define WINAPK_HOST_ARM64 0
#elif defined(__aarch64__) || defined(_M_ARM64)
#define WINAPK_HOST_X64 0
#define WINAPK_HOST_ARM64 1
#else
#error "Win.apk builds for Windows x64 or Windows ARM64 only."
#endif
