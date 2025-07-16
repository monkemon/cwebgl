#define _GNU_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

#define PORT 9999
#define SMALL_BUFF_SIZE 128
#define OUT_BUFF_SIZE 256
#define BIG_BUFF_SIZE 2048
#define READ_CHUNK_SIZE 512

void check_errno(const char* msg)
{
    if (errno != 0)
    {
        perror(msg);
        exit(1);
    }
}

void log_msg(const char* msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

static const char* hex_alphabet = "0123456789ABCDEF";
char* hexdump_int64_cdqe(char* buff, long* n) 
{
    char* b = (char*) n;
    char* p = buff;
    
    for (int i = 7; i > 0; i--) 
    {
        *p   = hex_alphabet[ b[i] >> 4  ]; // high nibble
        p[1] = hex_alphabet[ b[i] & 0xF ]; // low nibble
        p[2] = 0x20;                    // space
        p   += 3;
    }

    // last byte without space, null terminate
    *p   = hex_alphabet[*b >> 4];
    p[1] = hex_alphabet[*b & 0xF];
    p[2] = 0;

    return buff;
}


int main() 
{
    int successfull_requests = 0;

    errno = 0;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    check_errno("Failed to create socket\n");

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    check_errno("Failed to create socket\n");

    bind(sock, (struct sockaddr *)&sin, sizeof(sin));
    check_errno("Failed to bind\n");

    listen(sock, 0);
    check_errno("Failed to listen\n");

    {
        log_msg("Server starting...\n");

        char buff[SMALL_BUFF_SIZE];
        memset(buff, 0, SMALL_BUFF_SIZE);

        char out_buff[OUT_BUFF_SIZE];
        memset(out_buff, 0, OUT_BUFF_SIZE);

        struct sockaddr_in tmpaddr;
        socklen_t len = sizeof(tmpaddr);

        getpeername( sock, (struct sockaddr*)&tmpaddr, &len);
        sprintf(out_buff,"SERVER-PEER: %s:%d\n", 
            inet_ntop( AF_INET, &tmpaddr.sin_addr, buff, sizeof(buff)),
            ntohs( tmpaddr.sin_port )
        );
        log_msg(out_buff);
        memset(buff, 0, SMALL_BUFF_SIZE);
        memset(out_buff, 0, OUT_BUFF_SIZE);

        getsockname( sock, (struct sockaddr*)&tmpaddr, &len);
        sprintf(out_buff, "SERVER-SOCK: %s:%d\n",
            inet_ntop( AF_INET, &tmpaddr.sin_addr, buff, sizeof(buff)),
            ntohs( tmpaddr.sin_port )
        );
        log_msg(out_buff);
        memset(buff, 0, SMALL_BUFF_SIZE);
        memset(out_buff, 0, OUT_BUFF_SIZE);
    }


    for(;;)
    {
        int client_sock = accept( sock, NULL, 0 );
        log_msg("Client connected\n");

        char buf[BIG_BUFF_SIZE];
        memset(buf, 0, BIG_BUFF_SIZE);

        char tmpbuff[SMALL_BUFF_SIZE];
        memset(tmpbuff, 0, SMALL_BUFF_SIZE);

        char out_buff[OUT_BUFF_SIZE];
        memset(out_buff, 0, OUT_BUFF_SIZE);

        struct sockaddr_in tmpaddr;
        socklen_t len = sizeof(tmpaddr);

        getpeername( client_sock, (struct sockaddr*)&tmpaddr, &len);
        sprintf(out_buff,"peer: %s:%d\n", 
            inet_ntop( AF_INET, &tmpaddr.sin_addr, tmpbuff, sizeof(tmpbuff)),
            ntohs( tmpaddr.sin_port )
        );
        log_msg(out_buff);
        memset(tmpbuff, 0, SMALL_BUFF_SIZE);
        memset(out_buff, 0, OUT_BUFF_SIZE);

        getsockname( client_sock, (struct sockaddr*)&tmpaddr, &len);
        sprintf(out_buff, "sock: %s:%d\n",
            inet_ntop( AF_INET, &tmpaddr.sin_addr, tmpbuff, sizeof(tmpbuff)),
            ntohs( tmpaddr.sin_port )
        );
        log_msg(out_buff);
        memset(tmpbuff, 0, SMALL_BUFF_SIZE);
        memset(out_buff, 0, OUT_BUFF_SIZE);

        for (;;)
        {
            char mbuff[8 + 1];
            size_t read_bytes = read(client_sock, mbuff, 8);
            mbuff[8] = 0;

            strcat(buf, mbuff);

            if (read_bytes < 0)
                check_errno("Error while reading from socket\n");

            if (read_bytes == 0)
                break;

            if (read_bytes < 8)
                break;

            memset(mbuff, 0, 8);
            memset(tmpbuff, 0, SMALL_BUFF_SIZE);
        }

        memset(tmpbuff, 0, sizeof(tmpbuff));
        sprintf(tmpbuff, "\nMessage length: [%ld]", strlen(buf));
        log_msg(tmpbuff);

        log_msg("\nClient message:\n\n");
        log_msg(buf);
        log_msg("\n");

        const char* http_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 14\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello, World!\n";

        write(client_sock, http_response, strlen(http_response));

        close(client_sock);
        log_msg("Closing client socket\n");

        successfull_requests++;        

        char tmpb [128];
        sprintf(tmpb, "~~~~~~~~~~~~~~~~~~~~~~~ %d\n", successfull_requests);
        log_msg(tmpb);
    }

    close(sock);
    log_msg("Closing main socket\n");
    return 0;
}
