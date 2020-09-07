CC=gcc -Wall

compile: klient.c serwer.c
	$(CC) -o klient klient.c -lpthread -lm
	$(CC) -o serwer serwer.c -lpthread -lm

server: serwer
	./serwer 6666 socket

client_net: klient
	./klient "aaa" INET 127.0.0.1 6666

client_local: klient
	./klient "bbb" LOCAL socket

clean:
	rm -f klient serwer socket