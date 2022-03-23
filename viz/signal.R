#!/usr/bin/env Rscript
# plot numbers in a column on the y-axis
usage = "usage: ./signal.R DATA_FILE"

library(ggplot2)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(usage)
}

path = args[1]
df = read.csv(path)
plot = ggplot(df, aes(1:nrow(df), raw_signal)) +
    geom_line() +
	xlab('') +
	ylab('Amplitude') +
	labs(title = 'Signal')
ggsave(paste0(path, '.pdf'), plot)
