#!/usr/bin/env Rscript
# data analysis of read statistics
# given the read compression statistics in tsv format
# and the read stall statistics in tsv format
USAGE = 'usage: ./ida_stall.R PRESS_STATS_TSV STALL_STATS_TSV'

library(ggplot2)
#library(plotly)
library(tikzDevice)
library(tidyverse)
#library(htmlwidgets)

# check args
args = commandArgs(TRUE)
if (length(args) != 2) {
	stop(USAGE)
}

# read file
path1 = args[1]
press_df = read.delim(path1)
press_df = press_df %>%
		mutate(method = str_replace_all(method, '_', "-")) %>%
		mutate(method = str_replace(method, 'rccm', 'rc01s')) %>%
		mutate(method = str_replace(method, 'rc01s-svbbe21-zd', 'stall-fz'))
press_df = press_df[press_df$method != 'rc01s-vbe21-zd',]

path2 = args[2]
stall_df = read.delim(path2)
colnames(stall_df)[2] = 'read'

#press_df_rcvzd = press_df[press_df$method =='rccm_vbe21_zd',]
#ggplot(press_df) +
#	geom_point(aes(x=depress_bytes/2, y=press_bytes, color=method))

#press_df_rcsvzd = press_df[press_df$method =='rccm_svbbe21_zd',]
#press_df_rcsvzd_stall = merge(press_df_rcsvzd, stall_df, by='read')
#ggplot(press_df_rcsvzd_stall) +
#	geom_point(aes(x=n, y=press_ratio))

press_df_stall = merge(press_df, stall_df, by='read')
#ggplot(press_df_stall) +
#	geom_point(aes(x=n, y=press_ratio, color=method))

press_df_stall_best = press_df_stall %>%
			group_by(read) %>%
			filter(press_bytes == min(press_bytes))
#tikz(file = paste0(path1, '.best.tex'), width = 6, height = 5)
plot = ggplot(press_df_stall_best) +
	geom_point(aes(x=n, y=press_ratio, color=method), alpha=0.4) +
	xlab('Stall Length') +
	ylab('Compression Ratio') +
	labs(color = 'Best Method')
#dev.off()
ggsave(paste0(path1, '.best.pdf'), plot, width=7, height=5)

#ggplot(press_df_stall_best) +
#	geom_histogram(aes(x=n, fill=method))
#tikz(file = paste0(path1, '.prop.tex'), width = 6, height = 5)
#ticks=c(0, 1500, 2500, 5000, 7500, 10000)
#ggplot(press_df_stall_best) +
#	geom_histogram(aes(x=n, fill=method), position='fill', binwidth=50) +
#	xlab('Stall Length') +
#	ylab('Proportion of Reads') +
#	labs(fill = 'Method') +
#	geom_vline(xintercept = 1500, lintype='dotted') +
#	scale_x_continuous(breaks=ticks,
#			   limits=c(ticks[1], ticks[length(ticks)]))
#dev.off()
#ggplot(press_df_stall_best) +
#	geom_freqpoly(aes(x=n, color=method))
