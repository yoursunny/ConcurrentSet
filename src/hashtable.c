#include "hashtable.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PTHREAD_MUST(func, ...) \
  do { \
    int res = func(__VA_ARGS__); \
    if (res != 0) { \
      perror(#func); \
      abort(); \
    } \
  } \
  while (false)

const size_t Hashtable_nMinBuckets = 16;
const float Hashtable_expandLoadFactor = 0.5;
const float Hashtable_expandFactor = 2.0;
const float Hashtable_shrinkLoadFactor = 0.1;
const float Hashtable_shrinkFactor = 0.5;

/** \brief find a node
 *  \param[out] bucketLock pointer to HtBucket.lock
 *  \return pointer to HtBucket.first or HtNode.next field on the previous node, if found;
 *          otherwise, pointer to HtBucket.first or HtNode.next field on the last node in bucket
 *  \note This function acquires Hashtable.resizeLock-read and HtBucket.lock
 */
HtNode**
Hashtable_find(Hashtable* self, uint64_t h, const char* value, pthread_mutex_t** bucketLock);

/** \brief resize the hashtable
 *  \note This function requires Hashtable.resizeLock-read and upgrades it to write lock
 */
void
Hashtable_resize(Hashtable* self, size_t newNBuckets);

// ----------------------------------------------------------------

Hashtable*
Hashtable_new(void)
{
  Hashtable* self = (Hashtable*)malloc(sizeof(Hashtable));
  self->buckets = NULL;
  self->nBuckets = 0;
  self->nNodes = 0;
  PTHREAD_MUST(pthread_rwlock_init, &self->resizeLock, NULL);

  PTHREAD_MUST(pthread_rwlock_rdlock, &self->resizeLock);
  Hashtable_resize(self, Hashtable_nMinBuckets);
  PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);

  return self;
}

void
Hashtable_dtor(Hashtable* self)
{
  PTHREAD_MUST(pthread_rwlock_destroy, &self->resizeLock);

  for (size_t i = 0; i < self->nBuckets; ++i) {
    PTHREAD_MUST(pthread_mutex_destroy, &self->buckets[i].lock);
    HtNode* node = self->buckets[i].first;
    while (node != NULL) {
      HtNode* next = node->next;
      free(node);
      node = next;
    }
  }
  free(self->buckets);
  free(self);
}

bool
Hashtable_insert(Hashtable* self, const char* value)
{
  size_t valueLen = strlen(value);
  uint64_t h = CityHash64(value, valueLen);

  pthread_mutex_t* bucketLock = NULL;
  HtNode** it = Hashtable_find(self, h, value, &bucketLock);
  assert(it != NULL);
  assert(bucketLock != NULL);

  if (*it != NULL) { // node exists
    PTHREAD_MUST(pthread_mutex_unlock, bucketLock);
    PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
    return false;
  }

  HtNode* node = (HtNode*)malloc(sizeof(HtNode) + valueLen + 1);
  node->next = NULL;
  node->hash = h;
  memcpy((char*)node->value, value, valueLen + 1);
  *it = node;

  PTHREAD_MUST(pthread_mutex_unlock, bucketLock);

  size_t nNodes = atomic_fetch_add(&self->nNodes, 1) + 1;
  if (nNodes > self->nExpandThres) {
    Hashtable_resize(self, self->nBuckets * Hashtable_expandFactor);
  }

  PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
  return true;
}

bool
Hashtable_erase(Hashtable* self, const char* value)
{
  size_t valueLen = strlen(value);
  uint64_t h = CityHash64(value, valueLen);

  pthread_mutex_t* bucketLock = NULL;
  HtNode** it = Hashtable_find(self, h, value, &bucketLock);
  assert(it != NULL);
  assert(bucketLock != NULL);

  if (*it == NULL) { // node does not exist
    PTHREAD_MUST(pthread_mutex_unlock, bucketLock);
    PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
    return false;
  }

  HtNode* node = *it;
  *it = node->next;
  free(node);

  PTHREAD_MUST(pthread_mutex_unlock, bucketLock);

  size_t nNodes = atomic_fetch_sub(&self->nNodes, 1) - 1;
  if (nNodes < self->nShrinkThres) {
    size_t newNBuckets = self->nBuckets * Hashtable_shrinkFactor;
    if (newNBuckets < Hashtable_nMinBuckets) {
      newNBuckets = Hashtable_nMinBuckets;
    }
    Hashtable_resize(self, newNBuckets);
  }

  PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
  return true;
}

bool
Hashtable_has(Hashtable* self, const char* value)
{
  size_t valueLen = strlen(value);
  uint64_t h = CityHash64(value, valueLen);

  pthread_mutex_t* bucketLock = NULL;
  HtNode** it = Hashtable_find(self, h, value, &bucketLock);
  assert(it != NULL);
  assert(bucketLock != NULL);

  bool res = *it != NULL;

  PTHREAD_MUST(pthread_mutex_unlock, bucketLock);
  PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
  return res;
}

HtNode**
Hashtable_find(Hashtable* self, uint64_t h, const char* value, pthread_mutex_t** bucketLock)
{
  PTHREAD_MUST(pthread_rwlock_rdlock, &self->resizeLock);

  HtBucket* bucket = &self->buckets[h % self->nBuckets];
  *bucketLock = &bucket->lock;
  PTHREAD_MUST(pthread_mutex_lock, &bucket->lock);

  HtNode** it = &bucket->first;
  while (*it != NULL) {
    if (strcmp((*it)->value, value) == 0) {
      return it;
    }
    it = &(*it)->next;
  }
  return it;
}

void
Hashtable_resize(Hashtable* self, size_t newNBuckets)
{
  PTHREAD_MUST(pthread_rwlock_unlock, &self->resizeLock);
  PTHREAD_MUST(pthread_rwlock_wrlock, &self->resizeLock);

  if (newNBuckets == self->nBuckets) {
    return;
  }

  HtBucket* newBuckets = (HtBucket*)calloc(newNBuckets, sizeof(HtBucket));
  for (size_t j = 0; j < newNBuckets; ++j) {
    PTHREAD_MUST(pthread_mutex_init, &newBuckets[j].lock, NULL);
  }

  for (size_t i = 0; i < self->nBuckets; ++i) {
    PTHREAD_MUST(pthread_mutex_destroy, &self->buckets[i].lock);
    HtNode* node = self->buckets[i].first;
    while (node != NULL) {
      HtNode* next = node->next;
      size_t j = node->hash % newNBuckets;
      node->next = newBuckets[j].first;
      newBuckets[j].first = node;
      node = next;
    }
  }

  if (self->buckets != NULL) {
    free(self->buckets);
  }
  self->buckets = newBuckets;
  self->nBuckets = newNBuckets;
  self->nExpandThres = Hashtable_expandLoadFactor * newNBuckets;
  self->nShrinkThres = Hashtable_shrinkLoadFactor * newNBuckets;
}
