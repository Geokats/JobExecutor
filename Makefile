CC = gcc
OUT = jobExecutor worker
OBJ = jobExecutor.o

all: jobExecutor worker	

jobExecutor: jobExecutor.c
	$(CC) jobExecutor.c -o jobExecutor

worker: worker.c
	$(CC) worker.c -o worker

clean:
	rm $(OUT) $(OBJ)
