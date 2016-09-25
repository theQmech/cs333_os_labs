all: client server

client: multi-client.cpp
	g++ multi-client.cpp -o multi-client -pthread

server: server-mt.cpp
	g++ server-mt.cpp -o server-mt -pthread

clean:
	rm -f multi-client server-mt

