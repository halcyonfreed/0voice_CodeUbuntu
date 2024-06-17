#!/bin/bash

echo "Hello World"

zerovoice="www.0voice.com"
echo $zerovoice


for file in $(ls /home/halcyon/share/); do
	echo "${file}"
done
