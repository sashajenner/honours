#!/usr/bin/env Rscript
# plot numbers in a column on the y-axis
USAGE = 'usage: ./sig.R DATA_FILE'

library(ggplot2)
library(plotly)
library(htmlwidgets)
library(tikzDevice)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)
name = basename(path)
colname = names(df)[1]
title = paste(name, colname)
xtitle = 'Data Point'
ytitle = colname

#x=1:nrow(df)
x=29000:(29000+200)
tikz(file = paste0(path, '.dna.tex'), width = 5, height = 5)
ggplot(data.frame(df[x,]), aes(x, df[x,1])) + #color=cut(x, c(1,14,789,831,3000)))) +
	geom_line() +
	xlab('Position in Read') +
	ylab('Raw Signal')
	#xlim(0,1500) +
	#geom_rect(aes(xmin=1, xmax=14, ymin=-Inf,ymax=Inf),fill='red',alpha=0.002) +
	#geom_vline(xintercept=14,colour='red',alpha=0.9) +
	#geom_vline(xintercept=789,colour='dark orange',alpha=0.9) +
	#geom_vline(xintercept=831,colour='blue',alpha=0.9)
	#scale_color_manual(values=c("[1,14]"="red",
	#			    "(14,789]"="green",
	#			    "(789,831]"="blue",
	#			    "(831,3000]"="black"))
#ggsave(paste0(path, '.section.pdf'), plot)
dev.off()

#plot = ggplot(df, aes(1:nrow(df), df[,1])) +
#	geom_line() +
#	xlab(xtitle) +
#	ylab(ytitle) +
#	labs(title = title)
#ggsave(paste0(path, '.pdf'), plot, width=40)
#
#plotly = plot_ly(x = 1:nrow(df), y = df[,1],
#		 type = 'scatter',
#		 mode = 'lines',
#		 name = title) %>%
#	layout(xaxis = list(title = xtitle),
#	       yaxis = list(title = ytitle))
#saveWidget(plotly, paste0(path, '.html'))
