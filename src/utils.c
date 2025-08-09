#include "utils.h"

#include <stdio.h>
#include <assert.h>

// "Determining if an integer is a power of 2", Anderson, _Bit Twiddling Hacks_
// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool is_power_of_two(uint64_t n)
{
    return n && !(n & (n - 1));
}

void uint64_to_string(uint64_t n, char* str, size_t str_size)
{
    assert(str != NULL);
    assert(str_size >= UINT64_TO_STRING_SIZE);
    snprintf(str, str_size, "%llu", n);
    str[str_size-1] = '\0';
}

