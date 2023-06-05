#pragma once

enum {
    MAX_FDS = 100,
    BUF_SIZE = 1024,
    SMALL_BUF_SIZE = 200,
    NO_INFO = 0,
    CONNECTED,
    READY_FOR_TASK,
    IS_WORKING,
    FINISHED_TASK
};
