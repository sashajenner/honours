#!/bin/bash
# get number of points in each read signal into one column with header
# 'len_raw_signal'
USAGE="$0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

FILE="$1"

./rm_hdr.sh "$FILE" | cut -f 7
