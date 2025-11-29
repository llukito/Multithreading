#include "competition.h"

#include <stdlib.h>

#include "robot.h"

void CompetitionInit(Competition* comp) {
    comp->canAdd  =true;
    pthread_mutex_init(&comp->lock, NULL);
    comp->length = 0;
    sem_init(&comp->waitForWinner, 0, 0);
    pthread_cond_init(&comp->cond, NULL);
    comp->battlesOngoing = 0;
}

typedef struct{
  Robot** freeRobots;
  pthread_mutex_t* lock;
  Robot* r1;
  Robot* r2;
  int* length;
  int* battlesOngoing;
  pthread_cond_t* cond;
} battleArgument;

void* battle(void* argument){
  battleArgument* arg = (battleArgument*)argument;
  bool res = RobotFightWith(arg->r1, arg->r2);
  pthread_mutex_lock(arg->lock);
  if(res){
    arg->freeRobots[*arg->length] = arg->r1;
  } else {  
    arg->freeRobots[*arg->length] = arg->r2;
  }
  *arg->length = *arg->length + 1;
  *arg->battlesOngoing = *arg->battlesOngoing -1 ;
  pthread_cond_signal(arg->cond);
  pthread_mutex_unlock(arg->lock);
  free(arg);
  return NULL;
}

void* function(void* argument){
  Competition* arg = (Competition*)argument;
  while(true){
    pthread_mutex_lock(&arg->lock);
    
    while(arg->length < 2){
      if(!arg->canAdd && arg->length == 1 && arg->battlesOngoing == 0){
        pthread_mutex_unlock(&arg->lock);
        sem_post(&arg->waitForWinner);
        return NULL;
      }
      pthread_cond_wait(&arg->cond, &arg->lock);
    }

    if(arg->length > 1){
      pthread_t battleThread;
      battleArgument* bt = malloc(sizeof(battleArgument));
      bt->freeRobots = arg->freeRobots;
      bt->length = &arg->length;
      bt->lock = &arg->lock;
      bt->r1 = arg->freeRobots[arg->length-1];
      bt->r2 = arg->freeRobots[arg->length-2];
      bt->battlesOngoing = &arg->battlesOngoing;
      bt->cond = &arg->cond;
      arg->length-=2;
      arg->battlesOngoing++;
      pthread_create(&battleThread, NULL, battle, bt);
      pthread_detach(battleThread);
    }

    pthread_mutex_unlock(&arg->lock);
  }
  return NULL;
}

void CompetitionStart(Competition* comp) {
  pthread_t thread;
  pthread_create(&thread, NULL, function, comp);
  pthread_detach(thread);
}

const Robot* CompetitionStop(Competition* comp) {
  pthread_mutex_lock(&comp->lock);
  comp->canAdd = false;
  pthread_cond_signal(&comp->cond);
  pthread_mutex_unlock(&comp->lock);

  sem_wait(&comp->waitForWinner);

  Robot* res;
  pthread_mutex_lock(&comp->lock);
  res = comp->freeRobots[0];
  pthread_mutex_unlock(&comp->lock);

  return res;
}

bool CompetitionAddParticipant(Competition* comp, const Robot* robot) {
  pthread_mutex_lock(&comp->lock);
  if(comp->canAdd){
    comp->freeRobots[comp->length] = (Robot*)robot;
    comp->length++;
    pthread_cond_signal(&comp->cond);
    pthread_mutex_unlock(&comp->lock);
    return true;
  }
  pthread_mutex_unlock(&comp->lock);

  return false;
}
