#include "index.h"

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	bool done;
	bool inProcess;
	pthread_mutex_t lock; // protects its inner content
} document_info;

typedef struct {
	document_info* content;
	InvertedIndex* ii;
	pthread_mutex_t lock; // protects ii
	char** documents;
	int num_docs;
	int left;
} shared_info;

void* startIndexing(void* ar){ // we don't need argument
	shared_info* arg = (shared_info*)ar;
	while(true){
		pthread_mutex_lock(&arg->lock);
		if(arg->left == 0){
			pthread_mutex_unlock(&arg->lock);
			break;
		}
		pthread_mutex_unlock(&arg->lock);
		
		int i = 0;
		for(; i < arg->num_docs; i++){
			pthread_mutex_lock(&arg->content[i].lock);
			if(!arg->content[i].done && !arg->content[i].inProcess){
				arg->content[i].inProcess = true;
				pthread_mutex_unlock(&arg->content[i].lock);
				break;
			}
			pthread_mutex_unlock(&arg->content[i].lock);
		}

		if(i == arg->num_docs){
			break;
		}

		// i can safely get specific string, cause only this thread is accessing it
		char* str = arg->documents[i];
		int start = 0;
		bool to_be_started = true;
		for(int j = 0; j < strlen(str); j++){
			if(str[j] == ' '){
				if(to_be_started)continue; // double space
				// j is end
				char* curr = malloc(j-start+1);
				memcpy(curr, str+start, j-start);
				curr[j-start] = '\0';

				pthread_mutex_lock(&arg->lock);

				InvertedIndexInsert(arg->ii, curr, i, start);

				pthread_mutex_unlock(&arg->lock);
				to_be_started = true;
			} else {
				if(to_be_started){
					start = j;
					to_be_started = false;
				}
			}
		}
		// if there was not a " " in the end
		if(!to_be_started){
			char* curr = malloc(strlen(str)-start+1);
			memcpy(curr, str+start, strlen(str)-start);
			curr[strlen(str)-start] = '\0';

			pthread_mutex_lock(&arg->lock);

			InvertedIndexInsert(arg->ii, curr, i, start);

			pthread_mutex_unlock(&arg->lock);
		}

		// some indexing

		pthread_mutex_lock(&arg->lock);
		arg->left--;
		pthread_mutex_unlock(&arg->lock);

		pthread_mutex_lock(&arg->content[i].lock);
		arg->content[i].inProcess = false;
		arg->content[i].done = true;
		pthread_mutex_unlock(&arg->content[i].lock);
	}

	return NULL;
}

void BuildInvertedIndex(int num_docs,
			char** documents,
			int num_threads,
			InvertedIndex* ii) 
{	

	document_info document_infos[num_docs];
	for(int i = 0; i < num_docs; i++){
		document_infos[i].done = false;
		document_infos[i].inProcess = false;
		pthread_mutex_init(&document_infos[i].lock, NULL);
	}
	shared_info inf;
	inf.content = document_infos;
	inf.ii = ii;
	inf.documents = documents;
	inf.num_docs = num_docs;
	inf.left = num_docs;
	pthread_mutex_init(&inf.lock, NULL);

	pthread_t threads[num_threads];

	for(int i = 0; i < num_threads; i++){
		pthread_create(&threads[i], NULL, startIndexing, &inf);
	}

	for(int i = 0; i < num_threads; i++){
		pthread_join(threads[i], NULL);
	}

	for(int i = 0; i < num_docs; i++){
		pthread_mutex_destroy(&document_infos[i].lock);
	}

	pthread_mutex_destroy(&inf.lock);
	
}
