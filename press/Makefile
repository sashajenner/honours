CC = c++
CFLAGS = -g -Wall -Werror
LDFLAGS = -lm -lz -lzstd -lpthread
OBJ = test.o press.o trans.o flat.o bitmap.o stats.o util.o
SVB = streamvbyte
SVBLIB = $(SVB)/libstreamvbyte.so.0.0.1
BZIP2 = bzip2
BZIP2LIB = $(BZIP2)/libbz2.a
FLZMA2 = fast-lzma2
FLZMA2LIB = $(FLZMA2)/libfast-lzma2.a
FLAC = flac-1.3.4
FLACLIB = $(FLAC)/src/libFLAC/.libs/libFLAC.so
TURBOPFOR = TurboPFor-Integer-Compression
TURBOPFORLIB = $(TURBOPFOR)/libic.a
FASTPFOR = FastPFor
FASTPFORLIB = $(FASTPFOR)/build/libFastPFOR.a
LIB = $(SVBLIB) $(BZIP2LIB) $(FLZMA2LIB) $(FLACLIB) $(TURBOPFORLIB) $(FASTPFORLIB)

.PHONY = clean $(LIB)

test: $(OBJ) $(LIB)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

$(SVBLIB):
	make -C $(SVB)

$(BZIP2LIB):
	make -C $(BZIP2)

$(FLZMA2LIB):
	make -C $(FLZMA2)

$(FLACLIB):
	make -C $(FLAC)

$(TURBOPFORLIB):
	make -C $(TURBOPFOR)

$(FASTPFORLIB):
	mkdir -p $(FASTPFOR)/build
	cmake $(FASTPFOR)
	cmake --build $(FASTPFOR)/build

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
	make -C $(BZIP2) clean
	make -C $(FLZMA2) clean
