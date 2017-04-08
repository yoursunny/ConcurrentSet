#include "hashtable.h"
#include <CUnit/CUnit.h>

static void
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

static void
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

const int threadTest_nThreads = 8;
const int threadTest_maxValue = 1048576;

typedef struct ThreadArg
{
  Hashtable* ht;
  int k; ///< thread id
  atomic_int* counters; ///< number of operations returning true
  int* winners; ///< thread id when the operation returning true for the first time
} ThreadArg;

static void
threadTest_incrementCounter(ThreadArg* arg, int i)
{
  if (atomic_fetch_add(&arg->counters[i], 1) == 0) {
    arg->winners[i] = arg->k;
  }
}

static void
threadTest_checkWinners(int* winners)
{
  // While it's possible for a single thread to 'win' every time,
  // we expect that every thread has a chance invoking an operation returning true.

  int nWins[threadTest_nThreads];
  memset(nWins, 0, sizeof(nWins[0]) * threadTest_nThreads);

  for (int i = 0; i < threadTest_maxValue; ++i) {
    if (winners[i] < threadTest_nThreads) {
      ++nWins[winners[i]];
    }
  }

  for (int k = 0; k < threadTest_nThreads; ++k) {
    CU_ASSERT(nWins[k] > 0);
  }
}

#define threadTest_run(func) \
  pthread_t threads[threadTest_nThreads]; \
  ThreadArg args[threadTest_nThreads]; \
  atomic_int* counters = malloc(sizeof(atomic_int) * threadTest_maxValue); \
  for (int i = 0; i < threadTest_maxValue; ++i) { \
    atomic_init(&counters[i], 0); \
  } \
  int* winners = malloc(sizeof(int) * threadTest_maxValue); \
  memset(winners, 0xFF, sizeof(winners[0]) * threadTest_maxValue); \
  for (int k = 0; k < threadTest_nThreads; ++k) { \
    args[k].ht = ht; \
    args[k].k = k; \
    args[k].counters = counters; \
    args[k].winners = winners; \
    pthread_create(&threads[k], NULL, func, &args[k]); \
  } \
  for (int k = 0; k < threadTest_nThreads; ++k) { \
    pthread_join(threads[k], NULL); \
  } \
  do {} while (false)

#define threadTest_end() \
  free(counters); \
  free(winners); \
  Hashtable_dtor(ht)

static void*
insertThread(void* arg1)
{
  ThreadArg* arg = (ThreadArg*)arg1;

  char s[9];
  for (int i = 0; i < threadTest_maxValue; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    if (Hashtable_insert(arg->ht, s)) {
      threadTest_incrementCounter(arg, i);
    }
  }

  return NULL;
}

static void
test_thread_insert(void)
{
  Hashtable* ht = Hashtable_new();

  threadTest_run(insertThread);
  threadTest_checkWinners(winners);

  char s[9];
  for (int i = 0; i < threadTest_maxValue; ++i) {
    CU_ASSERT_EQUAL(counters[i], 1);
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_has(ht, s));
  }

  threadTest_end();
}

static void*
queryThread(void* arg1)
{
  ThreadArg* arg = (ThreadArg*)arg1;

  char s[9];
  for (int i = 0; i < threadTest_maxValue; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    if (Hashtable_has(arg->ht, s)) {
      threadTest_incrementCounter(arg, i);
    }
  }

  return NULL;
}

static void
test_thread_query(void)
{
  Hashtable* ht = Hashtable_new();

  char s[9];
  for (int i = 0; i < threadTest_maxValue; i += 2) {
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_insert(ht, s));
  }

  threadTest_run(queryThread);

  for (int i = 0; i < threadTest_maxValue; ++i) {
    CU_ASSERT_EQUAL(counters[i], i % 2 == 0 ? threadTest_nThreads : 0);
  }

  threadTest_end();
}

static void*
eraseThread(void* arg1)
{
  ThreadArg* arg = (ThreadArg*)arg1;

  char s[9];
  for (int i = 0; i < threadTest_maxValue; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    if (Hashtable_erase(arg->ht, s)) {
      threadTest_incrementCounter(arg, i);
    }
  }

  return NULL;
}

static void
test_thread_erase(void)
{
  Hashtable* ht = Hashtable_new();

  char s[9];
  for (int i = 0; i < threadTest_maxValue; ++i) {
    snprintf(s, sizeof(s), "%x", i);
    CU_ASSERT_TRUE(Hashtable_insert(ht, s));
  }

  threadTest_run(eraseThread);
  threadTest_checkWinners(winners);

  for (int i = 0; i < threadTest_maxValue; ++i) {
    CU_ASSERT_EQUAL(counters[i], 1);
  }

  threadTest_end();
}

void
suite_hashtable(void)
{
  CU_pSuite suite = CU_add_suite("hashtable", NULL, NULL);
  CU_add_test(suite, "basic", test_basic);
  CU_add_test(suite, "resize", test_resize);
  CU_add_test(suite, "thread_insert", test_thread_insert);
  CU_add_test(suite, "thread_query", test_thread_query);
  CU_add_test(suite, "thread_erase", test_thread_erase);
}
