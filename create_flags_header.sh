#!/bin/bash

FILE=src/CslFlags.h
INC_PREFIX=""
FLAG_PATH=src/img/flags

> $FILE

declare -a flags

for f in ${FLAG_PATH}/*.xpm
do
 flag=${f##*/}
 echo "#include \"${INC_PREFIX}${flag}\"" >>$FILE
 test $flag = "local.xpm" -o $flag = "unknown.xpm" || \
  flags=(${flags[*]} ${flag%%.xpm})
done

declare -i c=0
declare -i j=1
declare -i e=${#flags[*]}-1

echo -e "\nstatic const char **country_flags[] = {" >>$FILE
for flag in ${flags[*]}
do
 echo -n "${flag//\./_}_xpm" >> $FILE
 test $c != $e && echo -n "," >>$FILE
  test $j -eq 10 && { echo "" >>$FILE; j=0; } || { echo -n " " >>$FILE; }
 j+=1
 c+=1
done
echo -e "\n};" >>$FILE

c=0
j=1
echo -e "\nstatic const char *country_codes[] = {" >>$FILE
for flag in ${flags[*]}
do
 echo -n "\"$flag\"" >> $FILE
 test $c != $e && echo -n "," >>$FILE
  test $j -eq 10 && { echo "" >>$FILE; j=0; } || { echo -n " " >>$FILE; }
 j+=1
 c+=1
done
echo -e "\n};" >>$FILE
