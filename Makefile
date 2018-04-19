CC = gcc
OUT = jobExecutor worker
EX_OBJ = jobExecutor.o ipc.o
W_OBJ = worker.o ipc.o trie.o postingList.o textIndex.o
OBJ = jobExecutor.o worker.o ipc.o trie.o postingList.o textIndex.o

all: jobExecutor worker

jobExecutor: $(EX_OBJ)
	$(CC) $(EX_OBJ) -o jobExecutor

worker: $(W_OBJ)
	$(CC) $(W_OBJ) -o worker

jobExecutor.o: jobExecutor.c
	$(CC) -c jobExecutor.c

worker.o: worker.c
	$(CC) -c worker.c

ipc.o: ipc.c ipc.h
	$(CC) -c ipc.c

trie.o: trie.c trie.h
	$(CC) -c trie.c

postingList.o: postingList.c postingList.h
	$(CC) -c postingList.c

textIndex.o: textIndex.c textIndex.h
	$(CC) -c textIndex.c

clean:
	rm $(OUT) $(OBJ)
