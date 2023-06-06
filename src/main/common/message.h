#include <stddef.h>
#include <sys/time.h>

enum {
    CPU_BRAND_SIZE = 128
};


typedef enum MessageType {
    WORKER_WORK, // Send to worker to start work
    MASTER_INFO,
    MASTER_RESULT, // Send to master to return result
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
