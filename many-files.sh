#!/bin/bash

mkdir mnt/numbers

for n in {1..50}; 
do
    echo "$n" > "mnt/numbers/"$n".nums"
done

for n in {1..5}; 
do
    cat "mnt/numbers/"$((n*10))".nums"
done

ls mnt/numbers | wc -l
