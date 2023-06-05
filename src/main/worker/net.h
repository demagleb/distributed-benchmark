#include <stddef.h>

int connectToMaster();

void readFile(int socket, const char* filename, size_t filesize);
