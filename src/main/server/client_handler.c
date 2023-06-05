#define _GNU_SOURCE
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

#include "constants.h"
#include "client_handler.h"

void accept_client(struct epoll_event evt, int *epollfd, int *client) {
    while (1) {
        int connection =
                accept4(evt.data.fd, NULL, NULL, SOCK_NONBLOCK);
        if (connection == -1) {
            break;
        }
        if (*client != 0) {
            char msg[] = "Server is busy. Try to connect later\n";
            write(connection, msg, strlen(msg));
            continue;
        }
        printf("Got client\n");
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLET};
        evt1.data.fd = connection;
        epoll_ctl(*epollfd, EPOLL_CTL_ADD, connection, &evt1);
        *client = connection;
    }
}

void get_file_from_client(int *client, struct FILE_EXE *file){
    printf("getting file from client\n");
    int res = 0;
    if (file->size == 0) {
        if (read(*client, (void *)&file->size, sizeof(file->size)) < sizeof(file->size)) {
            printf("Cannot read filesize. The client is disconneted\n");
            close(*client);
            *client = 0;
            file->size = 0;
            return;
        }
        file->read_size = 0;
        printf("%lu\n", file->size);
    }
    if (file->filename[0] == '\0') {
        if (read(*client, file->filename, FILENAME_MAX + 1) < 0) {
            printf("Cannot read filename. The client is disconneted\n");
            close(*client);
            *client = 0;
            file->size = 0;
            return;
        }
    }
    if (file->content == NULL) {
        file->content = malloc(file->size + 1);
        file->content[file->size] = '\0';
        if (file->content == NULL) {
            printf("No enough memory for such big file.The client is disconneted\n");
            close(*client);
            *client = 0;
            file->size = 0;
            return;
        }
    }
    size_t left = file->size - file->read_size;
    while ((res = read(*client, file->content + file->read_size, left)) >= 0 && left > 0) {
        if (res == 0) {
            printf("disconnected client\n");
            close(*client);
            client = 0;
            free(file->content);
            file->size = 0;
            return;
        }
        left -= res;
        printf("%d\n", res);
        file->read_size += res;
    }
    printf("%d %lu\n", res, file->read_size);
}