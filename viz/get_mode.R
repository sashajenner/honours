#!/usr/bin/env Rscript
# get mode given frequency data
USAGE = 'usage: ./get_mode.R DATA_FILE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path)

print(df$signal[which(df$freq == max(df$freq))])
