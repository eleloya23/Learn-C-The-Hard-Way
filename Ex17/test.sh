#!/bin/bash
set -e

echo "Create database db.dat"
./ex17 db.dat c

echo "Set 1 foo foo@bar.com"
./ex17 db.dat s 1 foo foo@bar.com

echo "Set 2 baz baz@qux.com"
./ex17 db.dat s 2 baz baz@qux.com

echo "Printing all records"
./ex17 db.dat l

echo "Deleting record 1"
./ex17 db.dat d 1

echo "Getting record 2"
./ex17 db.dat g 2

