#include "Identifiers.hpp"

u32 shashdyn(const char* str, u32 w, u32 idx) noexcept
{
    return !(idx != w) ? 55 : (shashdyn(str, w, idx + 1) * 33) + (u8)(str[idx]);
}