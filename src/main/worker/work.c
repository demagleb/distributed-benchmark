#include "work.h"

#include <bits/time.h>
#include <stdlib.h>
#include <linux/prctl.h>
#include <stdio.h>
#include <time.h>
#include <seccomp.h>
#include <linux/filter.h>
#include <linux/bpf.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>



void configureSeccomp() {
    prctl(PR_SET_NO_NEW_PRIVS, 1);
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);

    /*Just to run libc*/
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(pread64), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rseq), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getrandom), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_load(ctx);
}


double execFile(const char *filename) {
    struct timespec start, finish;
    clock_gettime(CLOCK_REALTIME, &start);
    if (!fork()) {
        configureSeccomp();
        puts("start");
        execl("./loadedfile", "./loadedfile");
        exit(1);
    }

    wait(NULL);
    clock_gettime(CLOCK_REALTIME, &finish);
    return (finish.tv_sec - start.tv_sec) + 1e-9 * (finish.tv_nsec - start.tv_nsec); 
}
