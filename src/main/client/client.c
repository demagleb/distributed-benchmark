#include "create_connection.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../common/fileinfo.h"

enum { BUFFER_SIZE = 1024, MAX_ATTEMPTS = 100 };

int send_fileinfo(int socket, char *file_name, size_t file_size) {
    struct FileInfo file_info;
    file_info.file_size = file_size;
    memset(file_info.file_name, 0, sizeof(file_info.file_name));
    strcpy(file_info.file_name, file_name);

    size_t bytes_sent = 0;
    while (bytes_sent < sizeof(file_info)) {
        ssize_t res = write(socket, ((void *) &file_info) + bytes_sent, sizeof(file_info) - bytes_sent);
        if (res == -1) {
            perror("Error sending file info");
            return -1;
        }
        bytes_sent += res;
    }

    return 0;
}

int send_file(int socket, char *file_name, size_t file_size) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    char buffer[BUFFER_SIZE];

    size_t all_bytes_read = 0;

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        size_t pointer = 0;
        size_t attempts = 0;
        while (++attempts < MAX_ATTEMPTS) {
            ssize_t bytes_wrote = write(socket, buffer + pointer, bytes_read - pointer);
            if (bytes_wrote == -1) {
                perror("Error writing to socket");
                return -1;
            }
            pointer += bytes_wrote;
            if (pointer == bytes_read) {
                break;
            }
        }
        if (attempts >= MAX_ATTEMPTS) {
            perror("Too many attempts");
            return -1;
        }
        all_bytes_read += bytes_read;
    }
    if (bytes_read == -1 || all_bytes_read != file_size) {
        perror("Error reading from file");
        return -1;
    }

    close(fd);

    return 0;
}

int get_results(int socket) {
    char buffer[BUFFER_SIZE];

    while (1) {
        ssize_t bytes_read = read(socket, buffer, sizeof(buffer));
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read < 0) {
            perror("Error reading from socket");
            return -1;
        }
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    fflush(stdout);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s FILENAME NODE SERVICE\n", argv[0]);
        return 1;
    }
    int sock = create_connection(argv[2], argv[3]);
    if (sock < 0) {
        return 1;
    }

    char *file_name = argv[1];

    struct stat st;
    if (stat(file_name, &st) == -1) {
        perror("Error getting file size");
        return 1;
    }
    size_t file_size = st.st_size;

    if (send_fileinfo(sock, file_name, file_size) == -1) {
        close(sock);
        return 1;
    }

    if (send_file(sock, file_name, file_size) == -1) {
        close(sock);
        return 1;
    }

    if (get_results(sock) == -1) {
        close(sock);
        return 1;
    }

    close(sock);
}