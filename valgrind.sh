#!/bin/bash

rm -f out/valgrind.*.log
docker exec -it -w /root/out so-builder valgrind --log-file=valgrind.%p.log --trace-children=yes --track-origins=yes --leak-check=full ./master -p ./pwall ./pnaive ./pgreedy ./pgreedy_box ./pgreedy_w ./psgreedy ./psgreedy_box ./psgreedy_w ./pmixed_box -d 50 -v ./view -w 20 -h 15 -s 1000
