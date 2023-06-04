#include "connect.h"
#include "options.h"
#include <unistd.h>



int main(int argc, const char** argv) {
    parseOptions(argc, argv);

    int sock = connectToMaster();
    write(sock, "parasha\n", 8);
    return 0;
}