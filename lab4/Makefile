all: client server

client: multi-client.c
	gcc multi-client.c -o multi-client -pthread

server: server-mt.cpp
	g++ server-mt.cpp -o server-mt -pthread

clean:
	rm -f multi-client server-mt

