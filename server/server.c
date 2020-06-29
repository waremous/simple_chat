#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define DEBUG(dbgmsg) printf(dbgmsg)

int main(int argc, char **argv)
{
    int client_sock, listener_sock;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read = 0;
    //epoll listener vars
    struct epoll_event listener_ev, listener_wait_ev;
    int listener_epollfd;
    //epoll clients vars
    struct epoll_event client_ev, clients_wait_events[100];
    int clients_epollfd;

    listener_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_sock < 0)
    {
        perror("Error create listener socket.");
        exit(EXIT_FAILURE);
    }
    // Set nonblock listener socket.
    fcntl(listener_sock, F_SETFL, O_NONBLOCK);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5500);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind error.");
        exit(EXIT_FAILURE);
    }

    listen(listener_sock, 10);

    //epoll start init listener
    listener_epollfd = epoll_create(1);
    if (listener_epollfd == -1)
    {
        perror("epoll_create failure.");
        exit(EXIT_FAILURE);
    }
    listener_ev.events = EPOLLIN;
    listener_ev.data.fd = listener_sock;
    if (epoll_ctl(listener_epollfd, EPOLL_CTL_ADD, listener_sock, &listener_ev) == -1)
    {
        perror("epoll_ctl error.");
        exit(EXIT_FAILURE);
    }

    //epoll start init clients
    clients_epollfd = epoll_create(100);
    if (clients_epollfd == -1)
    {
        perror("Failure create clients epoll_create()");
        exit(EXIT_FAILURE);
    }

    //main for() vars
    int ep_ret = 0, clients_count = 0, clients_socket_list[100];
    for (int i = sizeof(clients_socket_list) - 1; i >= 0; i--)
    {
        clients_socket_list[i] = 0;
    }
    //main for()
    for (;;)
    {
        //wait new client socket
        ep_ret = epoll_wait(listener_epollfd, &listener_wait_ev, 1, 1000);
        if (ep_ret == -1)
        {
            perror("Wait listeners error.");
            exit(EXIT_FAILURE);
        }

        //add new client socket
        if (ep_ret > 0 && clients_count < sizeof(clients_socket_list) - 1)
        {
            client_sock = accept(listener_sock, NULL, NULL);
            if (client_sock < 0)
            {
                perror("Accept error.");
                exit(EXIT_FAILURE);
            }

            if (clients_socket_list[clients_count] == 0)
            {
                clients_socket_list[clients_count] = client_sock;
                clients_count++;
            }
            else
            {
                for (int i = 0; i < sizeof(clients_socket_list); i++)
                {
                    if (clients_socket_list[i] == 0)
                    {
                        clients_socket_list[i] = client_sock;
                        clients_count++;
                        break;
                    }
                }
            }

            client_ev.events = EPOLLIN;
            client_ev.data.fd = client_sock;
            if (epoll_ctl(clients_epollfd, EPOLL_CTL_ADD, client_sock, &client_ev) == -1)
            {
                perror("epoll_ctl add client socket error.");
                exit(EXIT_FAILURE);
            }
            DEBUG("New client.\n");
        }
        else if (clients_count >= sizeof(clients_socket_list) - 1)
        {
            client_sock = accept(listener_sock, NULL, NULL);
            close(client_sock);
            perror("Limit clients.");
        }

        //receive and send messages
        for (;;)
        {
            ep_ret = epoll_wait(clients_epollfd, clients_wait_events, 1, 1000);
            if (ep_ret == -1)
            {
                perror("epoll clients wait error.");
                exit(EXIT_FAILURE);
            }
            else if (ep_ret == 0) // If no messages from clients
            {
                break;
            }

            bytes_read = recv(clients_wait_events[0].data.fd, buf, sizeof(buf), 0);
            if (bytes_read == -1 || bytes_read == 0)
            {
                break;
            }
            buf[bytes_read] = '\0';

            /*
            if (bytes_read > 0 && bytes_read < 2)
            {
                buf[0] = '\n';
                buf[1] = '\0';
            }
            */
            DEBUG("point_001\n");
            if (bytes_read > 0)
            {
                printf("%s\n", buf);
                for (int i = 0; i < clients_count; i++)
                {
                    if (clients_socket_list[i] == clients_wait_events[0].data.fd) //no send message for sender
                        continue;
                    if (send(clients_socket_list[i], buf, bytes_read, 0) == -1) //send msg, if error del client from clients_socket_list and epoll
                    {
                        client_ev.events = EPOLLIN;
                        client_ev.data.fd = clients_socket_list[i];
                        if (epoll_ctl(clients_epollfd, EPOLL_CTL_DEL, clients_socket_list[i], &client_ev) == -1)
                        {
                            perror("epoll_ctl add client socket error.");
                            exit(EXIT_FAILURE);
                        }
                        close(clients_socket_list[i]);
                        clients_socket_list[i] = 0;

                        DEBUG("Delete client.\n");
                    }
                }
            }
        }

        //close(client_sock);
    }
    close(listener_sock);
    return 0;
}
