#!/bin/bash

clear
cd ..

cd pflayer && make clean && make && cd ..
cd amlayer && make clean && make && cd ..
cd dblayer && make clean && make 

./interface

make clean
cd .. && cd amlayer && make clean
cd .. && cd pflayer && make clean
cd ../dblayer