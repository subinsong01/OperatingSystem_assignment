#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N_COUNTER 4 // the size of a shared buffer
#define MILLI 1000  // time scale

void mywrite(int n);
int myread();

pthread_mutex_t critical_section; // POSIX mutex
sem_t semWrite, semRead; // POSIX semaphore
int queue[N_COUNTER]; // shared buffer
int wptr; // write pointer for queue[]
int rptr; // read pointer for queue[]

// producer thread function
void* producer(void* arg) { 
  for(int i=0; i<10; i++) {
    mywrite(i); /**** write i into the shared memory ****/
    printf("producer : wrote %d\n", i);

    // sleep m miliseconds
    int m = rand()%10;
    usleep(MILLI*m*10); // m*10
  }
  return NULL;
}

// consumer thread function
void* consumer(void* arg) { 
  for(int i=0; i<10; i++) {
    int n = myread(); /**** read a value from the shared memory ****/ 
    printf("\tconsumer : read %d\n", i);

    // sleep m miliseconds
    int m = rand()%10; 
    usleep(MILLI*m*10); // m*10 
  }
  return NULL;
}

// write n into the shared memory
void mywrite(int n) { 
  sem_wait(&semWrite); 
  pthread_mutex_lock(&critical_section);  // 뮤텍스 잠금 

  queue[wptr] = n; //버퍼에 데이터 쓰기 
  wptr = (wptr + 1) % N_COUNTER; // write pointer 이동 

  pthread_mutex_unlock(&critical_section); // 뮤텍스 해제
  sem_post(&semRead); //공유 버퍼에 새로운 데이터가 추가된거 소비자 스레드한테 알리기 
}

// write a value from the shared memory
int myread() { 
  /* [Write here] */
   sem_wait(&semRead); 
  pthread_mutex_lock(&critical_section); // 뮤텍스 잠금 

  int n = queue[rptr]; //rptr이 가리키는 위치에서 데이터 읽어옴 
  rptr = (rptr + 1) % N_COUNTER; //읽기 작업이 끝나고 다음 위치로 이동 

  pthread_mutex_unlock(&critical_section); 
  sem_post(&semWrite); // 빈공간 사용이 가능하다는 신호

  return n;
}

int main() {
  pthread_t t[2]; // thread structure
  srand(time(NULL)); 

  pthread_mutex_init(&critical_section, NULL); // init mutex

  // init semaphore
  /* [Write here] */

  sem_init(&semWrite, 0, N_COUNTER); // semWrite 초기화
  sem_init(&semRead, 0, 0); // 0으로 semRead 초기화

  // create the threads for the producer and consumer
  pthread_create(&t[0], NULL, producer, NULL); 
  pthread_create(&t[1], NULL, consumer, NULL); 

  for(int i=0; i<2; i++)
    pthread_join(t[i],NULL); // wait for the threads

  //destroy the semaphores
  /* [Write here] */

  sem_destroy(&semWrite); 
  sem_destroy(&semRead);

  pthread_mutex_destroy(&critical_section); // destroy mutex 
  return 0;
}

