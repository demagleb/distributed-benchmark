#pragma once
#include "../common/fileinfo.h"

struct FileExe{
    struct FileInfo info;
    char * content;
    size_t read_size;
};

struct Client {
    int fd;
    struct FileExe file;
};

void accept_client(struct epoll_event evt, int *epollfd, struct Client *client);
void remove_client(struct Client *client);
void get_file_from_client(struct Client *client);
