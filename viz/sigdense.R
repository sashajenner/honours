#!/usr/bin/env Rscript
# plot numbers in a column as a density histogram
USAGE = 'usage: ./sigdense DATA_FILE'

library(ggplot2)
library(plotly)
library(htmlwidgets)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)
name = basename(path)
colname = names(df)[1]
title = paste(name, colname, 'Density Histogram')
xtitle = 'ADC Output'

plot = ggplot(df, aes(df[,1])) +
	geom_density() +
	xlab(xtitle) +
	labs(title = title)
ggsave(paste0(path, '.dense.pdf'), plot)

plotly = ggplotly(plot)
saveWidget(plotly, paste0(path, '.dense.html'))
