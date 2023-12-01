#!/usr/bin/env Rscript
# apply zigzag transformation to frequency table
# useful for converting delta frequency table to zigzag delta
USAGE = 'usage: ./freq_zigzag.R FREQ_TABLE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

freq_path = args[1]
df = read.delim(freq_path)

df$signal[df$signal >= 0] = 2 * df$signal[df$signal >= 0]
df$signal[df$signal < 0] = 2 * abs(df$signal[df$signal < 0]) - 1
df = df[order(df$signal),]
write.table(df, sep='\t', row.names=FALSE, quote=FALSE)
