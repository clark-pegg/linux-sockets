#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include "http-headers.h"

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

void *socket_loop(void *args)
{
    int sockfd = *(int *)args;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    char buffer[1024] = {0};

    while (keep_running)
    {
        int new_socket = accept(sockfd, (struct sockaddr *)&address, &addrlen);

        printf("Connection from: %s\n", inet_ntoa(address.sin_addr));

        int read = recv(new_socket, buffer, 1024 - 1, 0);

        char **headerInfo = httpHeaderReader(buffer, read);

        char *filepath = malloc(strlen("public") + strlen(headerInfo[1]) + 1);
        filepath[0] = '\0';
        strcat(filepath, "public");
        strcat(filepath, headerInfo[1]);

        FILE *file = fopen(filepath, "rb");

        char *response = malloc(sizeof(char) * 1024);
        response[0] = '\0';

        if (!strcmp(filepath, "public/favicon.ico"))
        {
            strcpy(response, "HTTP/1.1 200 OK\nContent-Type: image/*\n\n");
        }
        else
        {
            strcpy(response, "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n");
        }

        int headerLength = strlen(response);

        free(filepath);

        fseek(file, 0, SEEK_END);
        int fileLength = ftell(file);
        rewind(file);

        response = realloc(response, strlen(response) + fileLength);

        fread(response + strlen(response), fileLength, 1, file);

        fclose(file);

        send(new_socket, response, headerLength + fileLength, 0);

        shutdown(new_socket, SHUT_RDWR);

        close(new_socket);
    }

    printf("All done!\n");
}

int main()
{
    signal(SIGINT, sig_handler);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    address.sin_family = AF_INET;
    inet_aton("192.168.2.118", &address.sin_addr);
    address.sin_port = htons(80);

    int fail = bind(sockfd, (struct sockaddr *)&address, addrlen);

    if (fail)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
    }

    listen(sockfd, 5);

    void *args = malloc(sizeof(int *));
    args = &sockfd;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &socket_loop, args);

    printf("Server running!\n");

    while (keep_running)
    {
        sleep(1);
    }

    printf("\nShutting down...\n");

    shutdown(sockfd, SHUT_RDWR);

    close(sockfd);

    printf("Shutdown succesful!\n");

    return 0;
}
