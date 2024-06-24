//1번과 2번 비교를 위한 1번 성능평가를 위해 코드 생성

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N_COUNTER 4 // the size of a shared buffer
#define MILLI 1000  // time scale
#define NUM_RUNS 10 // 평균 실행 시간을 계산하기 위한 것

void mywrite(int n);
int myread();

pthread_mutex_t critical_section; // POSIX mutex
sem_t semWrite, semRead; // POSIX semaphore
int queue[N_COUNTER]; // shared buffer
int wptr = 0; // write pointer for queue[]
int rptr = 0; // read pointer for queue[];

// producer thread function
void* producer(void* arg) { 
  for(int i=0; i<10; i++) {
    sem_wait(&semWrite); 
    pthread_mutex_lock(&critical_section);  // 뮤텍스 잠금 
    queue[wptr] = i; // 버퍼에 데이터 쓰기 
    wptr = (wptr + 1) % N_COUNTER; // write pointer 이동 
    pthread_mutex_unlock(&critical_section); // 뮤텍스 해제
    sem_post(&semRead); // 새로운 데이터가 추가된 것을 소비자 스레드에게 알림

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
    sem_wait(&semRead); 
    pthread_mutex_lock(&critical_section); // 뮤텍스 잠금 
    int n = queue[rptr]; // 데이터 읽어옴 
    rptr = (rptr + 1) % N_COUNTER; // 읽기 작업이 끝나고 다음 위치로 
    pthread_mutex_unlock(&critical_section); // 뮤텍스 해제
    sem_post(&semWrite); // 빈 공간 사용이 가능하다는 신호
    printf("\tconsumer : read %d\n", n);

    // sleep m miliseconds
    int m = rand()%10; 
    usleep(MILLI*m*10); // m*10 
  }
  return NULL;
}

// 실행시간 측정
double measure_execution_time() {
    pthread_t t[2]; // thread structure
    struct timespec start, end;

    // 현재 시간을 CLOCK_MONOTONIC 기준으로 가지고 와서 start에 저장한다.
    clock_gettime(CLOCK_MONOTONIC, &start);

    // 생산자와 소비자 스레드 생성
    pthread_create(&t[0], NULL, producer, NULL); 
    pthread_create(&t[1], NULL, consumer, NULL); 

    for(int i=0; i<2; i++)
        pthread_join(t[i], NULL);

    //end에 서장
    clock_gettime(CLOCK_MONOTONIC, &end);

    // 시작 시간과 끝 시간의 차이를 계산
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;
    return seconds + nanoseconds * 1e-9;
}

int main() {
    pthread_t t[2]; // thread structure

    pthread_mutex_init(&critical_section, NULL); // init mutex

    // init semaphore
    /* [Write here] */
    sem_init(&semWrite, 0, N_COUNTER); // semWrite 초기화
    sem_init(&semRead, 0, 0); // semRead 초기화

    double total_time = 0.0; // 전체 실행 시간을 저장할 변수
    for(int i = 0; i < NUM_RUNS; i++) {
        total_time += measure_execution_time();// 실행 시간을 측정+누적
    }

    //destroy the semaphores
    /* [Write here] */
    sem_destroy(&semWrite); 
    sem_destroy(&semRead);
    pthread_mutex_destroy(&critical_section); // destroy mutex 

    printf("평균시간: %.5f seconds.\n", total_time / NUM_RUNS);

    return 0;
}
