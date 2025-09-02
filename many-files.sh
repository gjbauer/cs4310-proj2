#!/bin/bash

mkdir mnt/nested

for n in {1..50}; 
do
    touch "mnt/nested/"$n".nums"
done
