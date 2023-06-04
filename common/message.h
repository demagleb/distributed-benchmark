#include <stddef.h>
#include <sys/time.h>

typedef enum MessageType {
    WORKER_STATUS, // Send to worker to get status
    WORKER_WORK, // Send to worker to start work
    WORKER_STOP, // Send to worker to stop work
    WORKER_SHUTDOWN, // Send to worker to shutdown
    MASTER_RESULT, // Send to master to return result
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
