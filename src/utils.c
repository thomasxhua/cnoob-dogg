#include "utils.h"

// "Determining if an integer is a power of 2", in: Sean Eron Anderson, _Bit Twiddling Hacks_, 1997-2005
// http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool is_power_of_two(uint64_t n)
{
    return n && !(n & (n - 1));
}

