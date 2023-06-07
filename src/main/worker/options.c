#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Options opts = {NULL, NULL, -1};

void printHelp(int argc, const char** argv) {
    printf("%s <master ipv4> <master port>\n", argv[0]);
}

void parseOptions(int argc, const char **argv) {
    if (argc != 3) {
        printHelp(argc, argv);
        exit(1);
    }
    opts.master_ip = argv[1];
    opts.master_port = argv[2];
    return;
}

void genFilename() {
    pid_t pid = getpid();
    sprintf(options()->filename, "./exec%d", pid);
    return;
}   

Options* options() {
    return &opts;
}
