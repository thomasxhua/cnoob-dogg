#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>

#define UINT64_TO_STRING_SIZE 22

bool is_power_of_two(uint64_t n);
void uint64_to_string(uint64_t n, char* str, size_t str_size);
uint64_t abs_diff(uint64_t a, uint64_t b);
size_t string_tokenize_alloc(char* str, char** tokens, size_t tokens_size);

#endif // UTILS_H

