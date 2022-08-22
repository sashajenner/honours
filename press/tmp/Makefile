CC = cc
CFLAGS = -g -Wall -Werror
LDFLAGS = -lm -lz -lzstd
OBJ = test.o press.o trans.o flat.o bitmap.o stats.o util.o
SVB = streamvbyte
SVBLIB = $(SVB)/libstreamvbyte.so.0.0.1

.PHONY = clean

test: $(OBJ) $(SVBLIB)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(SVBLIB):
	make -C $(SVB)

test.o: test.c test.h press.h bitmap.h
	$(CC) $< $(CFLAGS) -c -o $@

press.o: press.c press.h bitmap.h stats.h trans.h util.h flat.h
	$(CC) $< $(CFLAGS) -c -o $@

trans.o: trans.c trans.h
	$(CC) $< $(CFLAGS) -c -o $@

flat.o: flat.c flat.h stats.h util.h press.h bitmap.h
	$(CC) $< $(CFLAGS) -c -o $@

bitmap.o: bitmap.c bitmap.h
	$(CC) $< $(CFLAGS) -c -o $@

stats.o: stats.c stats.h
	$(CC) $< $(CFLAGS) -c -o $@

util.o: util.c util.h
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm $(OBJ) test
	make -C $(SVB) clean
