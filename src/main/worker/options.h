typedef struct Options {
    const char* master_ip;
    const char* master_port;
    char filename[100];
} Options;

void parseOptions(int argc, const char** argv);

void genFilename();
Options* options();
