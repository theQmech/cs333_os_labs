all: client server

client: src/multi-client.c
	gcc src/multi-client.c -o client -pthread

server: src/server-mp.c
	gcc src/server-mp.c -o server

clean:
	rm -f client server

