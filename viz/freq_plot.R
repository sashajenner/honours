#!/usr/bin/env Rscript
# plot frequency table as a histogram
# save the plot to FILE.pdf
USAGE = 'usage: ./freq_plot.R FREQ_TABLE'

library(ggplot2)
library(plotly)
library(htmlwidgets)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path)

name = basename(path)
colname = names(df)[1]
title = paste(name, 'Histogram')
xtitle = colname

plot = ggplot(df, aes(df[,1], df[,2])) +
	geom_histogram(stat='identity') +
	xlab(xtitle) +
	ylab('') +
	labs(title = title)
ggsave(paste0(path, '.pdf'), plot)

plotly = plot_ly(x = df[,1], y=df[,2],
		 type = 'bar',
		 name = title) %>%
	layout(xaxis = list(title = xtitle))
saveWidget(plotly, paste0(path, '.html'))
