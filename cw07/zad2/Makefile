CC = gcc -Wall

all: test clean

compile:
	make main
	make odbierajacy
	make pakujacy
	make wysylajacy

main: main.c
	$(CC) -o main main.c -lpthread -lrt

odbierajacy: odbierajacy.c
	$(CC) -o odbierajacy odbierajacy.c -lpthread -lrt

pakujacy: pakujacy.c
	$(CC) -o pakujacy pakujacy.c -lpthread -lrt

wysylajacy: wysylajacy.c
	$(CC) -o wysylajacy wysylajacy.c -lpthread -lrt

clean:
	rm -f main odbierajacy wysylajacy pakujacy

test:
	make compile
	./main