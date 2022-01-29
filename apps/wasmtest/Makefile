CFLAGS = -Wall -O3
INCLUDES = -I../../include
LDFLAGS = -L../../lib -lrtcmix_embedded -s EXPORTED_RUNTIME_METHODS=["ccall"]

all: glue.js glue.wasm

glue.js glue.wasm: glue.c
	$(CC) $(CFLAGS) -o glue.js $< $(INCLUDES) $(LDFLAGS)

clean:
	$(RM) glue.js glue.wasm
