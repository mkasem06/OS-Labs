#!/bin/bash

if [ $# -ne 2 ]
then
    echo "Exactly 2 arguments are required for this script to run."
    exit
else
    dir=$1
    mal_dir=$2
fi
list_options(){
    printf "1. Restore file\n2. Permanently delete file\n3. Leave file as it is\n"
}

while true; do
    ls -1 $mal_dir | nl #this lists the files in a numbered way
    ls -1 $mal_dir > mal_files
    if [ ! -s mal_files ] # -s checks if the file exists and has size greater than 0
    then
        echo "No malicious files to review"
        exit
    fi
    read -p "Enter the number of the file: " file_choice
    file=$(sed -n "${file_choice}p" mal_files) #this gets the content of the line number (file_choice)
    list_options
    read -p "Enter your choice: " action_choice
    while [[ $action_choice -gt 3 || $action_choice -lt 1 ]]; do
        echo "Invalid choise entered"
        read -p "Enter your choice: " action_choice
    done
    if [ $action_choice -eq 1 ]
    then
        echo "Restored $file to $dir"
        echo "$dir/$file" >> whitelist.txt 
        mv $mal_dir/$file $dir
    elif [ $action_choice -eq 2 ]
    then
        echo "$file permanently deleted"
        rm $mal_dir/$file
    else
        list_options
    fi
done


