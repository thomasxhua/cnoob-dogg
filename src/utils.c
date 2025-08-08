#include "utils.h"

#include <stdio.h>

// "Determining if an integer is a power of 2", Anderson, _Bit Twiddling Hacks_
// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool is_power_of_two(uint64_t n)
{
    return n && !(n & (n - 1));
}

char* uint64_to_string(uint64_t n)
{
    static char str[21] = {0};
    snprintf(str, sizeof(str), "%llu", n);
    return str;
}

