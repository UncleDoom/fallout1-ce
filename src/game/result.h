#pragma once

#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace fallout {

// Error codes for the game engine, replacing integer return codes.
enum class Error {
    Success = 0,
    GenericFailure = -1,
    FileNotFound,
    InvalidData,
    OutOfMemory,
    OutOfRange,
    AlreadyExists,
    NotInitialized,
};

// Lightweight Result<T, E> template for replacing integer return codes.
// On C++17, this is implemented with std::optional + an error code.
//
// Usage (returning a value):
//   Result<int> computeValue() {
//       if (bad) return Error::InvalidData;
//       return 42;
//   }
//
// Usage (void return, just success/error):
//   Result<void> doWork() {
//       if (bad) return Error::GenericFailure;
//       return {};
//   }
//
// Checking:
//   auto result = computeValue();
//   if (!result) { handleError(result.error()); }
//   int val = result.value();
//
// Early return (like Rust's `?` operator):
//   Result<int> doWork() {
//       int val = FO_TRY(computeValue());
//       return val + 1;
//   }
//
template <typename T, typename E = Error>
class Result {
public:
    // Implicit construction from value (success)
    Result(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value))
        , error_(E{})
    {
    }

    // Implicit construction from error
    Result(E error) noexcept
        : value_(std::nullopt)
        , error_(error)
    {
    }

    explicit operator bool() const noexcept { return value_.has_value(); }
    [[nodiscard]] bool hasValue() const noexcept { return value_.has_value(); }

    [[nodiscard]] const T& value() const& { return value_.value(); }
    [[nodiscard]] T& value() & { return value_.value(); }
    [[nodiscard]] T&& value() && { return std::move(value_).value(); }

    [[nodiscard]] const T& valueOr(const T& defaultVal) const& { return value_.value_or(defaultVal); }

    [[nodiscard]] E error() const noexcept { return error_; }

    /// Transform the contained value with `f`, propagating errors unchanged.
    /// Usage: result.map([](int x) { return x * 2; })
    template <typename F>
    [[nodiscard]] auto map(F&& f) const& -> Result<std::invoke_result_t<F, const T&>, E>
    {
        if (value_.has_value()) {
            return std::invoke(std::forward<F>(f), value_.value());
        }
        return error_;
    }

    /// Chain a computation that itself returns a Result.
    /// Usage: result.andThen([](int x) -> Result<std::string> { ... })
    template <typename F>
    [[nodiscard]] auto andThen(F&& f) const& -> std::invoke_result_t<F, const T&>
    {
        if (value_.has_value()) {
            return std::invoke(std::forward<F>(f), value_.value());
        }
        return error_;
    }

private:
    std::optional<T> value_;
    E error_;
};

// Specialization for void results (just success/failure with no value).
template <typename E>
class Result<void, E> {
public:
    // Default construction = success
    Result() noexcept
        : error_(E{})
        , success_(true)
    {
    }

    // Implicit construction from error
    Result(E error) noexcept
        : error_(error)
        , success_(false)
    {
    }

    explicit operator bool() const noexcept { return success_; }
    [[nodiscard]] bool hasValue() const noexcept { return success_; }
    [[nodiscard]] E error() const noexcept { return error_; }

    /// Chain a computation that returns a Result, only if this result is successful.
    template <typename F>
    [[nodiscard]] auto andThen(F&& f) const -> std::invoke_result_t<F>
    {
        if (success_) {
            return std::invoke(std::forward<F>(f));
        }
        return error_;
    }

private:
    E error_;
    bool success_;
};

/// Early-return macro for Result types, analogous to Rust's `?` operator.
/// Evaluates `expr`, returns the error if it failed, otherwise yields the value.
///
/// Usage:
///   Result<int> computeValue();
///   Result<int> doWork() {
///       int val = FO_TRY(computeValue());
///       return val + 1;
///   }
///
/// For Result<void>:
///   FO_TRY_VOID(doSomething());
///
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define FO_TRY(expr)                                              \
    ({                                                            \
        auto _fo_result = (expr);                                 \
        if (!_fo_result) return _fo_result.error();               \
        std::move(_fo_result).value();                            \
    })

#define FO_TRY_VOID(expr)                                        \
    do {                                                          \
        auto _fo_result = (expr);                                 \
        if (!_fo_result) return _fo_result.error();               \
    } while (0)
// NOLINTEND(cppcoreguidelines-macro-usage)

} // namespace fallout
