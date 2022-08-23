CC = cc
CFLAGS = -g -Wall -Werror
OBJ = decode.o decode_scalar.o decode_x64.o encode.o encode_scalar.o encode_x64.o

.PHONY = all clean

all: $(OBJ)

decode.o: decode.c svb16.h simd_detect_x64.h
	$(CC) $< $(CFLAGS) -c -o $@

decode_scalar.o: decode_scalar.c svb16.h
	$(CC) $< $(CFLAGS) -c -o $@

decode_x64.o: decode_x64.c intrinsics.h common.h shuffle_tables.h svb16.h
	$(CC) $< $(CFLAGS) -c -o $@

encode.o: encode.c svb16.h
	$(CC) $< $(CFLAGS) -c -o $@

encode_scalar.o: encode_scalar.c svb16.h
	$(CC) $< $(CFLAGS) -c -o $@

encode_x64.o: encode_x64.c intrinsics.h common.h shuffle_tables.h svb16.h
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm $(OBJ)
