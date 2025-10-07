#!/bin/bash

mkdir mnt/nested

for n in {1..50}; 
do
    echo "$n" > "mnt/nested/"$n".nums"
done

for n in {1..5}; 
do
    cat "mnt/nested/"$((n*10))".nums"
done
