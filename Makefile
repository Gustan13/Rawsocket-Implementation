all: server client

server: server.o
	gcc server.o -o server

client: client.o
	gcc client.o -o client

server.o: server.c
	gcc server.c -c server.o

client.o: client.c
	gcc client.c -c client.o

clean:
	rm -f *.o
	rm -f server
	rm -f client
	rm -f newfile.jpg
