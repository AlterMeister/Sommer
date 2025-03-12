#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_PORT 8888
#define BUFFER_SIZE 512

int create_server_socket(int port);
int accept_client_connection(int server_sock, char *client_ip, int *client_port);
int receive_data(int client_sock, char *buffer, int buffer_size);
int send_data(int client_sock, const char *data);
void close_connection(int sock);

int main(void) {
    int server_sock, client_sock;
    char client_ip[20];
    int client_port;
    char buffer[BUFFER_SIZE];

    // 创建服务器套接字
    server_sock = create_server_socket(SERVER_PORT);
    if (server_sock < 0) {
        fprintf(stderr, "Failed to create server socket\n");
        return EXIT_FAILURE;
    }

    // 接受客户端连接
    client_sock = accept_client_connection(server_sock, client_ip, &client_port);
    if (client_sock < 0) {
        fprintf(stderr, "Failed to accept client connection\n");
        close_connection(server_sock);
        return EXIT_FAILURE;
    }

    printf("Client connected from %s:%d\n", client_ip, client_port);

    // 接收和发送数据
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // 接收数据
        if (receive_data(client_sock, buffer, BUFFER_SIZE) <= 0) {
            fprintf(stderr, "Failed to receive data or client disconnected\n");
            break;
        }

        printf("Received from client: %s\n", buffer);  // buffer就是结果了，可以用来解析

        /*
            
        */

        // 如果接收到 "exit"，则退出
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client requested to exit. Closing connection...\n");
            break;
        }

        // 发送响应数据
        if (send_data(client_sock, "Message received") < 0) {
            fprintf(stderr, "Failed to send data to client\n");
            break;
        }
    }

    // 关闭连接
    close_connection(client_sock);
    close_connection(server_sock);

    return EXIT_SUCCESS;
}

// 创建服务器套接字并绑定端口
int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 50) < 0) {
        perror("listen error");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// 接受客户端连接
int accept_client_connection(int server_sock, char *client_ip, int *client_port) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int connfd;

    connfd = accept(server_sock, (struct sockaddr *)&client_addr, &addrlen);
    if (connfd < 0) {
        perror("accept error");
        return -1;
    }

    // 将客户端IP地址转换为字符串
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    *client_port = ntohs(client_addr.sin_port);

    return connfd;
}

// 接收数据
int receive_data(int client_sock, char *buffer, int buffer_size) {
    int ret = recv(client_sock, buffer, buffer_size - 1, 0);
    if (ret < 0) {
        perror("recv error");
    } else if (ret == 0) {
        printf("Client disconnected\n");
    } else {
        buffer[ret] = '\0'; // 确保字符串以 null 结尾
    }
    return ret;
}

// 发送数据
int send_data(int client_sock, const char *data) {
    int ret = send(client_sock, data, strlen(data), 0);
    if (ret < 0) {
        perror("send error");
    }
    return ret;
}

// 关闭连接
void close_connection(int sock) {
    if (sock >= 0) {
        close(sock);
    }
}