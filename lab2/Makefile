all: client server

client: src/multi-client.c
	gcc src/multi-client.c -o multi-client -pthread

server: src/server-mp.c
	gcc src/server-mp.c -o server-mp -pthread

clean:
	rm -f multi-client server-mp

