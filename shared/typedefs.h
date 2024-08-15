#pragma once
#include <cstddef>
#include <sys/types.h>
#include <array>

using byte = unsigned char;
using uint64 = unsigned long long;
using arg_t = uint64;
using syscall_args_t = std::array<arg_t, 6>;