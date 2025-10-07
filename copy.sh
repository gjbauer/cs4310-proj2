#!/bin/bash

echo "hello!" >> "mnt/hello.txt"

mkdir "mnt/dir"

ln "mnt/hello.txt" "mnt/ln.txt"

rm "mnt/hello.txt"

cat "mnt/ln.txt"

cp "mnt/ln.txt" "mnt/dir/hello.txt"

cat "mnt/dir/hello.txt"
