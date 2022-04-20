#!/usr/bin/env Rscript
# plot histogram given frequency data
USAGE = 'usage: ./freqhist.R DATA_FILE'

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

plot = ggplot(df, aes(x=df[,1], y=df[,2])) +
	geom_histogram(stat="identity") +
	xlab(xtitle) +
	labs(title = title)
ggsave(paste0(path, '.hist.pdf'), plot)

plotly = plot_ly(x = df[,1], y=df[,2],
		 type = 'bar',
		 name = title) %>%
	layout(xaxis = list(title = xtitle))
saveWidget(plotly, paste0(path, '.hist.html'))
