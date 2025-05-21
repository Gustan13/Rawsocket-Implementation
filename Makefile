all: server client

server: server.o
	gcc server.o -o server

client: client.o
	gcc client.o -o client

server.o: server.c
	gcc -c server.c -o server.o

client.o: client.c
	gcc -c client.c -o client.o

clean:
	rm -f *.o
	rm -f server
	rm -f client
	rm -f newfile.jpg
