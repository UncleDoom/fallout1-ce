#pragma once

/// @file raii.h
/// @brief RAII utility types for safe resource management.
///
/// Provides smart-pointer-like wrappers for legacy C-style resource pairs:
/// - DbFileGuard: RAII wrapper for DB_FILE* (db_fopen / db_fclose)
/// - MemBuffer<T>: RAII wrapper for mem_malloc / mem_free allocations
/// - MemDeleter: Custom deleter for use with std::unique_ptr

#include <cstddef>
#include <memory>
#include <utility>

#include "plib/db/db.h"
#include "plib/gnw/memory.h"

namespace fallout {

// ---------------------------------------------------------------------------
// MemDeleter — Custom deleter that calls mem_free.
// Use with std::unique_ptr for single objects or arrays allocated via
// mem_malloc.
//
// Example:
//   auto buf = std::unique_ptr<unsigned char[], MemDeleter>(
//       static_cast<unsigned char*>(mem_malloc(size)));
// ---------------------------------------------------------------------------
struct MemDeleter {
    void operator()(void* p) const noexcept {
        if (p) mem_free(p);
    }
};

/// Convenience alias: a unique_ptr<T[]> that frees via mem_free.
template <typename T>
using MemBuffer = std::unique_ptr<T[], MemDeleter>;

/// Convenience alias: a unique_ptr<T> (single object) that frees via mem_free.
template <typename T>
using MemPtr = std::unique_ptr<T, MemDeleter>;

/// Helper to allocate a MemBuffer<T> of `count` elements via mem_malloc.
/// Returns an empty MemBuffer on allocation failure.
template <typename T>
[[nodiscard]] MemBuffer<T> makeMemBuffer(std::size_t count) {
    auto* p = static_cast<T*>(mem_malloc(count * sizeof(T)));
    return MemBuffer<T>(p);
}

// ---------------------------------------------------------------------------
// DbFileGuard — RAII wrapper for DB_FILE* streams.
//
// Automatically calls db_fclose when the guard goes out of scope.
// Movable but not copyable.
//
// Example:
//   DbFileGuard file(db_fopen("data/proto.lst", "rb"));
//   if (!file) return -1;
//   db_fread(buf, 1, size, file.get());
//   // db_fclose called automatically
// ---------------------------------------------------------------------------
class DbFileGuard {
public:
    DbFileGuard() noexcept = default;

    explicit DbFileGuard(DB_FILE* stream) noexcept
        : stream_(stream) {}

    ~DbFileGuard() {
        if (stream_ != nullptr) {
            stream_->fclose();
        }
    }

    // Move-only
    DbFileGuard(DbFileGuard&& other) noexcept
        : stream_(std::exchange(other.stream_, nullptr)) {}

    DbFileGuard& operator=(DbFileGuard&& other) noexcept {
        if (this != &other) {
            if (stream_ != nullptr) {
                stream_->fclose();
            }
            stream_ = std::exchange(other.stream_, nullptr);
        }
        return *this;
    }

    DbFileGuard(const DbFileGuard&) = delete;
    DbFileGuard& operator=(const DbFileGuard&) = delete;

    /// Returns the underlying stream pointer.
    [[nodiscard]] DB_FILE* get() const noexcept { return stream_; }

    /// Boolean conversion — true if stream is valid.
    explicit operator bool() const noexcept { return stream_ != nullptr; }

    /// Release ownership without closing.
    DB_FILE* release() noexcept { return std::exchange(stream_, nullptr); }

    /// Close the stream early and reset to null.
    void reset(DB_FILE* stream = nullptr) noexcept {
        if (stream_) {
            stream_->fclose();
        }
        stream_ = stream;
    }

private:
    DB_FILE* stream_ = nullptr;
};

} // namespace fallout
