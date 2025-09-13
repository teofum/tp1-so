include ../Makefile.inc

OUTFILE=out/player
STATICLIBS=../out/chompchamps.a
INCLUDES=../lib/include

SOURCES=$(wildcard src/*.c)
OBJECTS=$(foreach I,$(SOURCES:.c=.o),build/$(notdir $I))

all: out/player_blind  out/player_naive  out/player_greedy  out/player_greedy_box  out/player_greedy_weighted  out/player_sgreedy  out/player_sgreedy_box  out/player_sgreedy_weighted

# "Blind" strategy
out/player_blind: GCCFLAGS += -DSTRAT_BLIND
out/player_blind: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_blind $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Naive" strategy
out/player_naive: GCCFLAGS += -DSTRAT_NAIVE
out/player_naive: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_naive $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + simple scoring
out/player_greedy: GCCFLAGS += -DSTRAT_GREEDY
out/player_greedy: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_greedy $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + box scoring
out/player_greedy_box: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_VALUE_BOX
out/player_greedy_box: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_greedy_box $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Greedy" strategy + weighted scoring
out/player_greedy_weighted: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_VALUE_WEIGHTED
out/player_greedy_weighted: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_greedy_weighted $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + simple scoring
out/player_sgreedy: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC
out/player_sgreedy: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_sgreedy $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + box scoring
out/player_sgreedy_box: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC -DGREEDY_VALUE_BOX
out/player_sgreedy_box: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_sgreedy_box $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

# "Stochastic Greedy" strategy + weighted scoring
out/player_sgreedy_weighted: GCCFLAGS += -DSTRAT_GREEDY -DGREEDY_STOCHASTIC -DGREEDY_VALUE_WEIGHTED
out/player_sgreedy_weighted: out build $(OBJECTS) $(STATICLIBS)
	$(GCC) -o out/player_sgreedy_weighted $(OBJECTS) $(STATICLIBS) $(LDFLAGS) 

build/%.o: src/%.c
	$(GCC) $(GCCFLAGS) -I./include $(foreach x,$(INCLUDES),-I$x) -c $< -o $@

out:
	mkdir out

build:
	mkdir build

clean:
	rm -rf build out

