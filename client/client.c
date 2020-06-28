#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int getstring(char* buf, int length);

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

    while (1)
    {

        //    char msg[1024] = "Hello!";
        char msg[1024] = "\n";
        getstring(msg, sizeof(msg));
        send(sock, msg, sizeof(msg), 0);
        recv(sock, msg, sizeof(msg), 0);
        printf(msg);
        putchar('\n');
       // getchar();
    }
}

int getstring(char* string_buf, int length) {
    char c = 0;
    int i = 0;
    while((c = getchar()) != '\n' && length > i) {
        string_buf[i] = c;
        i++;
    }

}
