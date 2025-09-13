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
	rm -f out/player_*
	ln player/out/player_blind out/player_blind
	ln player/out/player_naive out/player_naive
	ln player/out/player_greedy out/player_greedy
	ln player/out/player_greedy_box out/player_greedy_box
	ln player/out/player_greedy_weighted out/player_greedy_weighted
	ln player/out/player_sgreedy out/player_sgreedy
	ln player/out/player_sgreedy_box out/player_sgreedy_box
	ln player/out/player_sgreedy_weighted out/player_sgreedy_weighted

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
