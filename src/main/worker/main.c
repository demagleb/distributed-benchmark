#include "net.h"
#include "options.h"
#include "work.h"
#include "../common/message.h"
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/signal.h>
#include <stdio.h>


#include <time.h>

void pipeHandler(int) {
    exit(0);
}

int main(int argc, const char** argv) {
    parseOptions(argc, argv);
    genFilename();
    int sock = connectToMaster();
    
    WorkerWorkMessage workMessage;
    MasterResultMessage resultMessage;
    resultMessage.messageType = MASTER_RESULT;
    while (1) {
        printf("Read file started\n");
        read(sock, &workMessage, sizeof(workMessage));
        readFile(sock, options()->filename, workMessage.fileSize);
        printf("Read file finished\n");
        resultMessage.sec = execFile(options()->filename);
        printf("%.10g\n", resultMessage.sec);
        write(sock, &resultMessage, sizeof(resultMessage));
    }
    return 0;
}
