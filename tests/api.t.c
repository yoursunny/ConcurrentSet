#include "concurrent-set.h"
#include <CUnit/CUnit.h>

void
test_new_set(void)
{
  config c = {};
  set* s = new_set(&c);
  CU_ASSERT_PTR_NULL(s);
}

void
suite_api(void)
{
  CU_pSuite suite = CU_add_suite("api", NULL, NULL);
  CU_add_test(suite, "new_set", test_new_set);
}
