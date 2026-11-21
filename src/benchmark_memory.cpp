#include "social/benchmark_memory.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>

namespace {
struct Meter {
    bool enabled = false;
    std::size_t epoch = 0, live = 0, peak = 0;
} meter;
struct Header {
    void *allocation;
    std::size_t bytes, epoch;
};
void *allocate(std::size_t bytes, std::size_t alignment) {
    const std::size_t payload = bytes ? bytes : 1;
    if (payload > std::numeric_limits<std::size_t>::max() - alignment - sizeof(Header))
        throw std::bad_alloc();
    void *base = std::malloc(payload + alignment + sizeof(Header));
    if (!base)
        throw std::bad_alloc();
    auto raw = reinterpret_cast<std::uintptr_t>(base) + sizeof(Header);
    auto aligned = (raw + alignment - 1) & ~(static_cast<std::uintptr_t>(alignment) - 1);
    auto *header = reinterpret_cast<Header *>(aligned - sizeof(Header));
    *header = {base, bytes, meter.enabled ? meter.epoch : 0};
    if (meter.enabled) {
        meter.live += bytes;
        if (meter.live > meter.peak)
            meter.peak = meter.live;
    }
    return reinterpret_cast<void *>(aligned);
}
void release(void *pointer) noexcept {
    if (!pointer)
        return;
    auto *header = reinterpret_cast<Header *>(static_cast<char *>(pointer) - sizeof(Header));
    if (header->epoch && header->epoch == meter.epoch)
        meter.live -= header->bytes;
    std::free(header->allocation);
}
} // namespace
namespace social::benchmark_memory {
void begin() noexcept {
    ++meter.epoch;
    meter.live = meter.peak = 0;
    meter.enabled = true;
}
std::size_t finish() noexcept {
    meter.enabled = false;
    return meter.peak;
}
} // namespace social::benchmark_memory
void *operator new(std::size_t n) {
    return allocate(n, alignof(std::max_align_t));
}
void *operator new[](std::size_t n) {
    return ::operator new(n);
}
void operator delete(void *p) noexcept {
    release(p);
}
void operator delete[](void *p) noexcept {
    release(p);
}
void operator delete(void *p, std::size_t) noexcept {
    release(p);
}
void operator delete[](void *p, std::size_t) noexcept {
    release(p);
}
void *operator new(std::size_t n, std::align_val_t a) {
    return allocate(n, static_cast<std::size_t>(a));
}
void *operator new[](std::size_t n, std::align_val_t a) {
    return ::operator new(n, a);
}
void operator delete(void *p, std::align_val_t) noexcept {
    release(p);
}
void operator delete[](void *p, std::align_val_t) noexcept {
    release(p);
}
void operator delete(void *p, std::size_t, std::align_val_t) noexcept {
    release(p);
}
void operator delete[](void *p, std::size_t, std::align_val_t) noexcept {
    release(p);
}
void *operator new(std::size_t n, const std::nothrow_t &) noexcept {
    try {
        return ::operator new(n);
    } catch (...) {
        return nullptr;
    }
}
void *operator new[](std::size_t n, const std::nothrow_t &) noexcept {
    try {
        return ::operator new[](n);
    } catch (...) {
        return nullptr;
    }
}
void operator delete(void *p, const std::nothrow_t &) noexcept {
    release(p);
}
void operator delete[](void *p, const std::nothrow_t &) noexcept {
    release(p);
}
void *operator new(std::size_t n, std::align_val_t a, const std::nothrow_t &) noexcept {
    try {
        return ::operator new(n, a);
    } catch (...) {
        return nullptr;
    }
}
void *operator new[](std::size_t n, std::align_val_t a, const std::nothrow_t &) noexcept {
    try {
        return ::operator new[](n, a);
    } catch (...) {
        return nullptr;
    }
}
void operator delete(void *p, std::align_val_t, const std::nothrow_t &) noexcept {
    release(p);
}
void operator delete[](void *p, std::align_val_t, const std::nothrow_t &) noexcept {
    release(p);
}
