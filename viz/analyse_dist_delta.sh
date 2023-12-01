#!/bin/sh
# analyse the distribution of signal differences and zigzag differences
# given SLOW5 file
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
FREQ_DELTA_ANALYSIS_FILE="$FREQ_TABLE_DELTA".analyse
FREQ_ZD_ANALYSIS_FILE="$FREQ_TABLE_ZD".analyse

./create_freq_tables_delta.sh "$SLOW5"
./freq_analyse.R "$FREQ_TABLE_DELTA" > "$FREQ_DELTA_ANALYSIS_FILE"
./freq_analyse.R "$FREQ_TABLE_ZD" > "$FREQ_ZD_ANALYSIS_FILE"
