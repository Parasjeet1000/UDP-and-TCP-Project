#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#define BUFSIZE 100

struct pdu {
    char type;
    char data[BUFSIZE];
};

int main(int argc, char *argv[]) {
    struct sockaddr_in fsin;
    struct pdu spdu;
    int sock;
    int alen;
    struct sockaddr_in sin;
    int s;
    int port = 3000;
    FILE *fp;
    struct stat st;
    int filesize, remaining;

    if (argc == 2) {
        port = atoi(argv[1]);
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        fprintf(stderr, "can't create socket\n");
    }

    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        fprintf(stderr, "can't bind to %d port\n", port);
    }

    alen = sizeof(fsin);
    while (1) {
        if (recvfrom(s, &spdu, sizeof(spdu), 0,
                     (struct sockaddr *)&fsin, &alen) < 0) {
            fprintf(stderr, "recvfrom error\n");
        }
        if (spdu.type == 'C') {
            fp = fopen(spdu.data, "r");
            if (fp == NULL) {
                spdu.type = 'E';
                sprintf(spdu.data, "Can't open file %s", spdu.data);
                sendto(s, &spdu, strlen(spdu.data)+1, 0,
                       (struct sockaddr *)&fsin, sizeof(fsin));
                continue;
            }
            stat(spdu.data, &st);
            filesize = st.st_size;
            remaining = filesize;
            while (remaining > 0) {
                int n = fread(spdu.data, 1, BUFSIZE, fp);
                remaining -= n;
                if (remaining <= 0) {
                    spdu.type = 'F';
                } else {
                    spdu.type = 'D';
                }
                sendto(s, &spdu, n+1, 0,
                       (struct sockaddr *)&fsin, sizeof(fsin));
            }
            fclose(fp);
        }
    }
}
