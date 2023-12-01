#!/bin/sh
# create the frequency tables of the signal differences and zigzag differences
# given a SLOW5 file
# if they already doesn't exist
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_DELTA="$FREQ_TABLE"_delta
FREQ_TABLE_ZD="$FREQ_TABLE_DELTA"_zigzag

if [ ! -s "$FREQ_TABLE_DELTA" ]
then ./freq_delta_slow5 "$SLOW5" > "$FREQ_TABLE_DELTA"
fi
if [ ! -s "$FREQ_TABLE_ZD" ]
then ./freq_zigzag.R "$FREQ_TABLE_DELTA" > "$FREQ_TABLE_ZD"
fi
