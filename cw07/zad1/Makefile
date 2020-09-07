CC = gcc -Wall

all: test clean

compile:
	make main
	make odbierajacy
	make pakujacy
	make wysylajacy

main: main.c
	$(CC) -o main main.c wspolny.c

odbierajacy: odbierajacy.c
	$(CC) -o odbierajacy odbierajacy.c wspolny.c

pakujacy: pakujacy.c
	$(CC) -o pakujacy pakujacy.c wspolny.c

wysylajacy: wysylajacy.c
	$(CC) -o wysylajacy wysylajacy.c wspolny.c

clean:
	rm -f main odbierajacy wysylajacy pakujacy

test:
	make compile
	./main