all:
	gcc Client/multiclient.c -o client
	gcc Server/server-mp.c -o server

client:
	gcc Client/multiclient -o client

server:
	gcc Server/server-mp.c -o server

clean:
	rm -f client server
	rm *.txt
