#include "social/benchmark_memory.hpp"
#include <cstdint>
#include <iostream>
#include <new>
#include <stdexcept>

int main() {
    auto check = [](bool ok) {
        if (!ok)
            throw std::runtime_error("heap-meter invariant failed");
    };
    void *input = ::operator new(4096);
    social::benchmark_memory::begin();
    ::operator delete(input); // Pre-existing inputs must never underflow the meter.
    void *first = ::operator new(1024);
    void *second = ::operator new[](512);
    ::operator delete(first);
    void *third = ::operator new(128);
    ::operator delete[](second);
    ::operator delete(third);
    check(social::benchmark_memory::finish() == 1536);
    social::benchmark_memory::begin();
    auto *aligned = ::operator new(256, std::align_val_t(64));
    check(reinterpret_cast<std::uintptr_t>(aligned) % 64 == 0);
    ::operator delete(aligned, std::align_val_t(64));
    check(social::benchmark_memory::finish() == 256);
    social::benchmark_memory::begin();
    auto *retained = ::operator new(50);
    check(social::benchmark_memory::finish() == 50);
    social::benchmark_memory::begin();
    ::operator delete(retained); // A previous epoch is not this call's allocation.
    check(social::benchmark_memory::finish() == 0);
    std::cout << "Passed 4 heap-meter invariants\n";
}
