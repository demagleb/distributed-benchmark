#pragma once
#include "constants.h"
#include "client_handler.h"


struct WorkerParams {
    char CPU_brand[SMALL_BUF_SIZE];
    int CPU_units;
    int memory;
};


struct Worker {
    char addr[SMALL_BUF_SIZE];
    int port;
    int time;
    int status;
    int timer_fd;
    int bytes_num;
    struct WorkerParams worker_params;
};




void accept_workers(struct Worker *workers, struct epoll_event *evt, int *epollfd);
void get_worker_params(struct Worker *workers, struct epoll_event *evt);
void get_results(struct Worker *workers, struct epoll_event evt, struct Client *client);
void delete_worker(struct Worker *workers, int fd);
void hire_workers(struct Worker *workers, char *content, size_t size) ;
void continue_to_write_file_to_worker(struct Worker *workers, int fd, char *content, size_t size) ;