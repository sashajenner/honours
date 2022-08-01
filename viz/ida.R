#!/usr/bin/env Rscript
# data analysis of read statistics
# given the read statistics in tsv format
USAGE = 'usage: ./ida.R STATS_TSV'

library(ggplot2)

# check args
args = commandArgs(TRUE)
if (length(args) != 1) {
	stop(USAGE)
}

# read file
path = args[1]
df = read.delim(path)

# smallest read length?
cat('smallest read length: ', min(df$n), '\n', sep='')

# largest read length?
cat('largest read length: ', max(df$n), '\n', sep='')

# average read length?
cat('average read length: ', mean(df$n), '\n', sep='')

# distribution of read lengths?
ggplot(df, aes(x=n)) +
	geom_boxplot() +
	theme(axis.text.y=element_blank(),
	      axis.ticks.y=element_blank()) +
	labs(title='boxplot of read lengths')

# smallest signal value?
cat('smallest signal value: ', min(df$min), '\n', sep='')

# largest signal value?
cat('largest signal value: ', max(df$max), '\n', sep='')

# average signal value?
cat('average signal value: ', mean(df$mean), '\n', sep='')

# distribution of signal values? TODO

# smallest signal value in pa?
cat('smallest signal value (pa): ', min(df$min_pa), '\n', sep='')

# largest signal value in pa?
cat('largest signal value (pa): ', max(df$max_pa), '\n', sep='')

# average signal value in pa?
cat('average signal value (pa): ', mean(df$mean_pa), '\n', sep='')

# distribution of signal values in pa? TODO
