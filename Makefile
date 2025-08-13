all: master player view

master: out docker
	docker exec -it so-builder make all -C /root/master
	ln master/out/master out/master

player: out docker
	docker exec -it so-builder make all -C /root/player
	ln player/out/player out/player

view: out docker
	docker exec -it so-builder make all -C /root/view
	ln view/out/view out/view

out:
	mkdir out

clean:
	cd master; make clean
	cd player; make clean
	cd view; make clean
	rm -rf out

docker:
	docker start so-builder

docker_pull:
	docker pull agodio/itba-so-multi-platform:3.0
	docker run -d -v "${PWD}:/root" --security-opt seccomp:unconfined -it --name so-builder --platform=linux/amd64 --privileged agodio/itba-so-multi-platform:3.0

.PHONY: master player view
