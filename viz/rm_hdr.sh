#!/bin/sh
# remove (b|s)low5 header except for the column names
# print in slow5 format
USAGE="usage: $0 (B|S)LOW5_FILE"

SLOW5TOOLS='/home/en0cs/work/garvan/slow5/slow5tools/slow5tools'

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

FILE="$1"

# print from the line beginning with '#read_id'
LINE_NUM_END_HDR=$("$SLOW5TOOLS" view "$FILE" --to slow5 2>/dev/null |
	grep '^#read_id' -n | cut -d : -f 1)
"$SLOW5TOOLS" view "$FILE" --to slow5 2>/dev/null |
	tail -n +"$LINE_NUM_END_HDR"
