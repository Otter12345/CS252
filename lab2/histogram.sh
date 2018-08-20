#!/bin/bash
#Purpose: display statistics for a provided set of data
#Parameters: one, input filename
#return: 0 on success, 1 on error
file=$1
i=0

#check if a file exists
test -f $file 
if (($?!=0))
then
echo "$file is not readable"
exit 1
fi

#count total number of scores
declare -a score
num=$(wc -l < $file)
echo "$num scores total..."
cut "-d " -f2 $file > scores
#store scores into array
while read -r line
do
score[i++]=$line
done < scores

#Calculate histogram
declare -a his=(0 0 0 0 0 0 0 0 0 0 0)
sum=0
for ((j=0;j<$num;j++))
do
index=$((${score[j]}/10))
sum=$(($sum+${score[j]}))
his[$index]=$((${his[$index]}+1))
done
#cal avg score
avg=$(($sum/$num))
#display histogram
num=100
for ((j=10;j>-1;j--))
do
printf '%3s: ' $num
for ((k=0;k<${his[j]};k++))
do
echo -n "="
done
echo ""
num=$(($num-10))
done
echo "Average: $avg"
exit 0