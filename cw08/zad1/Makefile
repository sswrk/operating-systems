CC = gcc -Wall

compile:
	$(CC) -o main main.c -lpthread -lm

clean:
	rm -f main pomiary.txt
	rm -rf wyniki

test:
	make clean
	make compile
	mkdir wyniki
	./main 1 sign lena.ascii.pgm wyniki/sign1.txt
	./main 1 block lena.ascii.pgm wyniki/block1.txt
	./main 1 interleaved lena.ascii.pgm wyniki/interleaved1.txt

	./main 2 sign lena.ascii.pgm wyniki/sign2.txt
	./main 2 block lena.ascii.pgm wyniki/block2.txt
	./main 2 interleaved lena.ascii.pgm wyniki/interleaved2.txt

	./main 4 sign lena.ascii.pgm wyniki/sign4.txt
	./main 4 block lena.ascii.pgm wyniki/block4.txt
	./main 4 interleaved lena.ascii.pgm wyniki/interleaved4.txt

	./main 8 sign lena.ascii.pgm wyniki/sign8.txt
	./main 8 block lena.ascii.pgm wyniki/block8.txt
	./main 8 interleaved lena.ascii.pgm wyniki/interleaved8.txt