#!/bin/bash
#Purpose: to compile and test a series of C source files 
#Parameters: none
#return: 0 on success, 1 on error

flag=0
for ((i=1;i<4;i++));do
	echo "C program: src$i.c"
	gcc -Wall -std=c99 src$i.c
	if [ $? -ne 0 ]
	then
	echo "Compilation of src$i.c failed!"
	flag=1
	else
	echo "Successfully complied!"
	for ((j=1;j<4;j++));do
	echo "Input file: input$j.data"
	./a.out < input$j.data
	    if [ $? -eq 0 ]
	    then 
	    echo "Run successfully"
	    else
	    echo "Run failed on input$j.data."
	    flag=1
	    fi
	done
	fi
	echo ""
done
exit $flag