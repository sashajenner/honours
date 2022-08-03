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

# total number of reads?
cat('num reads: ', nrow(df), '\n', sep='')

# total number of signals?
num_sigs = sum(as.numeric(df$n))
cat('num signals: ', num_sigs, '\n', sep='')

# smallest read length?
cat('read length min: ', min(df$n), '\n', sep='')

# largest read length?
cat('read length max: ', max(df$n), '\n', sep='')

# average read length?
cat('read length mean: ', mean(df$n), '\n', sep='')

# distribution of read lengths?
ggplot(df, aes(x=n)) +
	geom_boxplot() +
	theme(axis.text.y=element_blank(),
	      axis.ticks.y=element_blank()) +
	labs(title='boxplot of read lengths')

# smallest signal value?
cat('raw signal min: ', min(df$min), '\n', sep='')

# largest signal value?
cat('raw signal max: ', max(df$max), '\n', sep='')

# average signal value?
cat('raw signal mean: ', sum(df$mean * df$n) / num_sigs, '\n', sep='')

# variance of signal values?
cat('raw signal var: ', sum(df$var * df$n) / num_sigs, '\n', sep='')

# standard deviation of signal values?
cat('raw signal sd: ', sum(df$sd * df$n) / num_sigs, '\n', sep='')

# distribution of signal values? TODO

# smallest signal value in pa?
cat('signal (pa) min: ', min(df$min_pa), '\n', sep='')

# largest signal value in pa?
cat('signal (pa) max: ', max(df$max_pa), '\n', sep='')

# average signal value in pa?
cat('signal (pa) mean: ', sum(df$mean_pa * df$n) / num_sigs, '\n', sep='')

# variance of signal values?
cat('signal (pa) var: ', sum(df$var_pa * df$n) / num_sigs, '\n', sep='')

# standard deviation of signal values?
cat('signal (pa) sd: ', sum(df$sd_pa * df$n) / num_sigs, '\n', sep='')

# distribution of signal values in pa? TODO
