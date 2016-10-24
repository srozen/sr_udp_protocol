#!/bin/bash

failTest=0
succesTest=0

for ((i=0 ; 20 - $i ; i++)); do
	./MyTest.sh &
	test=$!
	sleep 20
	if kill -0 $test &> /dev/null ; then
		kill -2 $test
		((failTest++))
	else
		if ! wait $receiver_pid ; then
    			((failTest++))
  		else
			((succesTest++))
		fi	
	fi
done

cleanup()
{
    kill -2 $test
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

echo "Succes = $succesTest, Fail = $failTest"
