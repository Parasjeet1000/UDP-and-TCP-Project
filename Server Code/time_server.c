#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE 1024

int main(int argc, char argv[]) {
struct sockaddr_in fsin; / the from address of a client /
char buf[BUFSIZE]; / input buffer */
char filename[BUFSIZE];
char pts;
int sock; / server socket /
int file_fd;
int len, bytes_sent;
time_t now; / current time /
int alen; / from-address length /
struct sockaddr_in sin; / an Internet endpoint address /
int s, type; / socket descriptor and socket type */
int port = 3000;
switch(argc) {
    case 1:
        break;
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
}

memset(&sin, 0, sizeof(sin));
sin.sin_family = AF_INET;
sin.sin_addr.s_addr = INADDR_ANY;
sin.sin_port = htons(port);

/* Allocate a socket */
s = socket(AF_INET, SOCK_DGRAM, 0);
if (s < 0) {
    fprintf(stderr, "can't creat socket\n");
    exit(1);
}

/* Bind the socket */
if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    fprintf(stderr, "can't bind to %d port\n", port);
    exit(1);
}

alen = sizeof(fsin);
while (1) {
    if ((len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen)) < 0) {
        fprintf(stderr, "recvfrom error\n");
        exit(1);
    }

    if (strcmp(buf, "download") == 0) {
        /* prompt the client for filename */
        if ((len = recvfrom(s, buf, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen)) < 0) {
            fprintf(stderr, "recvfrom error\n");
            exit(1);
        }

        /* open the file for reading */
        if ((file_fd = open(buf, O_RDONLY)) < 0) {
            fprintf(stderr, "can't open file %s\n", buf);
            exit(1);
        }

        /* send the file to the client */
        while ((len = read(file_fd, buf, BUFSIZE)) > 0) {
            if ((bytes_sent = sendto(s, buf, len, 0, (struct sockaddr *)&fsin, sizeof(fsin))) < 0) {
                fprintf(stderr, "sendto error\n");
                exit(1);
            }
        }

        close(file_fd);
    }
}
}
