#pragma once

#include <string.h>
#include "lib.hpp"

// Constexpr cstring hash
constexpr u32 shash(const char* str, u32 idx = 0) noexcept
{
    return !str[idx] ? 55 : (shash(str, idx + 1) * 33) + (u8)(str[idx]);
}

// Hash of a string that isn't null terminated, but with a predefined width
u32 shashdyn(const char* str, u32 w, u32 idx = 0) noexcept;
