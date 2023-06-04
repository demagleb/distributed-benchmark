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


int client = 0;

struct WorkerParams {
    char CPU_brand[SMALL_BUF_SIZE];
    int CPU_units;
    int memory;
};


struct Worker {
    char addr[SMALL_BUF_SIZE];
    int port;
    int time;
    struct WorkerParams worker_params;
};

int free_workers = 0;
int workers_overall = 0;
struct Worker workers[MAX_FDS];


struct GeneralInfo {
    int type;
    int status;
};

struct GeneralInfo info[MAX_FDS];


void accept_workers(struct epoll_event *evt, int *epollfd) {
    while (1) {
        struct sockaddr_in worker_addr;
        unsigned len = sizeof(worker_addr);
        int connection =
                accept4(evt->data.fd, (struct sockaddr *) &worker_addr, &len, SOCK_NONBLOCK);
        if (connection == -1) {
            break;
        }
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLET};
        evt1.data.fd = connection;
        epoll_ctl(*epollfd, EPOLL_CTL_ADD, connection, &evt1);
        info[connection].type = WORKER;
        info[connection].status = CONNECTED;
        free_workers++;
        workers_overall++;
        printf("WORKER CONNECTED\n");
        inet_ntop(AF_INET, &(worker_addr.sin_addr), workers[evt->data.fd].addr, BUF_SIZE);
        workers[evt->data.fd].port = ntohs(worker_addr.sin_port);
    }
}


int accept_client(struct epoll_event *evt, int *epollfd) {
    while (1) {
        int connection =
                accept4(evt->data.fd, NULL, NULL, SOCK_NONBLOCK);
        if (connection == -1) {
            break;
        }
        if (client != 0) {
            char msg[] = "Server is busy. Try to connect later\n";
            write(connection, msg, strlen(msg));
            break;
        }
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLET};
        evt1.data.fd = connection;
        epoll_ctl(*epollfd, EPOLL_CTL_ADD, connection, &evt1);
        info[connection].type = CLIENT;
        client = evt->data.fd;
    }
}


int main(int argc, char *argv[]) {
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
    char buf[BUF_SIZE] = {0};
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
            accept_workers(&evt, &epollfd);
        }
        if (evt.data.fd == client_sock) {
            accept_client(&evt, &epollfd);
        }
        if (evt.data.fd != sock && evt.data.fd != client_sock) {
            ssize_t res;
            if (info[evt.data.fd].type == CLIENT) {
                while ((res = read(evt.data.fd, buf, sizeof(buf) - 1)) >= 0) {
                    if (res == 0) {
                        fprintf(stderr, "disconnected %d fd\n", evt.data.fd);
                        close(evt.data.fd);
                    }
                    for (int fd = 0; fd < MAX_FDS; fd++) {
                        if (info[fd].type == WORKER) {
                            write(fd, buf, res);
                        }
                    }
                }
                for (int fd = 0; fd < MAX_FDS; fd++) {
                    if (info[fd].type == WORKER) {
                        free_workers--;
                        info[fd].status = IS_WORKING;
                    }
                }
            } else {
                if (info[evt.data.fd].status == CONNECTED) {
                    read(evt.data.fd, (char *) &workers[evt.data.fd].worker_params, sizeof(struct WorkerParams));
                    write(evt.data.fd, "OK\n", 3);
                    printf("%s %d %d\n", workers[evt.data.fd].worker_params.CPU_brand,
                           workers[evt.data.fd].worker_params.CPU_units,
                           workers[evt.data.fd].worker_params.memory);
                    info[evt.data.fd].status = GOT_INFO;
                }
                if (info[evt.data.fd].status == IS_WORKING) {
                    if (read(evt.data.fd, (char *) &workers[evt.data.fd].time, sizeof(workers[evt.data.fd].time)) ==
                        0) {
                        workers_overall--;
                        free_workers--;
                        info[evt.data.fd].type = 0;
                        info[evt.data.fd].status = 0;
                        close(evt.data.fd);
                    }
                    info[evt.data.fd].status = GOT_INFO;
                    free_workers++;
                    if (free_workers == workers_overall) {
                        for (int fd = 0; fd < MAX_FDS; fd++) {
                            char output_buf[BUF_SIZE] = {0};
                            snprintf(output_buf, BUF_SIZE, "%s:%d\n"
                                                           "CPU brand: %s\n"
                                                           "CPU units: %d\n"
                                                           "Memory: %d Mb\n"
                                                           "Time: %d nanosec\n",
                                     workers[fd].addr,
                                     workers[fd].port,
                                     workers[fd].worker_params.CPU_brand,
                                     workers[fd].worker_params.CPU_units,
                                     workers[fd].worker_params.memory,
                                     workers[fd].time);
                            write(client, output_buf, BUF_SIZE);
                            workers[fd].time = 0;
                        }
                        close(client);
                    }
                }
            }

        }
    }
}

