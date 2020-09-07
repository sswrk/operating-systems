CC = gcc -Wall -std=c11

all: clean compile

compile:
	$(CC) -o program main.c
	
clean:
	rm -f program wyniki.txt dane*

test:
	make

	for length in 1 4 512 1024 4096 8192 ; do \
		for stringNumber in 4096 8192; do \
			./program generate dane $$stringNumber $$length ;\
			./program copy dane dane$$stringNumber.$$length.lib $$stringNumber $$length lib ;\
			./program copy dane dane$$stringNumber.$$length.sys $$stringNumber $$length sys ;\
			./program sort dane$$stringNumber.$$length.lib $$stringNumber $$length lib ;\
			./program sort dane$$stringNumber.$$length.sys $$stringNumber $$length sys ;\
		done; \
	done

	rm -rf dane*

tmp:
	make
	./program generate dane 10 30