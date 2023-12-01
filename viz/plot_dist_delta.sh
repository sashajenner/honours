#!/bin/sh
# plot the signal differences and zigzag differences distribution
# given a SLOW5 file
# saved to SLOW5_PATH.freq_delta.pdf and SLOW5_PATH.freq_delta_zigzag.pdf
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

./create_freq_tables_delta.sh "$SLOW5"
./freq_plot.R "$FREQ_TABLE_DELTA"
./freq_plot.R "$FREQ_TABLE_ZD"
