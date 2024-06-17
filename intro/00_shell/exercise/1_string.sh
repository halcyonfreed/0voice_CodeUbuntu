#!/bin/bash
:<<'
str='this is a string'
echo $str

your_name="0voice"
str="hello, I know you are \"$your_name\"! \n"
echo -e $str

greeting="hello, "$your_name"!"
greeting1="hello, ${your_name} !"

echo $greeting $greeting1
'

greeting2='hello, '$your_name'!'
greeting3='hello, ${your_name} !'

echo $greeting2 $greeting3

string='abcd'
echo ${#string}


string='0voice is a great college'
echo ${string:1:4}
echo `expr index "$string" ic`
