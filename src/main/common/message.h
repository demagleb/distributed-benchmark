#include <stddef.h>
#include <sys/time.h>

enum { MESSAGE_MAX_SIZE = 128 };

typedef enum MessageType {
    WORKER_STATUS, // Send to worker.c to get status
    WORKER_WORK, // Send to worker.c to start work
    WORKER_STOP, // Send to worker.c to stop work
    WORKER_SHUTDOWN, // Send to worker.c to shutdown
    MASTER_RESULT, // Send to master to return result
    CLIENT_CONNECTED, // Send to client to notify that it is connected
    CLIENT_CONNECTION_ERROR, // Send to client to notify that it is not connected
} MessageType;

typedef enum WorkerStatus {
    WORK,
    WAIT
} WorkerStatus;


typedef struct WorkerStatusMessage {
    MessageType messageType; // = WORKER_STATUS
    WorkerStatus workerStatus;    
} WorkerStatusMessage;

typedef struct WorkerWorkMessage {
    MessageType messageType; // = WORKER_WORK
    size_t fileSize;
} WorkerWorkMessage;
// After this struct send file


typedef struct WorkerStopMessage {
    MessageType messageType; // = WORKER_STOP
} WorkerStopMessage;

typedef WorkerStopMessage WorkerShutdownMessage; // messageType = WORKER_SHUTDOWN

typedef struct MasterResultMessage {
    MessageType messageType; // = MASTER_RESULT
    double sec; 
} MasterResultMessage;

typedef struct ServerAnswer {
    MessageType messageType;
    char message[MESSAGE_MAX_SIZE];
} ServerAnswer;
