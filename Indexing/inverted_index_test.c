#include "inverted_index.h"

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

bool SingleOccurrences() {
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  InvertedIndexInsert(&ii, strdup("foo"), 0, 1);
  InvertedIndexInsert(&ii, strdup("bar"), 2, 3);
  const DocPos* p = InvertedIndexGet(&ii, "foo");
  ASSERT(p->doc_id == 0 && p->position == 1);
  ASSERT(p->next == NULL);
  p = InvertedIndexGet(&ii, "bar");
  ASSERT(p->doc_id == 2 && p->position == 3);
  ASSERT(p->next == NULL);  
  InvertedIndexDestroy(&ii);
  return true;
}

bool Duplicates() {
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  InvertedIndexInsert(&ii, strdup("foo"), 0, 1);
  InvertedIndexInsert(&ii, strdup("foo"), 2, 3);
  const DocPos* p = InvertedIndexGet(&ii, "foo");
  ASSERT(p->doc_id == 0 && p->position == 1);
  ASSERT(p->next != NULL);
  p = p->next;
  ASSERT(p->doc_id == 2 && p->position == 3);
  ASSERT(p->next == NULL);  
  InvertedIndexDestroy(&ii);
  return true;
}

int main(int argc, char** argv) {
  TEST(SingleOccurrences);
  TEST(Duplicates);
  return 0;
}

