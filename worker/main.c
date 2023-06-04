#include "net.h"
#include "options.h"
#include "work.h"
#include "../common/message.h"
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/signal.h>
#include <stdio.h>

void pipeHandler(int) {
    exit(0);
}

int main(int argc, const char** argv) {
    parseOptions(argc, argv);

    // int sock = connectToMaster();

    const char* filename = "./loadedfile";
    
    WorkerWorkMessage workMessage;
    MasterResultMessage resultMessage;
    resultMessage.messageType = MASTER_RESULT;
    while (1) {
        // read(sock, &workMessage, sizeof(workMessage));
        // readFile(sock, filename, workMessage.fileSize);
        resultMessage.sec = execFile(filename);
        printf("%.10g\n", resultMessage.sec);
        break;
        // write(sock, &resultMessage, sizeof(resultMessage));
    }
    return 0;
}