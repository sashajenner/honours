CFLAGS=-Wall -Werror -I ../
OBJ=svbyte.o test.o ../bitmap.o

.PHONY=clean

test: $(OBJ)
	cc $(CFLAGS) $^ -o $@

svbyte.o: svbyte.c svbyte.h
	cc $(CFLAGS) -c $< -o $@

test.o: test.c test.h
	cc $(CFLAGS) -c $< -o $@

../bitmap.o: ../bitmap.c ../bitmap.h
	cc $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJ) test
