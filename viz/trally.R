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
colnames = names(df)
title = paste(name, 'Transition Matrix')
xtitle = colnames[1]
ytitle = colnames[2]

plot = ggplot(df, aes(x=from, y=to, colour=freq, fill=freq)) +
	geom_tile() +
	#geom_abline(slope=1) +
	labs(title = title)
ggsave(paste0(path, '.trix.pdf'), plot)

from = unique(df[,1])
side = length(from)
dfm = matrix(df[,3], nrow=side, ncol=side)

plotly = plot_ly(x=from, y=from, z=dfm,
		 type = 'heatmap',
		 name = title) %>%
	layout(xaxis = list(title = xtitle),
		yaxis = list(title = ytitle))
saveWidget(plotly, paste0(path, '.trix.html'))
