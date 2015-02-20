#!/bin/bash

SANDBOX=./sandbox/ht
WEB=http://agava.ijs.si/~blazs/qminer-datasets

#echo "Fetching airline_14col dataset..."
#wget $WEB/airline_14col.names
#wget $WEB/airline_14col.dat.gz

echo "Fetching SEA dataset..."
wget $WEB/sea.dat -O $SANDBOX/sea.dat

echo "Fetching titanic-4M dataset..."
wget $WEB/titanic-4M.dat.gz -O $SANDBOX/titanic-4M.dat.gz
echo "Now decompressing (ungzip) titanic-4M..."
gunzip -v $SANDBOX/titanic-4M.dat.gz 
echo "Removing archive..."
rm $SANDBOX/titanic-4M.dat.gz

echo "Fetching reg-cont toy dataset..."
wget $WEB/reg-cont.dat -O $SANDBOX/reg-cont.dat

echo "Fetching regression-test toy dataset..."
wget $WEB/regression-test.dat -O $SANDBOX/regression-test.dat

echo "Fetching wind dataset..."
wget $WEB/wind.dat -O $SANDBOX/wind.dat


echo "Fetching winequality dataset..."
wget $WEB/winequality.dat -O $SANDBOX/winequality.dat

