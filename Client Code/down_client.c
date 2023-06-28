/* A simple TCP file download client */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#define SERVER_TCP_PORT 3000 /* well-known port */
#define BUFLEN 256 /* buffer length */

int main(int argc, char **argv)
{
    int n, i, bytes_to_read;
    int sd, port;
    struct hostent *hp;
    struct sockaddr_in server;
    char *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];
    int fd, filesize, remain_data;

    switch(argc){
        case 2:
            host = argv[1];
            port = SERVER_TCP_PORT;
            break;
        case 3:
            host = argv[1];
            port = atoi(argv[2]);
            break;
        default:
            fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
            exit(1);
    }

    /* Create a stream socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (hp = gethostbyname(host))
        bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
        fprintf(stderr, "Can't get server's address\n");
        exit(1);
    }

    /* Connecting to the server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
        fprintf(stderr, "Can't connect \n");
        exit(1);
    }

    printf("Enter file name to download: ");
    scanf("%s", sbuf);
    write(sd, sbuf, strlen(sbuf) + 1); /* send filename to server */

    /* Wait for server response */
    read(sd, rbuf, BUFLEN);
    if (strcmp(rbuf, "FILE_NOT_FOUND") == 0) {
        printf("File not found on server.\n");
        close(sd);
        exit(1);
    }

    /* Open local file for writing */
    fd = open(sbuf, O_CREAT | O_EXCL | O_WRONLY, 0666);
    if (fd == -1) {
        printf("File already exists.\n");
        close(sd);
        exit(1);
    }

    /* Read file size from server */
    read(sd, &filesize, sizeof(int));
    //printf("File size: %d bytes.\n", filesize);

    /* Download file */
    remain_data = filesize;
    while (remain_data > 0) {
        n = read(sd, rbuf, BUFLEN);
        write(fd, rbuf, n);
        remain_data -= n;
    }
    printf("Download complete.\n");

    close(fd);
    close(sd);
    return(0);
}
