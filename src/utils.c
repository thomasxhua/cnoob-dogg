#include "utils.h"

bool is_power_of_two(uint64_t n)
{
    // Anderson, http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    return n && !(n & (n - 1));
}

