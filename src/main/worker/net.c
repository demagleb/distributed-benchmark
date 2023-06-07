#include "net.h"
#include "options.h"
#include "sysinfo.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <cpuid.h>
#include <sys/sysinfo.h>
#include <unistd.h>

enum {
    MTU = 4 << 10
};

int makeConnection() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(1);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (!inet_aton(options()->master_ip, &addr.sin_addr)) {
        fprintf(stderr, "Incorrect ipv4 address: '%s'\n", options()->master_ip);
        exit(1);
    }
    char* end;
    long port = strtol(options()->master_port, &end, 10);
    if (*end != '\0' || !(0 <= port && port <= 1 << 16)) {
        fprintf(stderr, "Incorrect tcp port: '%s'\n", options()->master_port);
        exit(1);
    }
    addr.sin_port = htons(port);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("connect");
        exit(1);
    }
    return sock;
}

int connectToMaster() {
    MasterInfoMessage info;
    fillSystemInfo(&info);
    int sock = makeConnection();
    write(sock, &info, sizeof(info));
    return sock;
}

void readFile(int socket, const char *filename, size_t filesize) {
    char buf[MTU];
    size_t allGot = 0;
    FILE* file = fopen(filename, "w") ;
    while (allGot != filesize) {
        int got = read(socket, buf, (MTU < filesize - allGot ? MTU : filesize - allGot));
        fwrite(buf, 1, got, file);
        allGot += got;
    }
    fclose(file);
    chmod(filename, 0755);
    return;
}
