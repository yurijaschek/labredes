#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

uint64_t get_posix_clock_time()
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t) (ts.tv_sec * 1000000000LL + ts.tv_nsec);
    else
        return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s port\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if(port == 0 || port > 65535)
    {
        printf("Invalid port: should be a positive integer less than 65536\n");
        return EXIT_FAILURE;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
        .sin_zero = "", // to avoid warning
    };
    if(bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("could not bind port");
        return EXIT_FAILURE;
    }

    if(listen(listen_fd, 1) < 0)
    {
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("Waiting for incoming connection...\n");
    int client_fd = accept(listen_fd, NULL, NULL);
    if(client_fd < 0)
    {
        perror("accept");
        return EXIT_FAILURE;
    }

    uint8_t buffer[1024];
    printf("Connected! Receiving data...\n");
    uint64_t bits = 0;
    uint64_t nanoseconds = 0;
    uint64_t prev_time = get_posix_clock_time();
    for(;;)
    {
        int len = recv(client_fd, buffer, sizeof(buffer), 0);
        if(len <= 0) break;
        bits += 8*len;
        uint64_t curr_time = get_posix_clock_time();
        nanoseconds += curr_time - prev_time;
        prev_time = curr_time;
        if(nanoseconds >= 1000000000LL)
        {
            printf("%d Mbps\n", (int)(bits/1000000));
            bits = 0;
            nanoseconds %= 1000000000LL;
        }
    }

    return 0;
}
