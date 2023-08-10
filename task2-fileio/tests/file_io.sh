#!/bin/bash

###############################################
#
# ./file_io.sh <action> <target_file> <text>
# action - o(overwrite), a(append), d(display)
#
###############################################
key="$1"

case $key in
    -o|--overwrite)
    FILE_PATH="$2"
    TEXT="$3"
    echo $TEXT > $FILE_PATH
    ;;
    -a|--append)
    FILE_PATH="$2"
    TEXT="$3"
    echo $TEXT >> $FILE_PATH
    ;;
    -d|--display)
    FILE_PATH="$2"
    TEXT="$3"
    cat $FILE_PATH
    ;;
    --default)
    ;;
esac