#!/bin/bash
array_name=(1 2 "abc")
array_name[4]='happy'

echo ${array_name[@]}

echo ${#array_name[@]} ${#array_name[4]}
