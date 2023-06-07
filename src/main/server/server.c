#define _GNU_SOURCE

#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#include "create_listener.h"

#include "constants.h"
#include "client_handler.h"
#include "worker_handler.h"


void info_about_no_workers(struct Client *client) {
    char no_workers[] = "No workers right now or task requires more than 10 sec\n";
    write(client->fd, no_workers, strlen(no_workers));
    printf("No workers right now\n");
    remove_client(client);
}


int main(int argc, char *argv[]) {
    for (int fd = 0; fd < MAX_FDS; ++fd) {
        delete_worker(fd);
    }
    struct Client client = {.file.content = NULL};
    remove_client(&client);
    signal(SIGPIPE, SIG_IGN);
    if (argc != 4) {
        fprintf(stderr, "Usage: %s SERVICE PORT_FOR_WORKERS PORT_FOR_CLIENTS\n", argv[0]);
        return 1;
    }
    printf("ADDRESS AND PORT FOR WORKERS: ");
    int sock = create_listener(argv[1], argv[2]);
    if (sock < 0) {
        return 1;
    }
    printf("ADDRESS AND PORT FOR CLIENTS: ");
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
            accept_workers(&evt, &epollfd);
        }
        if (evt.data.fd == client_sock) {
            accept_client(evt, &epollfd, &client);
        }
        if (evt.data.fd != sock && evt.data.fd != client_sock) {
            if (evt.data.fd == client.fd) {
                get_file_from_client(&client);
                if (client.file.info.file_size > 0 && client.file.info.file_size == client.file.read_size) {
                    printf("Got full file. Start to pass jobs to workers\n");
                    if (get_workers_overall() == 0) {
                        info_about_no_workers(&client);
                    } else {
                        hire_workers(client.file.content, client.file.info.file_size, epollfd);
                    }
                }
            } else {
                int worker_fd = 0;
                if ((worker_fd = get_worker_for_timer_fd(evt.data.fd)) != -1) {
                    struct Worker worker = get_worker(worker_fd);
                    printf("worker %s:%d disconnected because waiting time expired\n", worker.addr, worker.port);
                    delete_worker(worker_fd);
                    close(worker_fd);
                    if (get_workers_overall() == 0) {
                        info_about_no_workers(&client);
                    }
                }
                if (evt.events & EPOLLOUT) {
                    if (get_worker_status(evt.data.fd) == READY_FOR_TASK && client.file.info.file_size > 0 &&
                        client.file.info.file_size == client.file.read_size) {
                        continue_to_write_file_to_worker(evt.data.fd, client.file.content, client.file.info.file_size,
                                                         epollfd);
                    }
                }
                if (evt.events & EPOLLIN) {
                    if (get_worker_status(evt.data.fd) == CONNECTED) {
                        get_worker_params(&evt);
                    }
                    if (get_worker_status(evt.data.fd) == IS_WORKING) {
                        get_results(evt, &client, epollfd);
                    }
                }
            }

        }
    }
}

