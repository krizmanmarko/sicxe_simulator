#!/bin/bash

cd src; make binary; cd -
./build/binary $1
cd src; make clean; cd -
