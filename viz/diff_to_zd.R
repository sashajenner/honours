#!/usr/bin/env Rscript
# delta to zigzag delta
USAGE = 'usage: ./diff_to_zd.R DELTA_DATA_FILE'

library(ggplot2)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path)
name = basename(path)

max_zd = max(min(df$signal) * -2 - 1,
	     max(df$signal) * 2)
df_zigzag = vector(length=max_zd, mode='numeric')

for (i in 1:nrow(df)) {
	zig = abs(df$signal[i]) * 2
	if (df$signal[i] < 0) {
		zig = zig - 1
	}

	df_zigzag[zig + 1] = df$freq[i]
}

df_zigzag_1byte = df_zigzag[1:256]

#for (i in 1:length(df_zigzag_1byte)) {
#	cat(i - 1, ': ', df_zigzag_1byte[i], '\n')
#}

df_zigzag_actual = data.frame(signal = 1:length(df_zigzag),
				freq = df_zigzag)
ggplot(df_zigzag_actual, aes(x=signal, y=freq)) +
	geom_col()

df_zigzag_1byte_actual = data.frame(signal = 1:length(df_zigzag_1byte),
				freq = df_zigzag_1byte)
ggplot(df_zigzag_1byte_actual, aes(x=signal, y=freq)) +
	geom_col()
