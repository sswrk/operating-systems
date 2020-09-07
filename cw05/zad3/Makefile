CC = gcc -Wall

main:
	$(CC) main.c -o main

producent:
	$(CC) producent.c -o producent

konsument:
	$(CC) konsument.c -o konsument

compile:
	make main
	make producent
	make konsument
	mkdir pliki
	touch pliki/wyniki

clean: 
	rm -f main
	rm -f producent
	rm -f konsument
	rm -f pipe
	rm -rf pliki

test:
	make compile
	./main