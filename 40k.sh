#!/bin/bash

for n in {1..1000}; 
do
    echo "=This string is fourty characters long.=" >> "mnt/hello.txt"
done

cat "mnt/hello.txt"
