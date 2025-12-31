#include "map.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void MapInit(Map* map,
	     int num_buckets,
	     size_t key_size,
	     size_t value_size,
	     CompareFn cmp_fn,
	     HashFn hash_fn,
	     FreeFn key_free_fn,
	     FreeFn value_free_fn) {
  map->num_buckets = num_buckets;
  map->buckets = malloc(num_buckets * sizeof(Bucket*));
  assert(map->buckets != NULL);
  memset(map->buckets, 0, num_buckets * sizeof(Bucket*));
  map->key_size = key_size;
  map->value_size = value_size;
  map->cmp_fn = cmp_fn;
  map->hash_fn = hash_fn;
  map->key_free_fn = key_free_fn;
  map->value_free_fn = value_free_fn;
}

void MapDestroy(Map* map) {
  for (int i = 0; i < map->num_buckets; ++i) {
    Bucket* node = map->buckets[i];
    while (node != NULL) {
      if (map->key_free_fn != NULL) {
	map->key_free_fn(node->key);
      }
      free(node->key);      
      if (map->value_free_fn != NULL) {
	map->value_free_fn(node->value);

      }
      free(node->value);      
      Bucket* next = node->next;
      free(node);
      node = next;
    }
  }
  free(map->buckets);
}

void* MapGet(Map* map, void* key) {
  const int pos = map->hash_fn(key, map->num_buckets);
  assert(0 <= pos && pos < map->num_buckets);
  Bucket* node = map->buckets[pos];
  while (node != NULL) {
    if (map->cmp_fn(node->key, key) == 0) {
      return node->value;
    }
    node = node->next;
  }
  return NULL;
}

void MapInsert(Map* map, void* key, void* value) {
  const int pos = map->hash_fn(key, map->num_buckets);
  assert(0 <= pos && pos < map->num_buckets);
  Bucket* node = map->buckets[pos];
  while (node != NULL) {
    if (map->cmp_fn(node->key, key) == 0) {
      if (map->value_free_fn != NULL) {
	map->value_free_fn(node->value);
      }
      memcpy(node->value, value, map->value_size);
      if (map->key_free_fn != NULL) {
	map->key_free_fn(key);
      }
      return;
    }
    node = node->next;
  }
  Bucket* new_node = malloc(sizeof(Bucket));
  assert(new_node != NULL);
  new_node->key = malloc(map->key_size);
  assert(new_node->key != NULL);
  memcpy(new_node->key, key, map->key_size);
  new_node->value = malloc(map->value_size);
  assert(new_node->value != NULL);
  memcpy(new_node->value, value, map->value_size);
  new_node->next = map->buckets[pos];
  map->buckets[pos] = new_node;
}
