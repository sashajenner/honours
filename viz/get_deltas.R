#!/usr/bin/env Rscript
# get the deltas between numbers in a column with a header
# output as one column with '_delta' appended to the header
USAGE = "./get_deltas.R DATA_FILE"

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

delta = diff(df[,1])
cat(names(df)[1], '_delta\n', sep = '')
for (x in delta) {
	cat(x, '\n', sep = '')
}
