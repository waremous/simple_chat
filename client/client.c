#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#define STDIN_FD 0

int getstring(char *buf, int length);

int main(int argc, char **argv)
{

    struct sockaddr_in bind_addr, server_addr;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Error create socket.");
        exit(1);
    }

    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0)
    {
        perror("Bind error.");
        exit(2);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(5500);
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect error.");
        exit(3);
    }

    struct epoll_event ev, out_event;
    int epollfd;
    epollfd = epoll_create(2);
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev) == -1)
    {
        perror("epoll_ctl: add net socket error.");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FD;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FD, &ev) == -1)
    {
        perror("epoll_ctl: add stdin descriptor error.");
        exit(EXIT_FAILURE);
    }

    int ep_ret;
    while (1)
    {
        ep_ret = epoll_wait(epollfd, &out_event, 1, 1000);
        if (ep_ret == -1)
        {
            perror("epoll_wait error.");
            exit(EXIT_FAILURE);
        }
        else if (ep_ret == 0)
        {
            continue;
        }
        else
        {
            if (out_event.data.fd != STDIN_FD)
            {
                char msg[1024];
                int rec_ret = recv(sock, msg, sizeof(msg), 0);
                msg[rec_ret] = '\0';
                printf(msg);
            }
            else
            {
                char msg[1024];
                int string_len = getstring(msg, sizeof(msg));
                send(sock, msg, string_len, 0);
            }
        }
    }
    close(sock);
}

int getstring(char *string_buf, int length)
{
    char c = 0;
    int i = 0;
    while ((c = getchar()) != '\n' && length > i - 2)
    {
        string_buf[i] = c;
        i++;
    }
    string_buf[i++] = '\n';
    string_buf[i] = '\0';
    return i;
}
