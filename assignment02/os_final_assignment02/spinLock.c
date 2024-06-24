#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

//2번 문제 : spinLock  구현

// 스핀락 구조체 정의
typedef struct {
    atomic_flag lock;
} Spinlock;

// 스핀락 초기화 함수
void spinlock_init(Spinlock *spinlock) {
    atomic_flag_clear(&spinlock->lock);
}

// 스핀락 잠금 함수
void spinlock_lock(Spinlock *spinlock) {
    while (atomic_flag_test_and_set(&spinlock->lock)) {
        // 바쁜 대기
    }
}

// 스핀락 해제 함수
void spinlock_unlock(Spinlock *spinlock) {
    atomic_flag_clear(&spinlock->lock);
}

// 스핀락 변수
Spinlock spinlock;

// 스레드 함수
void* thread_func(void* arg) {
    int thread_id = *((int*)arg);
    for (int i = 0; i < 5; ++i) {
        spinlock_lock(&spinlock);

        // 임계 구역 시작
        printf("Thread %d is in the critical section\n", thread_id);
        usleep(100000); 
        // 임계 구역 끝

        spinlock_unlock(&spinlock);
        usleep(100000); 
    }
    return NULL;
}

int main() {
    pthread_t threads[2];
    int thread_ids[2] = {0, 1};

    // 스핀락 초기화
    spinlock_init(&spinlock);

    // 스레드 생성
    for (int i = 0; i < 2; ++i) {
        pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]);
    }

    // 스레드 종료 대기
    for (int i = 0; i < 2; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
