#!/bin/sh
# analyse the distribution of read lengths
# given SLOW5 file
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_READLEN="$FREQ_TABLE"_readlen
FREQ_READLEN_ANALYSIS_FILE="$FREQ_TABLE_READLEN".analyse

./create_freq_table_readlen.sh "$SLOW5"
./freq_analyse.R "$FREQ_TABLE_READLEN" > "$FREQ_READLEN_ANALYSIS_FILE"
