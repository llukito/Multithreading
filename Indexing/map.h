#ifndef PARADIGMS_PROBLEMS_INVERTED_INDEX_MAP_H_
#define PARADIGMS_PROBLEMS_INVERTED_INDEX_MAP_H_

#include <stddef.h>

// CompareFn typed function pointers take two keys and must return:
// * Zero if keys are equal
// * Negative integer if first key is less than second
// * Positive integer if first key is greater than second
typedef int(*CompareFn)(const void* key_1, const void* key_2);

// HashFn typed function pointers take pointer to the key and a number of
// buckets and 0 based index of the bucket where given key belongs to.
typedef int(*HashFn)(const void* key, int num_buckets);

// FreeFn typed function pointers take pointers to the key, value pair and
// is responsible of freeing up all necessary memory.
typedef void(*FreeFn)(void* ptr);


typedef struct Bucket {
  void* key;
  void* value;
  struct Bucket* next;
} Bucket;

typedef struct {
  size_t key_size;
  size_t value_size;
  int num_buckets;
  Bucket** buckets;
  CompareFn cmp_fn;
  HashFn hash_fn;
  FreeFn key_free_fn;
  FreeFn value_free_fn;
} Map;

void MapInit(Map* map,
	     int num_buckets,
	     size_t key_size,
	     size_t value_size,
	     CompareFn cmp_fn,
	     HashFn hash_fn,
	     FreeFn key_free_fn,
	     FreeFn value_free_fn);

void MapDestroy(Map* map);

void* MapGet(Map* map, void* key);
void MapInsert(Map* map, void* key, void* value);


#endif // PARADIGMS_PROBLEMS_INVERTED_INDEX_MAP_H_
