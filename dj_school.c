#include "helper.h"

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define MAX_NUM_TEACHERS 10
#define MAX_NUM_STUDENTS 100

bool all_students_done;
sem_t studentsSem;
sem_t teachersSem;


int currTEACHERS = 0;

typedef struct {
  bool isFree;
  bool mark;
  int student_id;
  sem_t waitForSet;
  sem_t signalStudent;
} TeacherArgs;

TeacherArgs teacherArguments[MAX_NUM_TEACHERS];
pthread_mutex_t teacherLocks[MAX_NUM_TEACHERS];

sem_t waitForSets[MAX_NUM_TEACHERS];
sem_t signalStudents[MAX_NUM_TEACHERS];
int general = 0;

void* Teacher(void* args) {
  int id = *(int*)args;
  // IMPLEMENT
  while (true) {
    sem_wait(&teacherArguments[id].waitForSet);
    if(all_students_done)break;
    teacherArguments[id].mark = TeacherEvaluateStudent(id, teacherArguments[id].student_id);
    if( teacherArguments[id].mark)general++;
    sem_post(&teacherArguments[id].signalStudent);
  }
  TeacherDone(id);
  sem_post(&teachersSem);
  return NULL;
}

void* Student(void* args) {
  int id = *(int*)args;
  // IMPLEMENT
  while (true) {
    StudentRecordSet(id);
    int teachersId = -1;

    for(int i = 0;; i = (i+1)%currTEACHERS){
      pthread_mutex_lock(&teacherLocks[i]);
      if(teacherArguments[i].isFree){
        teachersId = i;
        break;
      }
      pthread_mutex_unlock(&teacherLocks[i]);
    }
    teacherArguments[teachersId].isFree = false;
    teacherArguments[teachersId].student_id = id;

    sem_post(&(teacherArguments[teachersId].waitForSet));
    sem_wait(&teacherArguments[teachersId].signalStudent);

    bool ans = teacherArguments[teachersId].mark;
    StudentWasEvaluatedByTeacher(id, teachersId, ans);
    teacherArguments[teachersId].isFree = true;

    pthread_mutex_unlock(&teacherLocks[teachersId]);

    if(ans)break;
  }
  StudentDone(id);
  sem_post(&studentsSem);
  return NULL;
}


void StartSchool(int num_students, int num_teachers) {
  currTEACHERS = num_teachers;
  sem_init(&studentsSem, 0, 0);
  sem_init(&teachersSem, 0, 0);
  all_students_done = false;

  for(int i = 0; i < num_teachers; i++){
    sem_init(&waitForSets[i], 0, 0);
    sem_init(&signalStudents[i], 0, 0);
    pthread_mutex_init(&teacherLocks[i], NULL);

    teacherArguments[i].isFree = true;
    teacherArguments[i].mark = false;
    teacherArguments[i].student_id = -1;
    teacherArguments[i].waitForSet = waitForSets[i];
    teacherArguments[i].signalStudent = signalStudents[i];
  }

  // IMPLEMENT: Initialize semaphores and mutexes if needed.
  int teacher_id[MAX_NUM_TEACHERS];
  int student_id[MAX_NUM_STUDENTS];
  for (int i = 0; i < num_teachers; ++i) {
    teacher_id[i] = i;
    pthread_t tid;
    pthread_create(&tid, NULL, Teacher, &teacher_id[i]);
    pthread_detach(tid);
  }
  for (int i = 0; i < num_students; ++i) {
    student_id[i] = i;
    pthread_t tid;
    pthread_create(&tid, NULL, Student, &student_id[i]);
    pthread_detach(tid);
  }
  // IMPLEMENT: Wait for the all students and teachers to finish.
  for(int i = 0; i < num_students; i++){
    sem_wait(&studentsSem);
  }

  all_students_done = true;

  for(int i = 0; i < num_teachers; i++){
    sem_post(&teacherArguments[i].waitForSet);
  }

  for(int i = 0; i < num_teachers; i++){
    sem_wait(&teachersSem);
  }

  // destroy stuff
  sem_destroy(&studentsSem);
  sem_destroy(&teachersSem);

  for(int i = 0; i < num_teachers; i++){
    sem_destroy(&waitForSets[i]);
    sem_destroy(&signalStudents[i]);
    pthread_mutex_destroy(&teacherLocks[i]);
  }

  SchoolEnd();
}