all:
	gcc Client/multi-client.c -o client
	gcc Server/server-mp.c -o server

client:
	gcc Client/multi-client -o client

server:
	gcc Server/server-mp.c -o server

clean:
	rm -f client server
	rm *.txt
