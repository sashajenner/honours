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
EXT="${FILE##*.}"

# if given file is not a slow5 file, make tmp slow5 file
if [ "$EXT" = 'slow5' ]
then
	slow5file="$FILE"
else
	slow5file="$(mktemp)"
	"$SLOW5TOOLS" view "$FILE" --to slow5 2>/dev/null > "$slow5file"
fi


# print from the line beginning with '#read_id'
LINE_NUM_END_HDR=$(grep '^#read_id' "$slow5file" -n | cut -d : -f 1)
tail -n +"$LINE_NUM_END_HDR" "$slow5file"
