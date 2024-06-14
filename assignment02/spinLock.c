#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

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
        // 바쁜 대기: 다른 스레드가 락을 해제할 때까지 반복
    }
}

// 스핀락 해제 함수
void spinlock_unlock(Spinlock *spinlock) {
    atomic_flag_clear(&spinlock->lock);
}

void* critical_section(void* arg) {
    int thread_id = *((int*)arg); // 스레드 ID
    for (int i = 0; i < 5; ++i) {
        spinlock_lock(&spinlock); // 스핀락 잠금

        // 임계 구역 시작
        printf("Thread %d is in the critical section\n", thread_id);
        usleep(100000); 
        // 임계 구역 끝

        spinlock_unlock(&spinlock); // 스핀락 해제
        usleep(100000);
    }
    return NULL;
}

// 전역 스핀락 변수
Spinlock spinlock;

int main() {
    pthread_t thread_handles[2];
    int thread_ids[2] = {0, 1}; // 스레드 ID

    // 스핀락 초기화
    spinlock_init(&spinlock);

    // 스레드 생성
    for (int i = 0; i < 2; ++i) {
        pthread_create(&thread_handles[i], NULL, critical_section, &thread_ids[i]);
    }

    // 스레드 종료 대기
    for (int i = 0; i < 2; ++i) {
        pthread_join(thread_handles[i], NULL);
    }

    return 0;
}
