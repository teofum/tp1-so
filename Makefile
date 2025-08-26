all: master player view

master: out docker lib
	docker exec -it so-builder make all -C /root/master
	rm -f out/master
	ln master/out/master out/master

player: out docker lib
	docker exec -it so-builder make all -C /root/player
	rm -f out/player
	ln player/out/player out/player

view: out docker lib
	docker exec -it so-builder make all -C /root/view
	rm -f out/view
	ln view/out/view out/view

lib: out docker
	docker exec -it so-builder make all -C /root/lib
	rm -f out/chompchamps.a
	ln lib/out/chompchamps.a out/chompchamps.a

out:
	mkdir out

clean:
	cd master; make clean
	cd player; make clean
	cd view; make clean
	cd lib; make clean
	rm -rf out

docker:
	docker start so-builder

docker_pull:
	docker pull agodio/itba-so-multi-platform:3.0
	docker run -d -v "${PWD}:/root" --security-opt seccomp:unconfined -it --name so-builder --platform=linux/amd64 --privileged agodio/itba-so-multi-platform:3.0
#	docker run -d -v "$(shell pwd):/root" --security-opt seccomp:unconfined -it --name so-builder --platform=linux/amd64 --privileged agodio/itba-so-multi-platform:3.0
#	tuve que usar este yo _(:v \)/_ (lolo)
	docker exec -it so-builder apt install libncurses-dev

.PHONY: master player view
