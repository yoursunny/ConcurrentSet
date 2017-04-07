#include "hashtable.h"
#include <CUnit/CUnit.h>

void test_basic(void)
{
  Hashtable* ht = Hashtable_new();
  CU_ASSERT_PTR_NOT_NULL_FATAL(ht);

  CU_ASSERT_TRUE(Hashtable_insert(ht, "s1"));
  CU_ASSERT_FALSE(Hashtable_insert(ht, "s1"));
  CU_ASSERT_TRUE(Hashtable_has(ht, "s1"));
  CU_ASSERT_FALSE(Hashtable_has(ht, "s2"));

  CU_ASSERT_TRUE(Hashtable_insert(ht, "s2"));
  CU_ASSERT_FALSE(Hashtable_insert(ht, "s2"));
  CU_ASSERT_TRUE(Hashtable_has(ht, "s1"));
  CU_ASSERT_TRUE(Hashtable_has(ht, "s2"));

  CU_ASSERT_TRUE(Hashtable_erase(ht, "s1"));
  CU_ASSERT_FALSE(Hashtable_erase(ht, "s1"));
  CU_ASSERT_FALSE(Hashtable_has(ht, "s1"));
  CU_ASSERT_TRUE(Hashtable_has(ht, "s2"));

  Hashtable_dtor(ht);
}

void suite_hashtable(void)
{
  CU_pSuite suite = CU_add_suite("hashtable", NULL, NULL);
  CU_add_test(suite, "basic", test_basic);
}
