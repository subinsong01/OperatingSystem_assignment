#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

// in your browser type: http://localhost:8090
// IF error: address in use then change the PORT number
#define PORT 8090
#define QUEUE_SIZE 10
#define MAX_THREAD_COUNT 100

char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain"
              "Content-Length: 20\n\nMy first web server!";

pthread_mutex_t queueMutex;

// Queue 구조체 정의 
typedef struct {
    int *data; // 메시지 저장용 배열 포인터
    int rear, front; // 큐의 앞 뒤 인덱스 
} Queue;

// 함수 원형 선언
void init(Queue *queue);
int get_queue_size(Queue *queue);
int enqueue(Queue *queue, int item);
int dequeue(Queue *queue);

// 큐 초기화
void init(Queue *queue) {
    // 큐의 메시지 배열을 동적으로 할당하고 -1로 초기화하기
    queue->data = (int *)malloc(QUEUE_SIZE * sizeof(int)); 
    for (int i = 0; i < QUEUE_SIZE; i++) { // 배열의 모든 요소를 -1로 초기화하기 위한 반복문
        queue->data[i] = -1;
    }
    queue->rear = queue->front = -1; // rear와 front 인덱스를 -1로 설정해서 큐가 비어있음을 표시
}

// 큐의 현재 크기를 반환하는 함수
int get_queue_size(Queue *queue) {
    if (queue->rear >= queue->front) {
        // rear가 front보다 크거나 같은 경우 큐의 크기를 계산
        return queue->rear - queue->front;
    } else {
        // rear가 front보다 작은 경우 계산 
        return QUEUE_SIZE - (queue->front - queue->rear);
    }
}

// 큐에 새로운 요소를 추가하는 함수
int enqueue(Queue *queue, int item) {
    // 큐의 숫자가 최대치면 큐가 가득 찬 상태
    if (get_queue_size(queue) == QUEUE_SIZE - 1) {
        printf("overflow\n");
        return -1;
    } else {
        // rear 인덱스를 한 칸 이동하고 새로운 아이템을 추가함
        queue->rear = (queue->rear + 1) % QUEUE_SIZE;
        queue->data[queue->rear] = item;
        return 1;
    }
}

// 큐에서 요소를 제거하는 함수
int dequeue(Queue *queue) {
    if (get_queue_size(queue) == 0) { // 큐가 비어있는지 확인하기
        return -1; // 실패면 -1 반환
    } else {
        // front 인덱스를 한 칸 이동
        queue->front = (queue->front + 1) % QUEUE_SIZE;
        int removed_value = queue->data[queue->front];
        // 제거된 위치를 -1로 설정하여 비어있음을 표시
        queue->data[queue->front] = -1;
        return removed_value; // 제거된 요소 반환
    }
}

// 스레드에 전달할 인자 구조체 정의
typedef struct {
    Queue *queue;
    int *thread_count;
} ThreadArgs;

void *worker_thread(void *args) {
    pthread_detach(pthread_self()); // 스레드를 분리하여 독립적으로 실행함
    long valread;
    int client_socket;
    Queue *queue = ((ThreadArgs *)args)->queue;
    int *active_thread_count = ((ThreadArgs *)args)->thread_count;

    while (1) {
        pthread_mutex_lock(&queueMutex); // 큐 접근을 위한 뮤텍스 잠금
        if (get_queue_size(queue) <= 0.2 * QUEUE_SIZE && *active_thread_count > 2) { // 큐 크기가 작고 스레드 수가 2개 이상인 경우 스레드 수 감소됨
            printf("Thread num is %d\n", *active_thread_count);
            (*active_thread_count)--;
            pthread_mutex_unlock(&queueMutex);
            break;
        }
        client_socket = dequeue(queue);
        pthread_mutex_unlock(&queueMutex);

        if (client_socket < 0) {
            usleep(10000); // 큐가 비어있으면 10ms 대기
            continue;
        }

        char buffer[30000] = {0};
        valread = read(client_socket, buffer, sizeof(buffer));
        sleep(5); // 처리 지연을 시뮬레이션
        write(client_socket, hello, strlen(hello));
        printf("Hello message sent. Queue size: %d\n", get_queue_size(queue));
        close(client_socket);
    }
    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_address;
    int address_length = sizeof(server_address);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    memset(server_address.sin_zero, '\0', sizeof server_address.sin_zero);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }
    // 메시지 큐 초기화
    Queue *request_queue = (Queue *)malloc(sizeof(Queue));
    init(request_queue);

    pthread_t thread_ids[MAX_THREAD_COUNT];
    ThreadArgs **thread_args = (ThreadArgs **)malloc(MAX_THREAD_COUNT * sizeof(ThreadArgs *));
    int *active_thread_count = (int *)malloc(sizeof(int));
    *active_thread_count = 0;

    // 스레드 생성 및 초기화 시킴
    for (int i = 0; i < MAX_THREAD_COUNT; i++) {
        thread_args[i] = (ThreadArgs *)malloc(sizeof(ThreadArgs));
        thread_args[i]->queue = request_queue;
        thread_args[i]->thread_count = active_thread_count;
    }

    // 첫 번째 스레드 생성
    if (pthread_create(&thread_ids[*active_thread_count], NULL, worker_thread, (void *)thread_args[*active_thread_count]) < 0) {
        perror("Thread creation failed");
        return 1;
    }
    (*active_thread_count)++; // thread 수 증가시킴

    while (1) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        printf("Queue size: %d\n", get_queue_size(request_queue));
        printf("Thread count: %d\n", *active_thread_count);

        if ((client_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&address_length)) < 0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        while (1) {
            pthread_mutex_lock(&queueMutex);
            if (enqueue(request_queue, client_socket) < 0) {
                usleep(10000); // 큐가 가득 찬 경우에는 대기하기
            } else {
                pthread_mutex_unlock(&queueMutex);
                break; // 루프 탈출
            }
            pthread_mutex_unlock(&queueMutex);
        }

        pthread_mutex_lock(&queueMutex);
        // 큐가 가득 찬 경우 새로운 스레드를 하나씩 추가
        if (get_queue_size(request_queue) >= 0.8 * QUEUE_SIZE && *active_thread_count < MAX_THREAD_COUNT) { // thread 수가 80% 이상인 경우 새로운 thread를 추가
            if (*active_thread_count < MAX_THREAD_COUNT) {
                if (pthread_create(&thread_ids[*active_thread_count], NULL, worker_thread, (void *)thread_args[*active_thread_count]) < 0) {
                    perror("Thread creation failed");
                } else {
                    (*active_thread_count)++;
                    printf("Thread added. Current thread count: %d\n", *active_thread_count);
                }
            }
        // 큐가 20% 이하로 비어있으면 thread 수 감소시키기
        } else if (get_queue_size(request_queue) <= 0.2 * QUEUE_SIZE && *active_thread_count > 2) {
            if (*active_thread_count > 2) {
                printf("Thread num is %d\n", *active_thread_count);
                (*active_thread_count)--; // thread 수 감소시키기
            }
        }
        pthread_mutex_unlock(&queueMutex);
    }

    return 0;
}
