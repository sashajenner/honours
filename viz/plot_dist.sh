#!/bin/sh
# plot the signal distribution of raw and pa-converted values
# given a SLOW5 file
# saved to SLOW5_PATH.freq.pdf and SLOW5_PATH.freq_pa.pdf
USAGE="usage: $0 (B|S)LOW5_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

SLOW5=$1
FREQ_TABLE="$SLOW5".freq
FREQ_TABLE_PA="$FREQ_TABLE"_pa

./create_freq_tables.sh "$SLOW5"
./freq_plot.R "$FREQ_TABLE"
./freq_plot.R "$FREQ_TABLE_PA"
