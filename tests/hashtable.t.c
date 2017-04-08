#include "hashtable.h"
#include <CUnit/CUnit.h>

void
test_basic(void)
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

void
test_resize(void)
{
  char s[5];
  Hashtable* ht = Hashtable_new();
  size_t nBuckets1 = ht->nBuckets;

  for (int i = 0; i < 4096; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_insert(ht, s));
  }

  size_t nBuckets2 = ht->nBuckets;
  CU_ASSERT(nBuckets2 > nBuckets1);

  for (int i = 0; i < 4096; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_has(ht, s));
  }

  size_t nBuckets3 = ht->nBuckets;
  CU_ASSERT(nBuckets3 == nBuckets2);

  for (int i = 0; i < 4096; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_erase(ht, s));
  }

  size_t nBuckets4 = ht->nBuckets;
  CU_ASSERT(nBuckets4 < nBuckets3);

  Hashtable_dtor(ht);
}

void
suite_hashtable(void)
{
  CU_pSuite suite = CU_add_suite("hashtable", NULL, NULL);
  CU_add_test(suite, "basic", test_basic);
  CU_add_test(suite, "resize", test_resize);
}
