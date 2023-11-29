#!/usr/bin/env Rscript
# convert frequency table to pa
USAGE = 'usage: ./freq_rawtopa.R FREQ_TABLE DOR_FILE'

args = commandArgs(TRUE)

if (length(args) != 2) {
	stop(USAGE)
}

freq_path = args[1]
dor_path = args[2]
df = read.delim(freq_path)
dor = read.delim(dor_path)

df$signal = (df$signal + dor$offset) * dor$range / dor$digitisation
write.table(df, sep='\t', row.names=FALSE, quote=FALSE)
