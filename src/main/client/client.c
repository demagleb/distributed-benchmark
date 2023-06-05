#include "create_connection.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

enum { BUFFER_SIZE = 1024 };

int send_fileinfo(int socket, char *file_name, size_t file_size) {
    if (write(socket, &file_size, sizeof(file_size)) != sizeof(file_size)) {
        perror("Error sending file size");
        return -1;
    }

    char buffer[FILENAME_MAX + 1];
    strcpy(buffer, file_name);
    buffer[strlen(file_name)] = 0;

    if (write(socket, buffer, strlen(file_name) + 1) != strlen(file_name) + 1) {
        perror("Error sending file name");
        return -1;
    }

    return 0;
}

int send_file(int socket, char *file_name) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(socket, buffer, bytes_read) == -1) {
            perror("Error writing to socket");
            return -1;
        }
    }
    if (bytes_read == -1) {
        perror("Error reading from file");
        return -1;
    }

    close(fd);

    return 0;
}

int get_results(int socket) {
    char buffer[BUFFER_SIZE];

    ssize_t bytes_read = read(socket, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading from socket");
        return -1;
    }
    buffer[bytes_read] = '\0';
    printf("%s", buffer);

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

    if (send_file(sock, file_name) == -1) {
        close(sock);
        return 1;
    }

    if (get_results(sock) == -1) {
        close(sock);
        return 1;
    }

    close(sock);
}