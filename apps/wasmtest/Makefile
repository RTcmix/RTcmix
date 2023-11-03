CFLAGS = -Wall -O3
INCLUDES = -I../../include
LDFLAGS = -L../../lib -lrtcmix_embedded -s EXPORTED_RUNTIME_METHODS=["ccall"] -sALLOW_MEMORY_GROWTH -sASSERTIONS

all: glue.js glue.wasm

glue.js glue.wasm: glue.c ../../lib/librtcmix_embedded.so
	$(CC) $(CFLAGS) -o glue.js $< $(INCLUDES) $(LDFLAGS)

clean:
	$(RM) glue.js glue.wasm
