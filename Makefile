.PHONY: all clean

all: client server

client: client.c
	gcc -std=gnu99 -Wall -Wextra -O1 client.c -o client

server: server.c
	gcc -std=gnu99 -Wall -Wextra -O1 server.c -o server

clean:
	rm -f client server
