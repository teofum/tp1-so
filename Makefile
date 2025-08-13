all: master player view

master: out
	cd master; make all
	ln master/out/master out/master

player: out
	cd player; make all
	ln player/out/player out/player

view: out
	cd view; make all
	ln view/out/view out/view

out:
	mkdir out

clean:
	cd master; make clean
	cd player; make clean
	cd view; make clean
	rm -rf out

.PHONY: master player view
