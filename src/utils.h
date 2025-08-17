#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>

#include "dyn_array.h"

#define UINT64_TO_STRING_SIZE 22

bool is_power_of_two(uint64_t n);
void uint64_to_string(uint64_t n, char* str, size_t str_size);
uint64_t abs_diff(uint64_t a, uint64_t b);
void string_tokenize_alloc(char* str, dyn_array_char_ptr* buffer);

#endif // UTILS_H

