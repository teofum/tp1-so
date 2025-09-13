all: master master_demo player view

master: out docker out/chompchamps.a
	docker exec -it so-builder make all -C /root/master
	rm -f out/master
	ln master/out/master out/master

master_demo: out demo/master_demo
	rm -f out/master_demo
	ln demo/master_demo out/master_demo

player: out docker out/chompchamps.a
	docker exec -it so-builder make all -C /root/player
	rm -f out/p*
	ln player/out/pblind out/pblind
	ln player/out/pnaive out/pnaive
	ln player/out/pgreedy out/pgreedy
	ln player/out/pgreedy_box out/pgreedy_box
	ln player/out/pgreedy_w out/pgreedy_w
	ln player/out/psgreedy out/psgreedy
	ln player/out/psgreedy_box out/psgreedy_box
	ln player/out/psgreedy_w out/psgreedy_w
	ln player/out/pmixed_box out/pmixed_box

view: out docker out/chompchamps.a
	docker exec -it so-builder make all -C /root/view
	rm -f out/view
	ln view/out/view out/view

out/chompchamps.a: out docker
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

clean_shm:
	docker exec -it so-builder rm -f /dev/shm/game_state
	docker exec -it so-builder rm -f /dev/shm/game_sync

docker:
	docker start so-builder

docker_pull:
	docker pull agodio/itba-so-multi-platform:3.0
	docker run -d -v "${PWD}:/root" --security-opt seccomp:unconfined -it --name so-builder --platform=linux/amd64 --privileged agodio/itba-so-multi-platform:3.0
#	docker run -d -v "$(shell pwd):/root" --security-opt seccomp:unconfined -it --name so-builder --platform=linux/amd64 --privileged agodio/itba-so-multi-platform:3.0
#	tuve que usar este yo _(:v \)/_ (lolo)
	docker exec -it so-builder apt install libncurses-dev

.PHONY: master master_demo player view
