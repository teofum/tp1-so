include ../Makefile.inc

OUTFILE=out/player
STATICLIBS=../out/chompchamps.a
INCLUDES=../lib/include

SOURCES=$(wildcard src/*.c)
OBJECTS=$(foreach I,$(SOURCES:.c=.o),build/$(notdir $I))

all: out/pblind  out/pnaive  out/pgreedy  out/pgreedy_box  out/pgreedy_w  out/psgreedy  out/psgreedy_box  out/psgreedy_w

# "Blind" strategy
out/pblind: GCCFLAGS += -DSTRAT_BLIND
out/pblind: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/pblind $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Naive" strategy
out/pnaive: GCCFLAGS += -DSTRAT_NAIVE
out/pnaive: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/pnaive $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + simple scoring
out/pgreedy: GCCFLAGS += -DSTRAT_GREEDY
out/pgreedy: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/pgreedy $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + box scoring
out/pgreedy_box: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_VALUE_BOX
out/pgreedy_box: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/pgreedy_box $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + weighted scoring
out/pgreedy_w: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_VALUE_WEIGHTED
out/pgreedy_w: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/pgreedy_w $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + simple scoring
out/psgreedy: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC
out/psgreedy: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/psgreedy $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + box scoring
out/psgreedy_box: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC -DGREEDY_VALUE_BOX
out/psgreedy_box: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/psgreedy_box $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + weighted scoring
out/psgreedy_w: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC -DGREEDY_VALUE_WEIGHTED
out/psgreedy_w: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c src/move.c -o build/move.o # recompile move
	$(GCC) -o out/psgreedy_w $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

build/%.o: src/%.c
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c $< -o $@

out:
	mkdir out

build:
	mkdir build

clean:
	rm -rf build out

