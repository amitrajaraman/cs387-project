#!bin/bash

clear

cd ../amlayer && make && cd ../dblayer
cd ../pflayer && make && cd ../dblayer
make clean && make

./interface