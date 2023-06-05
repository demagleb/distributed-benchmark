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

#include "worker_handler.h"
#include "constants.h"

int free_workers = 0;
int workers_overall = 0;


void accept_workers(struct Worker *workers, struct epoll_event *evt, int *epollfd) {
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
        workers[evt->data.fd].status = CONNECTED;
        free_workers++;
        workers_overall++;
        printf("WORKER CONNECTED\n");
        inet_ntop(AF_INET, &(worker_addr.sin_addr), workers[evt->data.fd].addr, BUF_SIZE);
        workers[evt->data.fd].port = ntohs(worker_addr.sin_port);
    }
}




void get_worker_params(struct Worker *workers, struct epoll_event *evt){
    read(evt->data.fd, (char *) &workers[evt->data.fd].worker_params, sizeof(struct WorkerParams));
    write(evt->data.fd, "OK\n", 3);
    printf("%s %d %d\n", workers[evt->data.fd].worker_params.CPU_brand,
           workers[evt->data.fd].worker_params.CPU_units,
           workers[evt->data.fd].worker_params.memory);
    workers[evt->data.fd].status = GOT_INFO;
}

void get_results(struct Worker *workers, struct epoll_event evt, int *client){
    if (read(evt.data.fd, (char *) &workers[evt.data.fd].time, sizeof(workers[evt.data.fd].time)) ==
        0) {
        workers_overall--;
        free_workers--;
        workers[evt.data.fd].status = 0;
        close(evt.data.fd);
    }
    workers[evt.data.fd].status = GOT_INFO;
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
            write(*client, output_buf, BUF_SIZE);
            workers[fd].time = 0;
        }
        close(*client);
    }
}

void delete_worker(struct Worker *workers, int fd) {
    struct WorkerParams no_params = {.memory = 0, .CPU_units = 0, .CPU_brand = {0}};
    struct Worker no_worker = {.port = 0, .addr = {0}, .status = NO_INFO, .bytes_num = 0, .worker_params = no_params, .timer_fd = 0, .time = 0};
    workers[fd] = no_worker;
}

void hire_workers(struct Worker *workers, char *content, size_t size) {
    for (int fd = 0; fd < MAX_FDS; fd++) {
        if (workers[fd].status == GOT_INFO) {
            workers[fd].bytes_num = write(fd, content, size);
            if (workers[fd].bytes_num == 0) {
                printf("Worker %s:%d disconnected\n", workers[fd].addr, workers[fd].port);
                delete_worker(workers, fd);
                close(fd);
            }
        } else {
            workers[fd].bytes_num = 0;
        }
    }
}


