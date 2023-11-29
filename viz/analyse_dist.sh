#!/bin/sh
# analyse the distribution of
# raw and pA-converted signal values
# given SLOW5 file
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_PA="$FREQ_TABLE"_pa
FREQ_ANALYSIS_FILE="$FREQ_TABLE".analyse
FREQ_PA_ANALYSIS_FILE="$FREQ_TABLE_PA".analyse

./create_freq_tables.sh "$SLOW5"
./freq_analyse.R "$FREQ_TABLE" > "$FREQ_ANALYSIS_FILE"
./freq_analyse.R "$FREQ_TABLE_PA" > "$FREQ_PA_ANALYSIS_FILE"
