#include "connect.h"
#include "options.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cpuid.h>
#include <sys/sysinfo.h>

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
    int sock = makeConnection();

    // struct sysinfo info;
    // if (sysinfo(&info)) {
        // perror("sysinfo");
        // exit(1);
    // }
 
    // TODO: send cpu info 
    // int cpuCount = get_nprocs();
    // int totalRam = info.totalram;


    return sock;
}