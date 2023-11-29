#!/bin/sh
# create the frequency tables raw and pA-converted
# given a SLOW5 file
# if they already don't exist
# assumes digitisation, offset and range are constant across the SLOW5 file
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
DOR_FILE="$SLOW5".dor
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_PA="$FREQ_TABLE"_pa

if [ ! -s "$FREQ_TABLE" ]
then ./freq_slow5 "$SLOW5" > "$FREQ_TABLE"
fi
if [ ! -s "$FREQ_TABLE_PA" ]
then
	# get the digitisation, offset and range
	./get_dor_slow5 "$SLOW5" > "$DOR_FILE"
	# convert frequency table to pa
	./freq_rawtopa.R "$FREQ_TABLE" "$DOR_FILE" > "$FREQ_TABLE_PA"
fi
