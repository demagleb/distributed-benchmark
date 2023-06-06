#pragma once

enum {
    MAX_FDS = 100,
    BUF_SIZE = 1024,
    SMALL_BUF_SIZE = 200,
    NO_INFO = 0,
    CONNECTED = 1,
    GOT_INFO = 2,
    IS_WORKING = 3,
    CLIENT = 1,
    WORKER = 2
};
