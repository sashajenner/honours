#!/bin/sh
# create the frequency table of read lengths
# given a SLOW5 file
# if they already don't exist
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_READLEN="$FREQ_TABLE"_readlen

if [ ! -s "$FREQ_TABLE_READLEN" ]
then ./freq_readlen_slow5 "$SLOW5" > "$FREQ_TABLE_READLEN"
fi
