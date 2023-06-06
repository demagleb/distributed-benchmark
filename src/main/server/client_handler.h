struct FILE_EXE{
    char * content;
    size_t size;
    char filename[FILENAME_MAX + 1];
    size_t read_size;
};

void accept_client(struct epoll_event evt, int *epollfd, int *client);
void get_file_from_client(int *client, struct FILE_EXE *file);
