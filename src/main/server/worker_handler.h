#pragma once
#include "constants.h"
#include "client_handler.h"


struct WorkerParams {
    char CPU_brand[SMALL_BUF_SIZE];
    size_t CPU_units;
    size_t memory;
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




void accept_workers(struct epoll_event *evt, int *epollfd);
void get_worker_params(struct epoll_event *evt);
void get_results(struct epoll_event evt, struct Client *client);
void delete_worker(int fd);
int hire_workers(char *content, size_t size) ;
void continue_to_write_file_to_worker(int fd, char *content, size_t size) ;
int get_worker_status(int fd);