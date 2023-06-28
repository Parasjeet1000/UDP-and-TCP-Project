#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>

#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256 /* buffer length */

int file_transfer(int);

int main(int argc, char **argv)
{
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;
    switch(argc){
        case 1:
            port = SERVER_TCP_PORT;
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }
    /* Create a stream socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }
    /* Bind an address to the socket */
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }
    /* queue up to 5 connect requests */
    listen(sd, 5);
    (void) signal(SIGCHLD, SIG_IGN); // ignore child signals
    while(1) {
        client_len = sizeof(client);
        new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
        if(new_sd < 0){
            fprintf(stderr, "Can't accept client \n");
            exit(1);
        }
        switch (fork()){
            case 0: /* child */
                (void) close(sd);
                exit(file_transfer(new_sd));
            default: /* parent */
                (void) close(new_sd);
                break;
            case -1:
                fprintf(stderr, "fork: error\n");
        }
    }
}

/* file transfer program */
int file_transfer(int sd)
{
    char buf[BUFLEN];
    int n, file_fd;
    
    // receive the file name from the client
    n = read(sd, buf, BUFLEN);
    if (n < 0) {
        fprintf(stderr, "Can't read filename from client\n");
        return -1;
    }
    buf[n] = '\0';

    // open the file in binary mode
    file_fd = open(buf, O_RDONLY);
    if (file_fd < 0) {
        fprintf(stderr, "Can't open file %s\n", buf);
        return -1;
    }

    // read file contents and send them to the client
    while ((n = read(file_fd, buf, BUFLEN)) > 0) {
        if (write(sd, buf, n) != n) {
            fprintf(stderr, "Can't send file %s\n", buf);
            return -1;
        }
    }

    // close the file and the socket
    close(file_fd);
    close(sd);
    return 0;
}

