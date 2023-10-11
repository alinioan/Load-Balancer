CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -g
LOAD=load_balancer
SERVER=server
HASHTABLE=Hashtable

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(HASHTABLE).o
	$(CC) $^ -o tema2

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(HASHTABLE).o: $(HASHTABLE).c $(HASHTABLE).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
