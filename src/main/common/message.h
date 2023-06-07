#include <stddef.h>
#include <sys/time.h>

enum { CPU_BRAND_SIZE = 128, MESSAGE_MAX_SIZE = 128 };

typedef enum MessageType {
    WORKER_WORK, // Send to worker to start work
    MASTER_INFO,
    MASTER_RESULT, // Send to master to return result
    CLIENT_CONNECTED, // Send to client to notify that it is connected
    CLIENT_CONNECTION_ERROR, // Send to client to notify that it is not connected
} MessageType;

typedef struct WorkerWorkMessage {
    MessageType messageType; // = WORKER_WORK
    size_t fileSize;
} WorkerWorkMessage;
// After this struct send file

typedef struct MasterInfoMessage {
    MessageType messageType;
    size_t ram;
    size_t cpuCount;
    char cpuBrand[CPU_BRAND_SIZE];
} MasterInfoMessage;

typedef struct MasterResultMessage {
    MessageType messageType; // = MASTER_RESULT
    double sec;
} MasterResultMessage;

typedef struct ServerAnswer {
    MessageType messageType;
    char message[MESSAGE_MAX_SIZE];
} ServerAnswer;
