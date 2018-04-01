#ifndef IPC_H
#define IPC_H

int getlineIPC(char **buffer, size_t *bufferSize, int fd);
int writelineIPC(int fd, char *str);

#endif
