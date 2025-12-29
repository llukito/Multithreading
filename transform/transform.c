#include "transform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    int generation;
    pthread_mutex_t barrierLock;
    pthread_cond_t barrierCond;
    int value;
    int rememberedValue;
} barrier;

void barrierInit(barrier* br, int val){
    br->value = val;
    br->rememberedValue = val;
    br->generation = 0;
    pthread_mutex_init(&br->barrierLock, NULL);
    pthread_cond_init(&br->barrierCond, NULL);
}

void barrierDestroy(barrier* br){
    pthread_mutex_destroy(&br->barrierLock);
    pthread_cond_destroy(&br->barrierCond);
    br->generation = 0;
    br->value = 0;
    br->rememberedValue = 0;
}

void barrierWait(barrier* br){
    pthread_mutex_lock(&br->barrierLock);
    br->value--;
    if(br->value != 0){
        int gen = br->generation;
        while(br->generation == gen){
            pthread_cond_wait(&br->barrierCond, &br->barrierLock);
        }
    } else {
        br->value = br->rememberedValue;
        br->generation++;
        pthread_cond_broadcast(&br->barrierCond);
    }
    pthread_mutex_unlock(&br->barrierLock);
}

typedef struct {
    barrier* br;
    int* data;
    int* nexts;
    int id;
    int n;
} tree_arg;

bool over(int* data, int n){
    if(n == 0)return true;
    int first = data[0];
    for(int i = 1; i < n; i++){
        if(first != data[i])return false;
    }
    return true;
}

void* grow(void* ar){
    tree_arg* arg = (tree_arg*)ar;

    int* res = malloc(sizeof(int));
    *res = 0;

    while(true){
        if(over(arg->data, arg->n)){
            break;
        }

        if(arg->id == 0){ // only right neigh
            if(arg->data[arg->id] < arg->data[arg->id+1]){
                arg->nexts[arg->id] = arg->data[arg->id+1];
            }
        } else if(arg->id == arg->n - 1){ // only left neigh
            if(arg->data[arg->id] < arg->data[arg->id-1]){
                arg->nexts[arg->id] = arg->data[arg->id-1];
            }
        } else { // both left and right neigh
            if((arg->data[arg->id] < arg->data[arg->id+1])  ||
                (arg->data[arg->id] < arg->data[arg->id-1])){
                arg->nexts[arg->id] = max(arg->data[arg->id+1], arg->data[arg->id-1]);
            }
        }

        barrierWait(arg->br);

        arg->data[arg->id] = arg->nexts[arg->id];

        barrierWait(arg->br);
        *res = *res + 1;
    }

    return res;
}

int Transform(int* heights, int n) {
    tree_arg arguments[n];
    barrier br;
    barrierInit(&br, n);
    int nexts[n];
    for(int i = 0; i < n; i++)nexts[i] = heights[i];

    pthread_t threads[n];
    
    for(int i = 0; i < n; i++){
        arguments[i].br = &br;
        arguments[i].data = heights;
        arguments[i].id = i;
        arguments[i].n = n;
        arguments[i].nexts = nexts;
        pthread_create(&threads[i], NULL, grow, &arguments[i]);
    }

    int iterations = 0;

    for(int i = 0; i < n; i++){
        void* res;
        pthread_join(threads[i], &res);
        iterations = *(int*)res;
        free(res);
    }

    barrierDestroy(&br);

    return iterations;
}
