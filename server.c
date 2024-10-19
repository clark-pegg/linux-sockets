#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(sockfd, (struct sockaddr *)&address, addrlen);

    listen(sockfd, 1);

    int new_socket = accept(sockfd, (struct sockaddr *)&address, &addrlen);

    FILE *file = fopen("index.html", "r");

    ssize_t valread;
    char buffer[1024] = {0};
    char response[1024] = "HTTP/1.1 200 OK\nContent-Type: text/html; charset=utf-8\n\n";

    while (fgets(response + strlen(response), 100, file))
        ;

    valread = read(new_socket, buffer, 1024 - 1);
    int sent = send(new_socket, response, strlen(response), MSG_EOR);
    printf("%d\\%d\n", sent, strlen(response));

    close(new_socket);

    close(sockfd);

    return 0;
}
