#include "work.h"

#include <stdlib.h>
#include <linux/prctl.h>
#include <stdio.h>
#include <time.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>


double execFile(const char *filename) {

    clock_t start = clock();
    printf("Start at %ld\n", start);
    if (!fork()) {
        printf("set seccomp\n");
        // prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT);
        printf("exec %s\n", filename);
        execl("./loadedfile", "./loadedfile");
        printf("Can not exec\n");
        exit(1);
    }

    wait(NULL);
    printf("Died\n");
    clock_t finish = clock();
    printf("Finish at %ld\n", finish);


    return 1.L * (finish - start) / CLOCKS_PER_SEC; 
}
