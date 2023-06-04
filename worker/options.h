typedef struct Options {
    const char* master_ip;
    const char* master_port;
} Options;

void parseOptions(int argc, const char** argv);
Options* options();