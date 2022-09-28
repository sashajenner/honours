#!/usr/bin/env Rscript
# get the zigzag of numbers in a column with a header
# output as one column with '_zigzag' appended to the header
USAGE = "./get_zigzag.R DATA_FILE"

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

cat(names(df)[1], '_zigzag\n', sep = '')
for (x in df[,1]) {
	if (x < 0)
		x = -2 * x - 1
	else
		x = 2 * x
	cat(x, '\n', sep = '')
}
