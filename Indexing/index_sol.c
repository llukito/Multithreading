#include "index.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void IndexDoc(char* doc, int doc_id, InvertedIndex* ii) {
  for (int i = 0; i < strlen(doc); ++i) {
    if (doc[i] == ' ') {
      continue;
    }
    int j = i;
    while (doc[j] != ' ' && doc[j] != '\0') {
      ++j;
    }
    InvertedIndexInsert(ii, strndup(doc + i, j - i), doc_id, i);
    i = j;
  }
}

typedef struct {
  int num_docs;
  char** documents;
  int* to_process;
  pthread_mutex_t* to_process_lock;
  InvertedIndex ii;
} WorkerArgs;

void* Worker(void* args_ptr) {
  WorkerArgs* args = args_ptr;
  while (true) {
    while(pthread_mutex_lock(args->to_process_lock) != 0) {}
    const int doc_id = *args->to_process;
    ++(*args->to_process);
    pthread_mutex_unlock(args->to_process_lock);
    if (doc_id >= args->num_docs) {
      break;
    }
    IndexDoc(args->documents[doc_id], doc_id, &args->ii);
  }
  return NULL;
}

void BuildInvertedIndex(int num_docs,
			char** documents,
			int num_threads,
			InvertedIndex* ii) {
  pthread_t* workers = malloc(num_threads * sizeof(pthread_t));
  assert(workers != NULL);
  WorkerArgs* args = malloc(num_threads * sizeof(WorkerArgs));
  assert(args != NULL);
  int to_process = 0;
  pthread_mutex_t to_process_lock;
  pthread_mutex_init(&to_process_lock, NULL);
  for (int i = 0; i < num_threads; ++i) {
    args[i].num_docs = num_docs;
    args[i].documents = documents;
    args[i].to_process = &to_process;
    args[i].to_process_lock = &to_process_lock;
    InvertedIndexInit(&args[i].ii);
    pthread_create(&workers[i], NULL, Worker, &args[i]);
  }
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(workers[i], NULL);
    InvertedIndexMergeWith(ii, &args[i].ii);
    InvertedIndexDestroy(&args[i].ii);
  }
  free(workers);
  free(args);
}
