#!/bin/sh
# plot the read length distribution
# given a SLOW5 file
# saved to SLOW5_PATH.freq_readlen.pdf
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_READLEN="$FREQ_TABLE"_readlen

./create_freq_table_readlen.sh "$SLOW5"
./freq_plot.R "$FREQ_TABLE_READLEN"
