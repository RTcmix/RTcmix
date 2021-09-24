CFLAGS = -Wall -O3
INCLUDES = -I../../src/rtcmix
LDFLAGS = -L../../lib -lrtcmix_embedded -s EXPORTED_RUNTIME_METHODS=["ccall"]

all: glue

glue: glue.c
	$(CC) $(CFLAGS) -o glue.js glue.c $(INCLUDES) $(LDFLAGS)

clean:
	$(RM) glue.js glue.wasm
