#!/bin/bash
./nachos -f
./nachos -cp test/big big
./nachos -D
./nachos -ap test/small big
./nachos -D
./nachos -hap test/small big
./nachos -D