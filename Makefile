CC = gcc
OUT = jobExecutor worker
OBJ = jobExecutor.o

all: jobExecutor worker
	rm *.o

jobExecutor: jobExecutor.c ipc
	$(CC) -c jobExecutor.c
	$(CC) jobExecutor.o ipc.o -o jobExecutor

worker: worker.c ipc
	$(CC) -c worker.c
	$(CC) worker.o ipc.o -o worker

ipc: ipc.c ipc.h
	$(CC) -c ipc.c

clean:
	rm $(OUT) $(OBJ)
