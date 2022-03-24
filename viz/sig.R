#!/usr/bin/env Rscript
# plot numbers in a column on the y-axis
USAGE = 'usage: ./sig.R DATA_FILE'

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
title = paste0(name, ' Raw Signal')
xtitle = 'Data Point'
ytitle = 'ADC Output'

plot = ggplot(df, aes(1:nrow(df), df[,1])) +
	geom_line() +
	xlab(xtitle) +
	ylab(ytitle) +
	labs(title = title)
ggsave(paste0(path, '.pdf'), plot, width=40)

plotly = plot_ly(x = 1:nrow(df), y = df[,1],
		 type = 'scatter',
		 mode = 'lines',
		 name = title) %>%
	layout(xaxis = list(title = xtitle),
	       yaxis = list(title = ytitle))
saveWidget(plotly, paste0(path, '.html'))
