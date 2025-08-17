#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DELOG(msg)       printf("[DELOG] " msg "\n")
#define DEVAR(fmt, name) printf("[DELOG] %s: " fmt "\n", #name, name)
#define ERROR(msg)       printf("[ERROR] " msg "\n")

#endif // DEBUG_H

