#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFSIZE 100

struct pdu {
    char type;
    char data[BUFSIZE];
};

int main(int argc, char **argv) {
    char *host = "localhost";
    int port = 3000;
    struct hostent *phe;
    struct sockaddr_in sin;
    int s, n;
    struct pdu spdu;
    FILE *fp;

    if (argc == 2) {
        host = argv[1];
    } else if (argc == 3) {
        host = argv[1];
        port = atoi(argv[2]);
    } else {
        fprintf(stderr, "usage: UDPfile [host [port]]\n");
        exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if ((phe = gethostbyname(host))) {
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    } else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        fprintf(stderr, "Can't get host entry \n");
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        fprintf(stderr, "Can't create socket \n");
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        fprintf(stderr, "Can't connect to %s %s \n", host, "File");
    }

    while (1) {
        printf("Enter filename or Q to quit: ");
        scanf("%s", spdu.data);
        if (strcmp(spdu.data, "Q") == 0) {
            break;
        }
        spdu.type = 'C';
        write(s, &spdu, strlen(spdu.data)+1);

        fp = fopen(spdu.data, "w");
        if (fp == NULL) {
            fprintf(stderr, "Can't open file %s\n", spdu.data);
            continue;
        }

        while (1) {
            n = read(s, &spdu, sizeof(spdu));
            if (n < 0) {
                fprintf(stderr, "Read failed\n");
                break;
            }
            if (spdu.type == 'F' || spdu.type == 'D') {
                fwrite(spdu.data, 1, n-1, fp);
                if (spdu.type == 'F') {
                    break;
                }
            } else if (spdu.type == 'E') {
                fprintf(stderr, "Error: %s\n", spdu.data);
                break;
            }
        }
        fclose(fp);
    }

    exit(0);
}
