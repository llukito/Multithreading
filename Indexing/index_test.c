#include "index.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inverted_index.h"

#define ASSERT(cmp) {					\
    if (!(cmp)) {						\
      printf("Assertion failed: %s\n", #cmp);	\
      return false;						\
    }								\
  }

#define ASSERT_WORDS(ii, words) { \
  const int num_words = sizeof(words) / sizeof(char*); \
  bool* exists = malloc(num_words * sizeof(bool)); \
  assert(exists != NULL); \
  memset(exists, 0, num_words * sizeof(bool)); \
  bool failed = false; \
  for (int i = 0; i < ii.num_buckets; ++i) { \
    Bucket* node = ii.buckets[i]; \
    while (node != NULL) { \
      bool found = false; \
      for (int i = 0; i < num_words; ++i) { \
        if (strcmp(words[i], *(char**)node->key) == 0) { \
          exists[i] = true; \
	  found = true; \
	  break; \
        } \
      } \
      if (!found) { \
        printf("Assertion failed, inverted index has unexpected word: %s\n", *(char**)node->key); \
	failed = true; \
      } \
      node = node->next; \
    } \
  } \
  for (int i = 0; i < num_words; ++i) { \
    if (!exists[i]) { \
      printf("Assertion failed, word is missing from inverted index: %s\n", words[i]); \
      failed = true; \
    } \
  } \
  free(exists); \
  if (failed) { \
    return false; \
  } \
}

#define ASSERT_POSITIONS(ii, word, positions) { \
  int num_pos = sizeof(positions) / sizeof(DocPos*); \
  bool* exists = malloc(num_pos * sizeof(bool)); \
  memset(exists, 0, num_pos * sizeof(bool)); \
  const DocPos* node = InvertedIndexGet(&ii, word); \
  bool failed = false; \
  while (node != NULL) { \
    bool found = false; \
    for (int i = 0; i < num_pos; ++i) { \
      if (node->doc_id == positions[i]->doc_id && \
  	  node->position == positions[i]->position) {	\
        exists[i] = true; \
	found = true; \
	break; \
      } \
    } \
    if (!found) { \
      printf("Assertion failed, extra position detected in inverted index: %s %d %d\n", \
             word, node->doc_id, node->position); \
      failed = true; \
    } \
    node = node->next; \
  } \
  for (int i = 0; i < num_pos; ++i) { \
    if (!exists[i]) { \
      printf("Assertion failed, expected position is missing from inverted index: %s %d %d\n", \
	     word, positions[i]->doc_id, positions[i]->position);	\
      failed = true; \
    } \
    free(positions[i]); \
  } \
  free(exists); \
  if (failed) { \
    return false; \
  } \
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

bool SingleDocumentUniqueWords() {
  char* docs[] = {"foo bar dev null"};
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  BuildInvertedIndex(/*num_docs=*/1, (char**)&docs, /*num_threads=*/1, &ii);
  char* words[] = {"foo", "bar", "dev", "null"};
  ASSERT_WORDS(ii, words);
  ASSERT_POSITIONS(ii, "foo", ((DocPos*[]){DocPosNew(0, 0)}));
  ASSERT_POSITIONS(ii, "bar", ((DocPos*[]){DocPosNew(0, 4)}));
  ASSERT_POSITIONS(ii, "dev", ((DocPos*[]){DocPosNew(0, 8)}));
  ASSERT_POSITIONS(ii, "null", ((DocPos*[]){DocPosNew(0, 12)}));
  InvertedIndexDestroy(&ii);
  return true;
}

bool SingleDocumentDuplicateWords() {
  char* docs[] = {"foo bar bar foo"};
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  BuildInvertedIndex(/*num_docs=*/1, (char**)&docs, /*num_threads=*/1, &ii);
  char* words[] = {"foo", "bar"};
  ASSERT_WORDS(ii, words);
  ASSERT_POSITIONS(ii, "foo", ((DocPos*[]){DocPosNew(0, 0), DocPosNew(0, 12)}));
  ASSERT_POSITIONS(ii, "bar", ((DocPos*[]){DocPosNew(0, 4), DocPosNew(0, 8)}));
  InvertedIndexDestroy(&ii);
  return true;
}

bool MultipleDocumentUniqueWords() {
  char* docs[] = {"foo bar", "dev", "null"};
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  BuildInvertedIndex(/*num_docs=*/3, (char**)&docs, /*num_threads=*/2, &ii);
  char* words[] = {"foo", "bar", "dev", "null"};
  ASSERT_WORDS(ii, words);
  ASSERT_POSITIONS(ii, "foo", ((DocPos*[]){DocPosNew(0, 0)}));
  ASSERT_POSITIONS(ii, "bar", ((DocPos*[]){DocPosNew(0, 4)}));
  ASSERT_POSITIONS(ii, "dev", ((DocPos*[]){DocPosNew(1, 0)}));
  ASSERT_POSITIONS(ii, "null", ((DocPos*[]){DocPosNew(2, 0)}));
  InvertedIndexDestroy(&ii);
  return true;
}

bool MultipleDocumentDuplicateWords() {
  char* docs[] = {"foo bar", "bar", "foo"};
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  BuildInvertedIndex(/*num_docs=*/3, (char**)&docs, /*num_threads=*/5, &ii);
  char* words[] = {"foo", "bar"};
  ASSERT_WORDS(ii, words);
  const DocPos* pos = InvertedIndexGet(&ii, "foo");
  ASSERT_POSITIONS(ii, "foo", ((DocPos*[]){DocPosNew(0, 0), DocPosNew(2, 0)}));
  ASSERT_POSITIONS(ii, "bar", ((DocPos*[]){DocPosNew(0, 4), DocPosNew(1, 0)}));
  InvertedIndexDestroy(&ii);
  return true;
}

bool Whitespaces() {
  char* docs[] = {"  foo  ", "", "  "};
  InvertedIndex ii;
  InvertedIndexInit(&ii);
  BuildInvertedIndex(/*num_docs=*/3, (char**)&docs, /*num_threads=*/2, &ii);
  char* words[] = {"foo"};
  ASSERT_WORDS(ii, words);
  ASSERT_POSITIONS(ii, "foo", ((DocPos*[]){DocPosNew(0, 2)}));
  InvertedIndexDestroy(&ii);
  return true;
}

int main(int argc, char** argv) {
  TEST(SingleDocumentUniqueWords);
  TEST(SingleDocumentDuplicateWords);
  TEST(MultipleDocumentUniqueWords);
  TEST(MultipleDocumentDuplicateWords);
  TEST(Whitespaces);
  return 0;
}
