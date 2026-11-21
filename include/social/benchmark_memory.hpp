#pragma once
#include <cstddef>
namespace social::benchmark_memory {
// Single-thread benchmark instrumentation. Tracks outstanding bytes requested
// through C++ new/new[] during one measured call; inputs allocated earlier,
// malloc-only allocations, stack and allocator metadata are excluded.
void begin() noexcept;
std::size_t finish() noexcept;
} // namespace social::benchmark_memory
