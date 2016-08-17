all: gof gof-sig server shell

gof: src/get-one-file.c
	mkdir -p bin
	gcc src/get-one-file.c -o bin/get-one-file -w

gof-sig: src/get-one-file-sig.c
	mkdir -p bin
	gcc src/get-one-file-sig.c -o bin/get-one-file-sig -w

server: src/server-slow.c
	mkdir -p bin
	gcc src/server-slow.c -o bin/server-slow -pthread -w

shell: src/client-shell.c
	mkdir -p bin
	gcc src/client-shell.c -o bin/client-shell -pthread -w

clean:
	rm -rf bin/*

