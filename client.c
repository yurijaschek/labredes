#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    if(argc != 3)
    {
        printf("Usage: %s address port\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[2]);
    if(port == 0 || port > 65535)
    {
        printf("Invalid port: should be a positive integer less than 65536\n");
        return EXIT_FAILURE;
    }

    int ip_addr;
    if(inet_pton(AF_INET, argv[1], &ip_addr) != 1)
    {
        perror("Invalid IPv4 address");
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = ip_addr,
        .sin_port = htons(port),
        .sin_zero = "", // to avoid warning
    };
    if(connect(sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0)
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    uint8_t buffer[1024];
    for(int i=0; i < (int)sizeof(buffer); i++)
    {
        buffer[i] = 'a' + i % 26;
    }
    buffer[sizeof(buffer)-1] = '\n';

    printf("Connected! Sending data...\n");
    uint64_t bits = 0;
    uint64_t nanoseconds = 0;
    uint64_t prev_time = get_posix_clock_time();
    for(;;)
    {
        int len = send(sockfd, buffer, sizeof(buffer), 0);
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
