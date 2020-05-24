#!/bin/sh

first=`pidof shutdown`
while [ 1 ]
do
	second=`pidof shutdown`
	if [ ! -z $second ]
	then
		echo "first = $first , second = $second"
		if [ $first != $second ]
		then
			kill `pidof genpowerd`
			kill `pidof shutdown`
			echo "Error occur!!!"
			exit
		fi
	fi
	sleep 1
done
