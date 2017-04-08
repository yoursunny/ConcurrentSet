#include "concurrent-set.h"
#include <CUnit/CUnit.h>

static void
test_basic(void)
{
  config c = {};
  set* s = new_set(&c);
  CU_ASSERT_PTR_NOT_NULL(s);

  CU_ASSERT_FALSE(set_contains(s, "A"));
  CU_ASSERT_FALSE(set_add(s, "A"));
  CU_ASSERT_TRUE(set_add(s, "A"));
  CU_ASSERT_TRUE(set_contains(s, "A"));
  CU_ASSERT_FALSE(set_del(s, "A"));
  CU_ASSERT_TRUE(set_del(s, "A"));
  CU_ASSERT_FALSE(set_contains(s, "A"));
}

void
suite_api(void)
{
  CU_pSuite suite = CU_add_suite("api", NULL, NULL);
  CU_add_test(suite, "basic", test_basic);
}
