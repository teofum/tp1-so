include ../Makefile.inc

OUTFILE=out/chompchamps.a
STATICLIBS=

SOURCES=$(wildcard src/*.c)
OBJECTS=$(foreach I,$(SOURCES:.c=.o),build/$(notdir $I))

all: $(OUTFILE)

$(OUTFILE): out build $(OBJECTS)
	$(AR) $(ARFLAGS) $(OUTFILE) $(OBJECTS) $(STATICLIBS)

build/%.o: src/%.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

out:
	mkdir out

build:
	mkdir build

clean:
	rm -rf build out

