#ifndef CONCURRENT_SET_H
#define CONCURRENT_SET_H

#include <stdbool.h>

typedef const char* string;

typedef struct config
{
} config;

typedef struct set
{
} set;

set*
new_set(config* c);

bool
set_add(set* s, string o);

bool
set_del(set* s, string o);

bool
set_contains(set* s, string o);

#endif // CONCURRENT_SET_H
