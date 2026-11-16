// arm64_native.cpp -- the arm64 guest interface (arm64_guest.h, arm64_aot.h)
// on an arm64 host, where guest code runs as it is: call() is direct and there
// are no translations.
//
// Not done yet: the AAPCS64 to Windows ARM64
// differences at the boundary (variadic host functions and va_list takers:
// register_va_list_function only remembers them), the TLSDESC resolver, the
// TPIDR_EL0 block of each thread, and mapping the library's code executable.

#include "winapk/arch.h"

#if !WINAPK_HOST_ARM64
#error "arm64_native.cpp is for an arm64 host"
#endif

#include "winapk/arm64_aot.h"
#include "winapk/arm64_guest.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <set>
#include <utility>
#include <vector>

namespace winapk::arm64 {

namespace {

struct Range {
    uintptr_t lo, hi;
};
std::mutex g_lock;
std::vector<Range> g_code;
std::set<std::pair<const void*, unsigned>> g_va_list_functions;
void (*g_loaded_hook)() = nullptr;

}  // namespace

void init() {}

void register_code(const void* start, size_t size) {
    std::lock_guard<std::mutex> hold(g_lock);
    const auto lo = reinterpret_cast<uintptr_t>(start);
    g_code.push_back({lo, lo + size});
}

void unregister_code(const void* start) {
    std::lock_guard<std::mutex> hold(g_lock);
    const auto lo = reinterpret_cast<uintptr_t>(start);
    g_code.erase(std::remove_if(g_code.begin(), g_code.end(), [lo](const Range& r) { return r.lo == lo; }),
                 g_code.end());
}

bool is_guest_code(const void* p) {
    const auto a = reinterpret_cast<uintptr_t>(p);
    std::lock_guard<std::mutex> hold(g_lock);
    for (const Range& r : g_code) {
        if (a >= r.lo && a < r.hi) return true;
    }
    return false;
}

uint64_t call(const void* fn, const uint64_t* args, size_t nargs) {
    uint64_t a[8] = {};
    if (args) std::memcpy(a, args, std::min<size_t>(nargs, 8) * sizeof(uint64_t));
    using Fn = uint64_t (*)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
    return reinterpret_cast<Fn>(const_cast<void*>(fn))(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
}

void* host_callable(void* fn) { return fn; }

void register_va_list_function(const void* host_fn, unsigned index) {
    std::lock_guard<std::mutex> hold(g_lock);
    g_va_list_functions.insert({host_fn, index});
}

bool set_engine(Engine e) { return e == Engine::kAot; }
Engine engine() { return Engine::kAot; }

Stats stats() { return {}; }
void set_profiling(bool) {}
void print_stats_if_enabled() {}

// setjmp, longjmp and the TLSDESC resolver work on registers: the shim's
// setjmp.cpp has native versions, and the resolver is still to be written.
void* guest_intrinsic(const char*) { return nullptr; }

// abi_override and linux_syscall are in arm64_abi.cpp.

// ---- translations: none on this host --------------------------------------------------

bool aot_load(const char*, uint64_t, uint64_t, uint64_t, uint64_t) { return false; }
void aot_unload(uint64_t, uint64_t) {}
void aot_code_written(uint64_t) {}
void aot_profile_hit(uint64_t) {}
size_t aot_profile_flush() { return 0; }
size_t aot_load_pending() { return 0; }
void aot_disable() {}
bool aot_disabled() { return true; }
void aot_set_loaded_hook(void (*hook)()) { g_loaded_hook = hook; }
AotFn aot_lookup(uint64_t) { return nullptr; }

}  // namespace winapk::arm64
