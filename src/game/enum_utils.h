#pragma once

// Utility header for C++17 enum modernization.
// Provides helpers for working with enum class types.

#include <type_traits>

namespace fallout {

// Convert an enum class value to its underlying integer type.
// Usage: to_underlying(MyEnum::Value) instead of static_cast<int>(MyEnum::Value)
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

// Macro to define bitwise operators for flag-style enum classes.
// Use after the enum class definition, inside the namespace.
//
// Example:
//   enum class MyFlags : unsigned int { A = 0x01, B = 0x02, C = 0x04 };
//   DEFINE_ENUM_FLAG_OPERATORS(MyFlags)
//
#define DEFINE_ENUM_FLAG_OPERATORS(ENUM_TYPE)                                                           \
    inline constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept                        \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(to_underlying(lhs) | to_underlying(rhs));                        \
    }                                                                                                  \
    inline constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept                        \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(to_underlying(lhs) & to_underlying(rhs));                        \
    }                                                                                                  \
    inline constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept                        \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(to_underlying(lhs) ^ to_underlying(rhs));                        \
    }                                                                                                  \
    inline constexpr ENUM_TYPE operator~(ENUM_TYPE val) noexcept                                       \
    {                                                                                                  \
        return static_cast<ENUM_TYPE>(~to_underlying(val));                                            \
    }                                                                                                  \
    inline constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept                     \
    {                                                                                                  \
        return lhs = lhs | rhs;                                                                        \
    }                                                                                                  \
    inline constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept                     \
    {                                                                                                  \
        return lhs = lhs & rhs;                                                                        \
    }                                                                                                  \
    inline constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept                     \
    {                                                                                                  \
        return lhs = lhs ^ rhs;                                                                        \
    }                                                                                                  \
    inline constexpr bool hasFlag(ENUM_TYPE value, ENUM_TYPE flag) noexcept                             \
    {                                                                                                  \
        return (to_underlying(value) & to_underlying(flag)) != 0;                                      \
    }

} // namespace fallout
