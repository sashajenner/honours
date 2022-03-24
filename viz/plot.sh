#!/bin/sh
# plot everything for numbers in a column
USAGE="usage: $0 DATA_FILE"

if [ $# -ne 1 ]
then
	echo "$USAGE"
	exit 1
fi

FILE="$1"
FILE_STAT="$FILE.stat"
FILE_DELTA1="$FILE.delta"
FILE_DELTA1_STAT="$FILE_DELTA1.stat"
FILE_SIGNREP1="$FILE_DELTA1.signrep"
FILE_SIGNREP1_STAT="$FILE_SIGNREP1.stat"
#FILE_DELTA2="$FILE.delta.delta"
#FILE_SIGNREP2="$FILE.delta.delta.signrep"
BASEDIR=$(dirname "$0")

"$BASEDIR/sig.R" "$FILE"
"$BASEDIR/sighist.R" "$FILE"
"$BASEDIR/sigdense.R" "$FILE"
"$BASEDIR/sigstat.R" "$FILE" > "$FILE_STAT"

"$BASEDIR/get_deltas.R" "$FILE" > "$FILE_DELTA1"
"$BASEDIR/sig.R" "$FILE_DELTA1"
"$BASEDIR/sighist.R" "$FILE_DELTA1"
"$BASEDIR/sigdense.R" "$FILE_DELTA1"
"$BASEDIR/sigstat.R" "$FILE_DELTA1" > "$FILE_DELTA1_STAT"

"$BASEDIR/get_signrep.R" "$FILE_DELTA1" > "$FILE_SIGNREP1"
"$BASEDIR/sig.R" "$FILE_SIGNREP1"
"$BASEDIR/sighist.R" "$FILE_SIGNREP1"
"$BASEDIR/sigdense.R" "$FILE_SIGNREP1"
"$BASEDIR/sigstat.R" "$FILE_SIGNREP1" > "$FILE_SIGNREP1_STAT"
