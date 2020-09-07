CC = gcc -Wall

compile:
	$(CC) -o ffind main.c

compile_nftw:
	$(CC) -o ffindnftw main_nftw.c
	
clean:
	rm -f ffind
	rm -f ffindnftw

test:
	mkdir d1
	mkdir d1/d2
	mkdir d1/d2/d3
	mkdir d1/d2/d3/d4
	touch d1/f1
	touch d1/d2/f2
	touch d1/d2/d3/f3
	touch d1/f12
	ln -s d1 l1
	ln -s d1/d2 l2
	make compile
	./ffind ../ -maxdepth 3 -mtime 0
	make clean_test

test_nftw:
	mkdir d1
	mkdir d1/d2
	mkdir d1/d2/d3
	mkdir d1/d2/d3/d4
	touch d1/f1
	touch d1/d2/f2
	touch d1/d2/d3/f3
	touch d1/f12
	ln -s d1 l1
	ln -s d1/d2 l2
	make compile_nftw
	./ffindnftw ../ -maxdepth 3 -mtime 0
	make clean_test

clean_test:
	make clean
	rm -f f1
	rm -f f2
	rm -f f3
	rm -f l1
	rm -f l2
	rm -r d1