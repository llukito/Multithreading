#ifndef PARADIGMS_ROBOTS_COMPETITION_H_
#define PARADIGMS_ROBOTS_COMPETITION_H_

#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "robot.h"

typedef struct {
  Robot* freeRobots[1005];
  bool canAdd;
  pthread_mutex_t lock;

  int length;

  sem_t waitForWinner;
  pthread_cond_t cond;

  int battlesOngoing;
} Competition;

// უკეთებს შეჯიბრს ინიციალიზაციას.
void CompetitionInit(Competition* comp);
// უნდა გაუშვას შეჯიბრის მენეჯმენტისთვის მთავარი ნაკადი.
void CompetitionStart(Competition* comp);
// უნდა აკრძალოს შეჯიბრში ახალი მონაწილეების დამატება,
// დაელოდოს უკვე დამატებულ მონაწილეებს შორის გამარჯვებულს,
// და დააბრუნოს იგი.
const Robot* CompetitionStop(Competition* comp);
// აბრუნებს true თუ რობოტი წარმატებით დაემატა შეჯიბრში,
// და false წინააღმდეგ შემთხვევაში.
bool CompetitionAddParticipant(Competition* comp, const Robot* robot);

#endif // PARADIGMS_ROBOTS_COMPETITION_H_
