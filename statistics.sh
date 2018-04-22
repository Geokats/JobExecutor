#!/bin/sh
files=./logs/*.log
# echo $files;

searchCount=$(cat $files | awk -F ' : ' '{if($2=="search") print $2" "$3}' | sort | uniq -c | wc -l)
echo "The workers performed a total of $searchCount searches";

mostFound=$(cat $files | awk -F ' : ' '{for(i=4;i<=NF;i++) if($2 != "wc") print $3}' | sort | uniq -c | sort -nr | head -n 1)
mostFoundWord=$(echo $mostFound | awk -F ' ' '{print $2}')
filesFound=$(cat $files | awk -F ' : ' -v word=$mostFoundWord '{for(i=4;i<=NF;i++) if($2 != "wc" && $3 == word) print $i}' | sort | uniq -c | wc -l)
echo "The word $mostFoundWord was found the most times [totalNumFilesFound: $filesFound]"

leastFound=$(cat $files | awk -F ' : ' '{for(i=4;i<=NF;i++) if($2 != "wc") print $3}' | sort | uniq -c | sort -n | head -n 1)
leastFoundWord=$(echo $leastFound | awk -F ' ' '{print $2}')
filesFound=$(cat $files | awk -F ' : ' -v word=$leastFoundWord '{for(i=4;i<=NF;i++) if($2 != "wc" && $3 == word) print $i}' | sort | uniq -c | wc -l)
echo "The word $leastFoundWord was found the least times [totalNumFilesFound: $filesFound]"
