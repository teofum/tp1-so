#!/bin/bash

docker exec -it -w /root/out so-builder ./master "$@"
