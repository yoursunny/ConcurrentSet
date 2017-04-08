#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../cityhash-c/city.h"

/** \brief a hashtable node
 */
typedef struct HtNode
{
  struct HtNode* next; ///< the next node, in a singly linked list
  uint64_t hash; ///< the hash value
  const char value[0]; ///< the stored payload
} HtNode;

/** \brief a hashtable bucket
 */
typedef struct HtBucket
{
  HtNode* first; ///< the first node
  pthread_mutex_t lock; ///< lock to use bucket
} HtBucket;

/** \brief a hashtable
 */
typedef struct Hashtable
{
  HtBucket* buckets; ///< array of buckets
  size_t nBuckets; ///< number of buckets
  atomic_size_t nNodes; ///< number of nodes
  pthread_rwlock_t resizeLock; ///< lock to prevent resize
  size_t nExpandThres; ///< expand nBuckets if nNodes is over this threshold
  size_t nShrinkThres; ///< shrink nBuckets if nNodes is under this threshold
} Hashtable;

/** \brief constructor
 *  \note This function is not thread safe.
 */
Hashtable*
Hashtable_new(void);

/** \brief constructor
 *  \note This function is not thread safe.
 */
void
Hashtable_dtor(Hashtable* self);

/** \brief insert a value
 *  \return true if the value is inserted
 *  \note This function is thread safe.
 */
bool
Hashtable_insert(Hashtable* self, const char* value);

/** \brief delete a value
 *  \return true if the value previously exists
 *  \note This function is thread safe.
 */
bool
Hashtable_erase(Hashtable* self, const char* value);

/** \brief whether a value exists
 *  \note This function is thread safe.
 */
bool
Hashtable_has(Hashtable* self, const char* value);

#endif // HASHTABLE_H
