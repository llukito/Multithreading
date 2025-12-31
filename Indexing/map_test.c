#include "map.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(cmp) {					\
    if (!(cmp)) {						\
      printf("Assertion failed: %s\n", #cmp);	\
      return false;						\
    }								\
  }

#define TEST(t) { \
    printf("-- TESTING: %s\n", #t); \
    if (t()) {	  \
      printf("TEST %s: SUCCEEDED\n", #t);	\
    } \
    else { \
      printf("TEST %s: FAILED\n", #t);		\
    } \
    printf("####\n\n"); \
  }

int CmpInt(const void* a, const void* b) {
  return (*(const int*)a) - (*(const int*)b);
}

int HashInt(const void* a, int num_buckets) {
  return (*(const int*)a) % num_buckets;
}

bool IntegerMap() {
  Map m;
  MapInit(&m, 10, sizeof(int), sizeof(int), CmpInt, HashInt, NULL, NULL);
  int zero = 0;
  int one = 1;
  ASSERT(MapGet(&m, &zero) == NULL);
  ASSERT(MapGet(&m, &one) == NULL);
  MapInsert(&m, &zero, &one);
  ASSERT(*(int*)MapGet(&m, &zero) == 1);
  MapInsert(&m, &one, &zero);
  ASSERT(*(int*)MapGet(&m, &one) == 0);
  MapDestroy(&m);
  return true;
}

bool IntegerMapSingleBucket() {
  Map m;
  MapInit(&m, 1, sizeof(int), sizeof(int), CmpInt, HashInt, NULL, NULL);
  for (int i = 0; i < 100; ++i) {
    ASSERT(MapGet(&m, &i) == NULL);
  }
  for (int i = 0; i < 100; ++i) {
    int v = 100 - i;
    MapInsert(&m, &i, &v);
  }
  for (int i = 99; i >= 0; --i) {
    ASSERT(*(int*)MapGet(&m, &i) == 100 - i);
  }  
  MapDestroy(&m);
  return true;
}

int CmpStr(const void* a, const void* b) {
  return strcmp(*(const char**)a, *(const char**)b);
}

int HashStr(const void* a, int num_buckets) {
  const char* key = *(const char**)a;
  int ret = 0;
  for (int i = 0; i < strlen(key); ++i) {
    ret = (1000 * ret + key[i]) % num_buckets;
  }
  return ret;
}

void FreeStr(void* ptr) {
  free(*(void**)ptr);
}

bool StringIntegerMap() {
  Map m;
  MapInit(&m, 3, sizeof(char*), sizeof(int), CmpStr, HashStr, FreeStr, NULL);
  char* digits[] = {strdup("zero"),
		    strdup("one"),
		    strdup("two"),
		    strdup("three"),
		    strdup("four"),
		    strdup("five"),
		    strdup("six"),
		    strdup("seven"),
		    strdup("eight"),
		    strdup("nine")};
  for (int i = 0; i < 10; ++i) {
    MapInsert(&m, &digits[i], &i);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT(*(int*)MapGet(&m, &digits[i]) == i);
  }  
  MapDestroy(&m);
  return true;
}

void FreeInt(void* ptr) {
  free(*(int**)ptr);
}

bool StringAllocatedIntegerMap() {
  Map m;
  MapInit(&m, 5, sizeof(char*), sizeof(int*), CmpStr, HashStr, FreeStr, FreeInt);
  char* digits[] = {strdup("zero"),
		    strdup("one"),
		    strdup("two"),
		    strdup("three"),
		    strdup("four"),
		    strdup("five"),
		    strdup("six"),
		    strdup("seven"),
		    strdup("eight"),
		    strdup("nine")};
  for (int i = 0; i < 10; ++i) {
    int* value = malloc(sizeof(int*));
    assert(value != NULL);
    *value = i;
    MapInsert(&m, &digits[i], &value);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT(**(int**)MapGet(&m, &digits[i]) == i);
  }  
  MapDestroy(&m);
  return true;
}

bool IntegerMapWithDuplicates() {
  Map m;
  MapInit(&m, 1, sizeof(int), sizeof(int), CmpInt, HashInt, NULL, NULL);
  int ids[2] = {0, 1};
  for (int i = 0; i < 100; ++i) {
    MapInsert(&m, &ids[i % 2], &i);
  }
  ASSERT(*(int*)MapGet(&m, ids) == 98);
  ASSERT(*(int*)MapGet(&m, ids + 1) == 99);
  MapDestroy(&m);
  return true;
}

bool StringAllocatedIntegerMapWithDuplicates() {
  Map m;
  MapInit(&m, 5, sizeof(char*), sizeof(int*), CmpStr, HashStr, FreeStr, FreeInt);
  char* digits[] = {strdup("zero"),
		    strdup("one"),
		    strdup("two"),
		    strdup("three"),
		    strdup("four"),
		    strdup("five"),
		    strdup("six"),
		    strdup("seven"),
		    strdup("eight"),
		    strdup("nine")};
  for (int i = 0; i < 100; ++i) {
    char* key = strdup(digits[i % 10]);
    int* value = malloc(sizeof(int*));
    assert(value != NULL);
    *value = i;
    MapInsert(&m, &key, &value);
  }
  for (int i = 0; i < 10; ++i) {
    ASSERT(**(int**)MapGet(&m, &digits[i]) == 90 + i);
  }  
  MapDestroy(&m);
  return true;
}

int main(int argc, char** argv) {
  TEST(IntegerMap);
  TEST(IntegerMapSingleBucket);
  TEST(StringIntegerMap);
  TEST(StringAllocatedIntegerMap);
  TEST(IntegerMapWithDuplicates);
  TEST(StringAllocatedIntegerMapWithDuplicates);  
  return 0;
}
