#!/bin/bash    

mkdir -p docjs
mkdir -p docjs/temp

function getmarkdown {
    #find lines that contain '//#', then remove white space, then cut three chars, then remove white space
    grep '//#' $1 | sed 's/^[ \t]*//' | cut -c 4-
} 

getmarkdown src/qminer/qminer_js.h > ./docjs/temp/jsdoc_1.txt

# over each line in jsdoc.txt
#  if 'JSIMPLEMENT:x' is found, then (append!) getmarkdown x jsfinal.txt
#  else append line to jsfinal.txt

function getmarkdownJS {
   echo > $2
   while IFS= read line
   do
     echo "$line" | grep "JSIMPLEMENT" -q
     if [ $? -eq 0 ]
     then
        #a = source path
        a=$(echo "$line" | cut -c 13-)
        getmarkdown $a >> $2
     else
        # print to $2
        echo "$line" >> $2
     fi
   done < $1
}

getmarkdownJS ./docjs/temp/jsdoc_1.txt ./docjs/temp/jsdoc_2.txt

# remove leading space
cat ./docjs/temp/jsdoc_2.txt | sed 's/^ //' > ./docjs/jsdocfinal.txt

