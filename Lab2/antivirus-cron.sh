#!/bin/bash

echo "Script executed at: $(date +'%Y-%m-%d %H:%M:%S')" >> ~/Documents/OS-Labs/Lab2/execution.log #this is used as a test to check that the cron job executes the way we want
check_extension(){
    find "$dir" -type f \( -name "*.exe" -o -name "*.bat" -o -name "*.vbs" -o -name "*.scr" -o -name "*.ps1" \) > mal_files # -o is for or
}
check_content(){
    grep -rilE "virus|trojan|malware|work|ransomware" $dir >> mal_files 
}
quar_files(){
    if [ -s mal_files ]; then #checks that the file is not empty and exists
    while read -r file; do #reads every line from the file
        mv "$file" $mal_dir
        echo "$file is malicious and it is DELETED"
    done < mal_files #take input from the file
    > mal_files #empty the file
    fi
}
check_files(){
    check_extension
    check_content
    quar_files
}
if [ $# -ne 2 ]
then
    echo "Exactly 2 arguments are required for this script to run."
    exit
else
    dir=$1
    mal_dir=$2
fi
olddir=directory-info.last
newdir=directory-info.new
ls -l $dir > $newdir    
if cmp -s $olddir $newdir; then # -s is for silent, return only the exit code and dont't print anything to screen
rm -f $newdir
else
    check_files
    ls -l $dir > $olddir
    
fi