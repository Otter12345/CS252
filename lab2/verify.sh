#!/bin/bash
#Purpose: To display validation information regarding data
#Parameters: one, input filename	
#Return: 0 on success, 1 for improper arguments, 2 for filename is not readable

#check if file exists
test -f $1
if (($?!=0))
then
echo "Error: $1 is not readable!"
exit 2
fi

#save name and addr 
declare -a input
input=$(sed -e 's/;/' <<< $1)
echo $input
name=${input[0]}
addr=$(cut -d';' -f2 $1)
last=$(echo $name | cut -d, -f1)
first=$(echo $name | cut -d',' -f2)
addnum=$(echo $addr | cut -d' ' -f1)
adds=$(echo $addr | cut -d' ' -f2)
flag=0
flag1=0

#check first & mid name
if [[ ! $first =~ [A-Za-z*] ]] || [[ $first =~ " "s$ ]] 
then 
flag=1
fi 

#echo $first 
#echo "first $flag"

if [[ ! $(echo $first | grep \.) ]]
then
mid=$(echo $first | cut -d' ' f2)
if [[ ! $mid =~ ^[A-Z][..]$ ]] 
then 
flag=1
fi
fi

#echo "mid $flag"

#check last name
if [[ ! $last =~ ^[A-Z][a-z]+ ]]
then
flag=1
fi

#echo "last $flag"
#echo $addnum

#check addr. number
if [[ ! $addnum =~ ^[0-9]+ ]]
then
flag1=1
fi

#echo "addnum $flag1"

#check addrs.
if [[ $adds =~ " "$ ]]; then 
flag1=1
fi

#echo "addnumT $flag1"

declare -a addrr
IFS=' '
read -ra addrr <<< "$adds"
for i in "${addrr[@]}"; do
if [[ ! addrr[i] =~ ^[0-9A-Za-z.]+ ]]
then
flag1=1
fi
done

#echo "adds $flag1"

#print error message
if [ $flag = 1 ] && [ $flag1 = 1 ]
then
echo "Invalid name and address!"
elif (( $flag == 1 ))
then
echo "Invalid name!"
elif (($flag1== 1))
then
echo "Invalid address!"
else
echo "Data is valid."
fi

