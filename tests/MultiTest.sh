#!/bin/bash

failTest=0
succesTest=0

for ((i=0 ; 10 - $i ; i++)); do
	./LitleFileTest.sh
	test=$!
	result=$?
	if [ $result -eq 0 ] ; then
    		((succesTest++))
  	else
		((failTest++))
		exit 1
	fi	
done

cleanup()
{
    kill -2 $test
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

echo "Succes = $succesTest, Fail = $failTest"
