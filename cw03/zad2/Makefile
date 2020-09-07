CC = gcc -Wall

compile_matrix:
	$(CC) matrix.c -lm -o macierz

compile_matrix_generator:
	$(CC) -o matrix_generator matrix_generator.c
	mkdir matrixes

compile_matrix_test:
	$(CC) -o matrix_test matrix_test.c

test_all:
	make clean
	make compile_matrix_generator
	./matrix_generator 15 4 18
	make compile_matrix
	./macierz lista 10 100000 1
	make compile_matrix_test
	./matrix_test lista

clean:
	rm -f macierz matrix_generator lista matrix_test
	rm -rf matrixes