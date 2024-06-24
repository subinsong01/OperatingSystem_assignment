#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>// <pthread.h> : 멀티스레드 프로그래밍을 위한 함수 제공하는 라이브라리

#define PORT 8090

void* handle_client(void* arg) {
    int new_socket = *(int*)arg;
    free(arg); // 소켓 디스크립터를 저장한 메모리 해제
    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain" \
                  "Content-Length: 20\n\nMy first web server!";
    char buffer[30000] = {0};
    long valread = read(new_socket, buffer, 30000);
    printf("Request received:\n%s\n", buffer);
    sleep(5); 
    write(new_socket, hello, strlen(hello));
    printf("Hello message sent\n");
    close(new_socket);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Bind successful\n");

    if (listen(server_fd, 10) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Listening on port %d\n", PORT);

    while (1)
    {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted\n");

        // 스레드를 사용하여 클라이언트 요청을 처리
        pthread_t thread_id;
        int* pclient = malloc(sizeof(int)); // 소켓 디스크립터를 저장할 메모리 할당
        *pclient = new_socket;
        if (pthread_create(&thread_id, NULL, handle_client, pclient) != 0) {
            perror("Failed to create thread");
            free(pclient); // 스레드 생성 실패 시 메모리 해제
        }
        pthread_detach(thread_id); // 스레드 분리
    }
    return 0;
}
