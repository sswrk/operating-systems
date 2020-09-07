CC = gcc
FLAGS = -Wall

compile: klient.c serwer.c
	$(CC) $(FLAGS) -o klient klient.c -lpthread -lm
	$(CC) $(FLAGS) -o serwer serwer.c -lpthread -lm

clean:
	rm -f klient
	rm -f serwer