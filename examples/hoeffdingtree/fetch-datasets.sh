#!/bin/bash

#echo "Fetching airline_14col dataset..."
#wget http://agava.ijs.si/~blazs/qminer-datasets/airline_14col.names
#wget http://agava.ijs.si/~blazs/qminer-datasets/airline_14col.dat.gz

echo "Fetching SEA dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/sea.dat -O ./sandbox/ht/sea.dat

echo "Fetching titanic-4M dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/titanic-4M.dat.gz -O ./sandbox/ht/titanic-4M.dat.gz
echo "Now decompressing (ungzip) titanic-4M..."
gunzip -v ./sandbox/ht/titanic-4M.dat.gz 
echo "Removing archive..."
rm ./sandbox/ht/titanic-4M.dat.gz

echo "Fetching reg-cont toy dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/reg-cont.dat -O ./sandbox/ht/reg-cont.dat

echo "Fetching regression-test toy dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/regression-test.dat -O ./sandbox/ht/regression-test.dat

echo "Fetching wind dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/wind.dat -O ./sandbox/ht/wind.dat


echo "Fetching winequality dataset..."
wget http://agava.ijs.si/~blazs/qminer-datasets/winequality.dat -O ./sandbox/ht/winequality.dat

