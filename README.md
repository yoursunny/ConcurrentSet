# ConcurrentSet

The **ConcurrentSet** C library provides a thread-safe *set* data structure.
The underlying container is a hashtable, similar to C++ `std::unordered_set`.
API supports addition, removal, and membership query.

## Usage

This library requires Ubuntu 16.04 or above.

Dependencies can be installed with:

    sudo apt-get install build-essential libcunit1-dev

To compile and install the library:

    ./waf configure
    ./waf
    sudo ./waf install

To run unit tests:

    ./waf configure --with-tests --debug
    ./waf
    build/unit-tests

## API

Construct a new set:

    set*
    new_set(config* c);

Add a member:

    bool
    set_add(set* s, string o);

Remove a member:

    bool
    set_del(set* s, string o);

Membership query:

    bool
    set_contains(set* s, string o);

## Complexity

The underlying data structure is a hashtable.
Hash conflicts are resolved by organizing members in the same bucket as a singly linked list.
Number of buckets is dynamically adjusted according to occupancy rate.
Memory usage is linear in the size of set, plus strings themselves.

Time complexity of each operation, in terms of number of comparisons, is constant on average, worst case linear in the size of set.
The worst case occurs if all members are hashed to the same bucket.
Each string comparison has a time complexity linear in the length of string.

## Thread Safety

Public APIs of this library, except the constructor, are thread-safe.

A global reader-writer lock protects/prevents resize operation.
A thread that wants to resize the hashtable (change number of buckets) must hold a writer lock.
Any other access to buckets requires a reader lock, which prevents resizing.

Per-bucket mutex protects a bucket.
A thread that wants to query or modify a bucket must hold the mutex, which prevents any other thread from modifying the bucket.

The above implementation assumes the access pattern to have equal amount of query and modification operations, and the size of set stays stable so that resize does not happen very often.
