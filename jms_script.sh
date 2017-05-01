#!/bin/sh
# 
# File:   jms_script.sh
# Author: angelique
#
# Created on Apr 24, 2017, 2:36:50 PM
#


clear

echo "The script starts here!"
echo

#for var in "$@"
#do
#    echo "$var"
    
#done

path="$2"
command="$4"
size="$5"

echo "$path"
echo "$command"
echo "$size"

if [ "$command" = "list" ]
then
    echo "LIST!"
    for entry in "$path"/*
    do
      echo "$entry"
    done
	
elif [ "$command" = "size" ]
then
    echo "SIZE!"
    du -ah "$path" | grep 'sdi13*' | sort -h | head -"$size"
else
    echo "PURGE!"
    find "$path" | grep 'sdi13*' | xargs rm -fr
fi