#include "hashtable.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

const size_t Hashtable_nMinBuckets = 16;
const float Hashtable_expandLoadFactor = 0.5;
const float Hashtable_expandFactor = 2.0;
const float Hashtable_shrinkLoadFactor = 0.1;
const float Hashtable_shrinkFactor = 0.5;

/** \brief find a node
 *  \return pointer to HtBucket.first or HtNode.next field on the previous node, if found;
 *          otherwise, pointer to HtBucket.first or HtNode.next field on the last node in bucket
 */
HtNode**
Hashtable_find(Hashtable* self, uint64_t h, const char* value);

/** \brief resize the hashtable
 */
void
Hashtable_resize(Hashtable* self, size_t newNBuckets);

// ----------------------------------------------------------------

Hashtable*
Hashtable_new()
{
  Hashtable* self = (Hashtable*)malloc(sizeof(Hashtable));
  self->buckets = NULL;
  self->nBuckets = 0;
  self->nNodes = 0;
  Hashtable_resize(self, Hashtable_nMinBuckets);
  return self;
}

void
Hashtable_dtor(Hashtable* self)
{
  for (size_t i = 0; i < self->nBuckets; ++i) {
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
  HtNode** it = Hashtable_find(self, h, value);
  assert(it != NULL);

  if (*it != NULL) { // node exists
    return false;
  }

  HtNode* node = (HtNode*)malloc(sizeof(HtNode) + valueLen + 1);
  node->next = NULL;
  node->hash = h;
  memcpy((char*)node->value, value, valueLen + 1);
  *it = node;

  ++self->nNodes;
  if (self->nNodes > self->nExpandThres) {
    Hashtable_resize(self, self->nBuckets * Hashtable_expandFactor);
  }

  return true;
}

bool
Hashtable_erase(Hashtable* self, const char* value)
{
  size_t valueLen = strlen(value);
  uint64_t h = CityHash64(value, valueLen);
  HtNode** it = Hashtable_find(self, h, value);
  assert(it != NULL);

  if (*it == NULL) { // node does not exist
    return false;
  }

  HtNode* node = *it;
  *it = node->next;
  free(node);

  --self->nNodes;
  if (self->nNodes < self->nShrinkThres) {
    size_t newNBuckets = self->nBuckets * Hashtable_shrinkFactor;
    if (newNBuckets < Hashtable_nMinBuckets) {
      newNBuckets = Hashtable_nMinBuckets;
    }
    if (newNBuckets != self->nBuckets) {
      Hashtable_resize(self, newNBuckets);
    }
  }

  return true;
}

bool
Hashtable_has(Hashtable* self, const char* value)
{
  size_t valueLen = strlen(value);
  uint64_t h = CityHash64(value, valueLen);
  HtNode** it = Hashtable_find(self, h, value);
  assert(it != NULL);

  return *it != NULL;
}

HtNode**
Hashtable_find(Hashtable* self, uint64_t h, const char* value)
{
  HtBucket* bucket = &self->buckets[h % self->nBuckets];
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
  HtBucket* newBuckets = (HtBucket*)calloc(newNBuckets, sizeof(HtBucket));
  for (size_t i = 0; i < self->nBuckets; ++i) {
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
