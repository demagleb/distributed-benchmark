#pragma once

enum {
    NO_INFO = 0,
    CONNECTED = 1,
    READY_FOR_TASK = 2,
    IS_WORKING = 3,
    FINISHED_TASK = 4,
    MAX_FDS = 100,
    BUF_SIZE = 1024,
    SMALL_BUF_SIZE = 128,
    SECONDS_TO_WAIT = 10
};
