#include "utils.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

uint64_t abs_diff(uint64_t a, uint64_t b)
{
    return (a < b) ? b-a : a-b;
}

#include "debug.h"
void string_tokenize_alloc(char* str, dyn_array_char_ptr* buffer)
{
    assert(str != NULL);
    char* p = str;
    while (*p)
    {
        for (; isspace(*p); ++p);
        if (*p == '\0') break;
        const char* token_start = p;
        char* token;
        if (*p++ == '"')
        {
            for (; *p && *p != '"'; ++p);
            const size_t token_size = p - token_start;
            token = malloc(token_size+1);
            memcpy(token, token_start+1, token_size-1);
            token[token_size] = '\0';
            if (*p == '"')
                ++p;
        }
        else
        {
            for (; *p && !isspace(*p); ++p);
            const size_t token_size = p - token_start;
            token = malloc(token_size+1);
            memcpy(token, token_start, token_size);
            token[token_size] = '\0';
        }
        dyn_array_char_ptr_append(buffer, token);
    }
}

