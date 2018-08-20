#!/bin/bash
#purpose: to display directory information
#parameters: none
#Return: 0 if found, 1 otherwise

flag=0
echo "Welcome to MagicPhone!"
echo -n "Please enter part or all of a name to search for: "
read name
tail -n +32 house_dir_tab.txt > contact.txt
grep "$name" contact.txt | wc -l > match
if [ "$(grep "$name" contact.txt | wc -l)" -eq 0 ]
then
echo "Sorry, I could not find that person."
flag=1
elif [ "$(grep "$name" contact.txt | wc -l)" -eq 1 ]
then 
echo "Match Found!"
grep "$name" contact.txt | tr '\t' '\n' > output
echo "Name: $(head -1 output)"
echo "State: $(cat output | head -2 | tail -1)"
echo "Phone: $(cat output | tail -2 | head -1 | tr -dc '^[0-9]+\-[0-9]+')"
echo "Search complete on $(date)"
else 
echo "I found $(cat match) matches"
echo "You might want to be more specific."
grep "$name" contact.txt | tr '\t' '\n' | head -5 > output
echo "Name: $(head -1 output)"
echo "State: $(cat output | head -2 | tail -1)"
echo "Phone: $(cat output | tail -2 | head -1 | tr -dc '^[0-9]+\-[0-9]+')"
echo "Search complete on $(date)"
fi
exit $flag
