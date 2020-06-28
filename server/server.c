#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>

int main(int argc, char **argv)
{
    int client_sock, listener_sock;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read = 0;

    listener_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_sock < 0)
    {
        perror("Error create listener socket.");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(5500);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(listener_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind error.");
        exit(2);
    }

    listen(listener_sock, 10);

    while (1)
    {
        client_sock = accept(listener_sock, NULL, NULL);
        if (client_sock < 0)
        {
            perror("Accept error.");
            exit(3);
        }
        while (1)
        {
            bytes_read = recv(client_sock, buf, 1024, 0);
            if (bytes_read <= 0)
                break;
            printf("%s\n", buf);
            send(client_sock, buf, bytes_read, 0);
        }
        close(client_sock);
    }
    close(listener_sock);
    return 0;
}
