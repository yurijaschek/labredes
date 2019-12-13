#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int init_server(uint16_t port, int backlog)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };
    if(bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("could not bind port");
        return -1;
    }

    if(listen(sockfd, backlog) < 0)
    {
        perror("listen");
        return -1;
    }

    return sockfd;
}

int listen_fd;

void sig_handler(int signum)
{
    shutdown(listen_fd, SHUT_RDWR);
    close(listen_fd);
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s port\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[1]);
    if(port == 0 || port > 65535)
    {
        printf("Invalid port: should be a positive integer less than 65536\n");
        return -1;
    }

    listen_fd = init_server(port, 16);

    signal(SIGINT, sig_handler); // To shutdown the server with Ctrl-C

    for(;;) // Accepting connections
    {

    }


    return 0;
}
