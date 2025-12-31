#ifndef PARADIGMS_PROBLEMS_INVERTED_INDEX_INVERTED_INDEX_H_
#define PARADIGMS_PROBLEMS_INVERTED_INDEX_INVERTED_INDEX_H_

#include "map.h"

#define INVERTED_INDEX_NUM_BUCKETS 10

typedef struct DocPos {
  int doc_id;
  int position;
  struct DocPos* next;
} DocPos;

// Creates and returns new DocPos. Caller takes ownership.
DocPos* DocPosNew(int doc_id, int position);
// Clones and returns give position. Caller takes ownership.
DocPos* DocPosClone(DocPos* pos);
// Cleans up memory of give document position chain objects.
void DocPosDestroy(DocPos* pos);

typedef Map InvertedIndex;

// Initializes InvertedIndex.
void InvertedIndexInit(InvertedIndex* ii);
// Cleans up memory occupied by given inverted index.
void InvertedIndexDestroy(InvertedIndex* ii);
// Returns all occurrences of given word.
const DocPos* InvertedIndexGet(InvertedIndex* ii, char* word);
// Takes ownership of passed word.
void InvertedIndexInsert(InvertedIndex* ii,
			 char* word,
			 int doc_id,
			 int position);
// Takes ownership of passed word.
void InvertedIndexInsertDocPos(InvertedIndex* ii,
			       char* word,
			       DocPos* pos);
// Merges current inverted index with the other one.
void InvertedIndexMergeWith(InvertedIndex* ii, InvertedIndex* other);

#endif // PARADIGMS_PROBLEMS_INVERTED_INDEX_INVERTED_INDEX_H_
