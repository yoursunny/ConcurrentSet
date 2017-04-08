#include "concurrent-set.h"
#include "hashtable.h"

set*
new_set(config* c)
{
  set* s = (set*)malloc(sizeof(set));
  s->ht = Hashtable_new();
  return s;
}

bool
set_add(set* s, string o)
{
  return !Hashtable_insert(s->ht, o);
}

bool
set_del(set* s, string o)
{
  return !Hashtable_erase(s->ht, o);
}

bool
set_contains(set* s, string o)
{
  return Hashtable_has(s->ht, o);
}
