#!/usr/bin/env Rscript
# data analysis of read statistics
# given the read statistics in tsv format
USAGE = 'usage: ./ida.R STATS_TSV'

library(ggplot2)
library(tidyverse)
library(tikzDevice)

# check args
args = commandArgs(TRUE)
if (length(args) != 1) {
	stop(USAGE)
}

# read file
path = args[1]
df = read.delim(path)

# total number of reads?
#cat('num reads: ', nrow(df), '\n', sep='')

# total number of signals?
#num_sigs = sum(as.numeric(df$n))
#cat('num signals: ', num_sigs, '\n', sep='')

# smallest read length?
#cat('read length min: ', min(df$n), '\n', sep='')

# largest read length?
#cat('read length max: ', max(df$n), '\n', sep='')

# average read length?
#cat('read length mean: ', mean(df$n), '\n', sep='')

# quartiles?
#q = quantile(df$n)
#cat('read length quartiles: ', q[2], q[3], q[4], '\n', sep=' ')

# sd?
#cat('read length sample sd: ', sd(df$n), '\n', sep='')

#getmode <- function(v) {
#	uniqv <- unique(v)
#	uniqv[which.max(tabulate(match(v, uniqv)))]
#}

# mode?
#cat('read length mode: ', getmode(df$n), '\n', sep='')

# distribution of read lengths?
#tikz(file = paste0(path, '.nhist.tex'), width = 5, height = 5)
#ggplot(df, aes(x=n/10^3)) +
#	geom_histogram(binwidth=1) +
#	xlim(0,1000) +
#	xlab('Read Length ($\\times 10^3$)') +
#	ylab('Count')
#	#theme(axis.text.y=element_blank(),
#	#      axis.ticks.y=element_blank()) +
#	#labs(title='boxplot of read lengths')
#dev.off()

# smallest signal value?
#cat('raw signal min: ', min(df$min), '\n', sep='')

# largest signal value?
#cat('raw signal max: ', max(df$max), '\n', sep='')

# average signal value?
#cat('raw signal mean: ', sum(df$mean * df$n) / num_sigs, '\n', sep='')

# variance of signal values?
#raw_sig_var = sum(df$var * df$n) / num_sigs
#cat('raw signal var: ', raw_sig_var, '\n', sep='')

# standard deviation of signal values?
#cat('raw signal sd: ', sqrt(raw_sig_var), '\n', sep='')

# distribution of signal values? TODO

# smallest signal value in pa?
#cat('signal (pa) min: ', min(df$min_pa), '\n', sep='')

# largest signal value in pa?
#cat('signal (pa) max: ', max(df$max_pa), '\n', sep='')

# average signal value in pa?
#cat('signal (pa) mean: ', sum(df$mean_pa * df$n) / num_sigs, '\n', sep='')

# variance of signal values?
#sig_var = sum(df$var_pa * df$n) / num_sigs
#cat('signal (pa) var: ', sig_var, '\n', sep='')

# standard deviation of signal values?
#cat('signal (pa) sd: ', sqrt(sig_var), '\n', sep='')

# distribution of signal values in pa? TODO

# start_time vs mean
#hist(df$channel_num)
#df_cn1 = df[df$channel_num == 2642,]
#ggplot(df_cn1, aes(start_time, mean)) +
#	geom_point()

df_avg = df %>% group_by(start_time) %>%
	summarise(mean_n = mean(n))

ggplot(df_avg, aes(start_time, mean_n)) +
	geom_point()
