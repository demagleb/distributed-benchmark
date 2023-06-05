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

#include "create_listener.h"

#include "constants.h"
#include "client_handler.h"
#include "worker_handler.h"







int main(int argc, char *argv[]) {
    struct Worker workers[MAX_FDS];
    struct Client client = {.file.content = NULL};
    remove_client(&client);
    signal(SIGPIPE, SIG_IGN);
    if (argc != 4) {
        fprintf(stderr, "Usage: %s SERVICE\n", argv[0]);
        return 1;
    }
    int sock = create_listener(argv[1], argv[2]);
    if (sock < 0) {
        return 1;
    }
    int client_sock = create_listener(argv[1], argv[3]);
    if (client_sock < 0) {
        return 1;
    }
    int epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll");
        return 1;
    }
    struct epoll_event evt = {.events = EPOLLIN | EPOLLET};
    evt.data.fd = sock;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &evt);
    evt.data.fd = client_sock;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sock, &evt);
    while (1) {
        int timeout = -1;
        errno = 0;
        if (epoll_wait(epollfd, &evt, 1, timeout) < 1) {
            if (errno == EINTR) {
                continue;
            }
            return 1;
        }
        if (evt.data.fd == sock) {
            accept_workers(workers, &evt, &epollfd);
        }
        if (evt.data.fd == client_sock) {
            accept_client(evt, &epollfd, &client);
        }
        if (evt.data.fd != sock && evt.data.fd != client_sock) {
            if (evt.data.fd == client.fd) {
                get_file_from_client(&client);
                if (client.file.info.file_size > 0 && client.file.info.file_size == client.file.read_size) {
                    printf("Got full file. Start to pass jobs to workers\n");
                    hire_workers(workers, client.file.content, client.file.info.file_size);
                }
            } else {
                if (evt.events & EPOLLOUT) {
                    if (workers[evt.data.fd].status == READY_FOR_TASK) {
                        continue_to_write_file_to_worker(workers, evt.data.fd, client.file.content, client.file.info.file_size);
                    }
                } else {
                    if (workers[evt.data.fd].status == CONNECTED) {
                        get_worker_params(workers, &evt);
                    }
                    if (workers[evt.data.fd].status == IS_WORKING) {
                        get_results(workers, evt, &client);
                    }
                }
            }

        }
    }
}

