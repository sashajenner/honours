#!/bin/sh
# write the deltas between numbers in a column with a header
# to DATA_FILE.delta as one column with '_delta' appended to the header
USAGE="usage: $0 DATA_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

FILE="$1"
BASEDIR=$(dirname "$0")

"$BASEDIR"/get_deltas.R "$FILE" > "$FILE".delta
