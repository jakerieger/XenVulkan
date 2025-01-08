// Author: Jake Rieger
// Created: 1/7/2025.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

namespace x {
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

#if defined(__GNUC__) || defined(__clang__)
    using u128 = __uint128_t;
    using i128 = __int128_t;
#endif

    using f32 = float;
    using f64 = double;

    using cstr = const char*;
    using str  = std::string;
    using wstr = std::wstring;

#define CAST static_cast
#define CCAST const_cast
#define DCAST dynamic_cast
#define RCAST reinterpret_cast
}  // namespace x