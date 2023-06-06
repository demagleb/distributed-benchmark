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

void accept_client(struct epoll_event evt, int *epollfd, struct Client *client) {
    while (1) {
        int connection =
                accept4(evt.data.fd, NULL, NULL, SOCK_NONBLOCK);
        if (connection == -1) {
            break;
        }
        if (client->fd != -1) {
            char msg[] = "Server is busy. Try to connect later\n";
            write(connection, msg, strlen(msg));
            close(connection);
            continue;
        }
        printf("Got client\n");
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLET};
        evt1.data.fd = connection;
        epoll_ctl(*epollfd, EPOLL_CTL_ADD, connection, &evt1);
        client->fd = connection;
    }
}

void remove_client(struct Client *client) {
    struct FileInfo no_info = {.file_size = 0, .file_name = {0}};
    struct FileExe no_file = {.info = no_info, .content = NULL, .read_size = 0};
    free(client->file.content);
    close(client->fd);
    client->file = no_file;
    client->fd = -1;
}

void get_file_from_client(struct Client *client){
    int res = 0;
    if (client->file.info.file_size == 0) {
        if (read(client->fd, (void *)&client->file.info, sizeof(client->file.info)) < sizeof(client->file.info)) {
            printf("Cannot read filesize. The client is disconneted\n");
            remove_client(client);
            return;
        }
        client->file.read_size = 0;
    }
    if (client->file.content == NULL) {
        client->file.content = malloc(client->file.info.file_size + 2);
        client->file.content[client->file.info.file_size] = '\0';
        if (client->file.content == NULL) {
            printf("No enough memory for such big file.The client is disconneted\n");
            remove_client(client);
            return;
        }
    }
    size_t left = client->file.info.file_size - client->file.read_size;
    while (left > 0 && (res = read(client->fd, client->file.content + client->file.read_size, left)) >= 0) {
        if (res == 0) {
            printf("Client disconnected\n");
            remove_client(client);
            return;
        }
        left -= res;
        client->file.read_size += res;
    }
}