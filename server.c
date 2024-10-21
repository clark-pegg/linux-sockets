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

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

void *socket_loop(void *args)
{
    signal(SIGINT, sig_handler);

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int sockfd = *(int *)args;
    char response[1024] =
        "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n";

    FILE *file = fopen("public/index.html", "r");

    while (fgets(response + strlen(response), 100, file))
        ;

    while (keep_running)
    {
        int new_socket = accept(sockfd, (struct sockaddr *)&address, &addrlen);
        printf("Connection from: %s\n", inet_ntoa(address.sin_addr));
        // recv(new_socket, buffer, 1024 - 1, 0);
        // printf("%s\n", buffer);
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int fail = bind(sockfd, (struct sockaddr *)&address, addrlen);

    if (fail)
    {
        fprintf(stderr, "Couldn't bind to socket!\n");
        return 1;
    }

    listen(sockfd, 3);

    void *args = malloc(sizeof(int *));
    args = &sockfd;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &socket_loop, args);

    printf("Server running!\n");

    while (keep_running)
    {
        sleep(1);
    }

    shutdown(sockfd, SHUT_RDWR);

    close(sockfd);

    return 0;
}
