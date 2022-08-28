#!/bin/bash
./nachos -f
./nachos -cp test/huge huge
./nachos -D
./nachos -ap test/huge huge
./nachos -D
./nachos -ap test/huge huge
./nachos -D