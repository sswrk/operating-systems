CC = gcc -Wall

compile:
	$(CC) main.c -o main
	$(CC) exec.c -o exec

clean:
	rm -rf main exec

test_fork:
	make clean
	make compile
	./main ignore fork
	./main pending fork
	./main mask fork
	./main handler fork

test_exec:
	make clean
	make compile
	./main ignore exec
	./main pending exec
	./main mask exec