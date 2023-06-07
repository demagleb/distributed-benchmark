#define _GNU_SOURCE

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>

#include "worker_handler.h"
#include "constants.h"
#include "../common/message.h"

int workers_finished = 0;
int workers_overall = 0;
struct Worker workers[MAX_FDS];
int timer_fds[MAX_FDS];


void accept_workers(struct epoll_event *evt, int *epollfd) {
    while (1) {
        struct sockaddr_in worker_addr;
        unsigned len = sizeof(worker_addr);
        int connection =
                accept4(evt->data.fd, (struct sockaddr *) &worker_addr, &len, SOCK_NONBLOCK);
        if (connection == -1) {
            break;
        }
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLOUT | EPOLLET};
        evt1.data.fd = connection;
        epoll_ctl(*epollfd, EPOLL_CTL_ADD, connection, &evt1);
        workers[connection].status = CONNECTED;
        printf("Worker connected\n");
        inet_ntop(AF_INET, &(worker_addr.sin_addr), workers[connection].addr, BUF_SIZE);
        workers[connection].port = ntohs(worker_addr.sin_port);
    }
}


void get_worker_params(struct epoll_event *evt) {
    struct MasterInfoMessage msg_info;
    read(evt->data.fd, (char *) &msg_info, sizeof(struct MasterInfoMessage));
    // write(evt->data.fd, "OK\n", 3);
    strcpy(workers[evt->data.fd].worker_params.CPU_brand, msg_info.cpuBrand);
    workers[evt->data.fd].worker_params.CPU_units = msg_info.cpuCount;
    workers[evt->data.fd].worker_params.memory = msg_info.ram;
    printf("%s %lu %lu\n", workers[evt->data.fd].worker_params.CPU_brand,
           workers[evt->data.fd].worker_params.CPU_units,
           workers[evt->data.fd].worker_params.memory);
    workers_overall++;
    workers[evt->data.fd].status = READY_FOR_TASK;
}

void get_results(struct epoll_event evt, struct Client *client, int epollfd) {
    struct MasterResultMessage msg_res;
    if (read(evt.data.fd, (char *) &msg_res, sizeof(msg_res)) ==
        0) {
        printf("Worker %s:%d disconnected\n", workers[evt.data.fd].addr, workers[evt.data.fd].port);
        delete_worker(evt.data.fd);
        close(evt.data.fd);
        return;
    }
    workers[evt.data.fd].time = msg_res.sec;
    workers[evt.data.fd].status = FINISHED_TASK;
    workers[evt.data.fd].bytes_num = 0;
    workers_finished++;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, workers[evt.data.fd].timer_fd, NULL);
    timer_fds[evt.data.fd] = -1;
    if (workers_finished == workers_overall) {
        for (int fd = 0; fd < MAX_FDS; fd++) {
            if (workers[fd].status == FINISHED_TASK) {
                char output_buf[BUF_SIZE] = {0};
                snprintf(output_buf, BUF_SIZE, "%s:%d\n"
                                               "CPU brand: %s\n"
                                               "CPU units: %lu\n"
                                               "Memory: %lu bytes\n"
                                               "File: %s\n"
                                               "Time: %.10g sec\n\n",
                         workers[fd].addr,
                         workers[fd].port,
                         workers[fd].worker_params.CPU_brand,
                         workers[fd].worker_params.CPU_units,
                         workers[fd].worker_params.memory,
                         client->file.info.file_name,
                         workers[fd].time);
                write(client->fd, output_buf, BUF_SIZE);
                workers[fd].time = 0;
                workers[fd].status = READY_FOR_TASK;
            }
        }
        workers_finished = 0;
        remove_client(client);
    }
}

void delete_worker(int fd) {
    struct WorkerParams no_params = {.memory = 0, .CPU_units = 0, .CPU_brand = {0}};
    struct Worker no_worker = {.port = 0, .addr = {
            0}, .status = NO_INFO, .bytes_num = 0, .worker_params = no_params, .timer_fd = -1, .time = 0};
    if (workers[fd].timer_fd != -1) {
        close(workers[fd].timer_fd);
    }
    workers[fd] = no_worker;
    if (workers_overall > 0) {
        workers_overall--;
    }
    timer_fds[fd] = -1;
}

void hire_workers(char *content, size_t size, int epollfd) {
    if (content == NULL) {
        return;
    }
    for (int fd = 0; fd < MAX_FDS; fd++) {
        if (workers[fd].status == READY_FOR_TASK) {
            struct WorkerWorkMessage msg_work = {0};
            msg_work.messageType = WORKER_WORK;
            msg_work.fileSize = size;
            write(fd, (char *) &msg_work, sizeof(msg_work));
            continue_to_write_file_to_worker(fd, content, size, epollfd);
        }
    }
}

void continue_to_write_file_to_worker(int fd, char *content, size_t size, int epollfd) {
    int res = 0;
    if (content == NULL) {
        return;
    }
    size_t left = size - workers[fd].bytes_num;
    while (left > 0 && (res = write(fd, content + workers[fd].bytes_num, left)) >= 0) {
        if (res == 0) {
            printf("Worker %s:%d disconnected\n", workers[fd].addr, workers[fd].port);
            delete_worker(fd);
            close(fd);
        }
        left -= res;
        workers[fd].bytes_num += res;
    }
    if (workers[fd].bytes_num == size) {
        printf("worker started job\n");
        workers[fd].timer_fd = timerfd_create(CLOCK_REALTIME, 0);
        timer_fds[workers[fd].timer_fd] = fd;
        struct itimerspec wait_to_disconnect = {.it_value.tv_sec = SECONDS_TO_WAIT, .it_value.tv_nsec = 0, .it_interval.tv_nsec = 0, .it_interval.tv_sec = 0};
        struct epoll_event evt1 = {.events = EPOLLIN | EPOLLET};
        evt1.data.fd = workers[fd].timer_fd;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, workers[fd].timer_fd, &evt1);
        timerfd_settime(workers[fd].timer_fd, 0, &wait_to_disconnect, NULL);
        workers[fd].status = IS_WORKING;
    }
}

int get_worker_status(int fd) {
    return workers[fd].status;
}

int get_worker_for_timer_fd(int timer_fd) {
    return timer_fds[timer_fd];
}

struct Worker get_worker(int fd) {
    return workers[fd];
}

int get_workers_overall() {
    return workers_overall;
}





