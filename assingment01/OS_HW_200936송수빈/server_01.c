#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>

#define PORT 8090

void handle_client(int new_socket) {
    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain" \
                  "Content-Length: 20\n\nMy first web server!";
    char buffer[30000] = {0};
    long valread = read(new_socket, buffer, 30000);
    printf("Request received:\n%s\n", buffer);
    sleep(5); 
    write(new_socket, hello, strlen(hello));
    printf("Hello message sent\n");
    close(new_socket);
    exit(0);
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
    } /*서버 소켓 생성*/
    printf("Socket created successfully\n");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    /*소켓 주소 설정*/

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Bind successful\n");/*소켓 바인딩*/

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

        pid_t pid = fork(); /*새로운 자식 프로세스 생성 */
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

             if (pid == 0) {  /* 자식 프로세스의 경우*/
            close(server_fd);  /*자식 프로세스 ->  서버 소켓을 닫기*/
            handle_client(new_socket);  /*클라이언트 요청 처리*/
        } else {  // 부모 프로세스의 경우
            close(new_socket);  /* 부모 프로세스는 클라이언트 소켓을 닫기*/
            waitpid(-1, NULL, WNOHANG);  /* 비동기적으로 자식 프로세스의 종료를 기다림*/
        }
    }
    return 0;
}
