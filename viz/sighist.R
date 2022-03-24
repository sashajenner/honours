#!/usr/bin/env Rscript
# plot numbers in a column as a histogram
USAGE = 'usage: ./sighist.R DATA_FILE'

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
title = paste(name, 'Histogram')
xtitle = colname

plot = ggplot(df, aes(df[,1])) +
	geom_histogram() +
	xlab(xtitle) +
	labs(title = title)
ggsave(paste0(path, '.hist.pdf'), plot)

plotly = plot_ly(x = df[,1],
		 type = 'histogram',
		 name = title) %>%
	layout(xaxis = list(title = xtitle))
saveWidget(plotly, paste0(path, '.hist.html'))
