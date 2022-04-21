#!/usr/bin/env Rscript
# plot transition probability matrix
USAGE = 'usage: ./trally.R DATA_FILE'

library(ggplot2)
library(plotly)
library(htmlwidgets)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path, sep='\t')
name = basename(path)
colname = names(df)[1]
title = paste(name, 'Transition Matrix')
xtitle = colname

plot = ggplot(df, aes(x=to, y=from, colour=freq, fill=freq)) +
	geom_tile() +
	labs(title = title)
ggsave(paste0(path, '.trix.pdf'), plot)
