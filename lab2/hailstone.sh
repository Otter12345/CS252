#!/bin/bash
#Purpose: to calculate and display a Hailstone sequence
#Parameters: start n and limit l
#Return: 0 on success, 1 on error

#check if arguments are supplied
if [ $# -eq 0 ]
then
exit 1
fi

n=$1
l=$2
echo -n "$n "
for ((i=1;i<l;i++))
do
  if (($n % 2 == 0 ))
  then
  n=$((n/2))
  else
  n=$((3*n+1))
  fi
  echo -n "$n "
done
echo ""
exit 0
  