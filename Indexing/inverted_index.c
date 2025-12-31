#include "inverted_index.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

DocPos* DocPosNew(int doc_id, int position) {
  DocPos* pos = malloc(sizeof(DocPos));
  assert(pos != NULL);
  pos->doc_id = doc_id;
  pos->position = position;
  pos->next = NULL;
  return pos;
}

DocPos* DocPosClone(DocPos* pos) {
  if (pos == NULL) {
    return pos;
  }
  DocPos* ret = malloc(sizeof(DocPos));
  ret->doc_id = pos->doc_id;
  ret->position = pos->position;
  ret->next = DocPosClone(pos->next);
  return ret;
}

void DocPosDestroy(DocPos* pos) {
  if (pos == NULL) {
    return;
  }
  DocPosDestroy(pos->next);
  free(pos);
}

int StrCmp(const void* key_1, const void* key_2) {
  return strcmp(*(const char**)key_1, *(const char**)key_2);
}

int StrHash(const void* key, int num_buckets) {
  const char* a = *(const char**)key;
  int ret = 0;
  for (int i = 0; i < strlen(a); ++i) {
    ret = (255 * ret + a[i]) % num_buckets;
  }
  return ret;
}

void StrFree(void* ptr) {
  free(*(char**)ptr);
}

void DocPosFree(void* ptr) {
  DocPosDestroy(*(DocPos**)ptr);
}

void InvertedIndexInit(InvertedIndex* ii) {
  MapInit(ii,
	  INVERTED_INDEX_NUM_BUCKETS,
	  sizeof(char*),
	  sizeof(DocPos*),
	  StrCmp,
	  StrHash,
	  StrFree,
	  DocPosFree);
}

void InvertedIndexDestroy(InvertedIndex* ii) {
  MapDestroy(ii);
}

const DocPos* InvertedIndexGet(InvertedIndex* ii, char* word) {
  return *(DocPos**)MapGet(ii, &word);
}

void InvertedIndexInsert(InvertedIndex* ii,
			 char* word,
			 int doc_id,
			 int position) {
  DocPos* pos = malloc(sizeof(DocPos));
  pos->doc_id = doc_id;
  pos->position = position;
  pos->next = NULL;
  InvertedIndexInsertDocPos(ii, word, pos);
}

void InvertedIndexInsertDocPos(InvertedIndex* ii,
			       char* word,
			       DocPos* pos) {
  DocPos** cur = MapGet(ii, &word);
  if (cur == NULL) {
    MapInsert(ii, &word, &pos);
  } else {
    DocPos* node = *cur;
    while (node->next != NULL) {
      node = node->next;
    }
    node->next = pos;
  }
}

void InvertedIndexMergeWith(InvertedIndex* ii, InvertedIndex* other) {
  for (int i = 0; i < other->num_buckets; ++i) {
    Bucket* node = other->buckets[i];
    while (node != NULL) {
      InvertedIndexInsertDocPos(ii, strdup(*(char**)node->key), DocPosClone(*(DocPos**)node->value));
      node = node->next;
    }
  }
}
