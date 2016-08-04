all:
	gcc Client/multi-client.c -o client -w -pthread
	gcc Server/server-mp.c -o server -w

client:
	gcc Client/multi-client -o client -w -pthread

server:
	gcc Server/server-mp.c -o server -w

clean:
	rm -f client server
	rm *.txt
