#include "sysinfo.h"

#include <stdlib.h>
#include <linux/sysinfo.h>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <string.h>

void fillSystemInfo(MasterInfoMessage* msg) {
    msg->cpuCount = get_nprocs();
    struct sysinfo info;
    if (sysinfo(&info)) {
        perror("sysinfo");
        exit(1);
    }
    msg->messageType = MASTER_INFO;
    msg->ram = info.totalram;
    
    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    char* line = NULL;
    size_t size;
    size_t strlen = 0;
    while ((strlen = getline(&line, &size, cpuinfo)) != -1) {
        line[strlen - 1] = '\0';
        if (!strstr(line, "model name")) {
            free(line);
            line = NULL;
            continue;
        }
        char* ptr = strstr(line, ":");
        strcpy(msg->cpuBrand, ptr + 2);
        free(line);
        break;
    }
    return;
}