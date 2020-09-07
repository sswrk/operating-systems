CC = gcc -Wall

compile:
	$(CC) -o main main.c

test:
	make compile
	./main child
	./main segfault
	./main status

clean:
	rm main