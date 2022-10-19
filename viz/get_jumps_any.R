#!/usr/bin/env Rscript

USAGE = 'usage: ./get_jumps_any.R DATA_FILE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

library(ggplot2)
library(ggtext)
library(ggrepel)
library(tikzDevice)

path = args[1]
df = read.csv(path)

df_diff = diff(df[,1])

trend = 0
indec = data.frame(start=numeric(), end=numeric())
nindec = 0
for (i in 28900:29199) {
	if (df_diff[i] == 0) {
		new_trend = 0
	} else if (df_diff[i] > 0) {
		new_trend = 1
	} else {
		new_trend = -1
	}

	if (trend == 0 && new_trend != 0) {
		start = i
	} else if (trend != new_trend) {
		nindec = nindec + 1
		indec[nindec,] = c(start, i)

		start = i
	}

	trend = new_trend
}
indec[nindec,] = c(start, i)

x=(28900):(29200)
plot = ggplot(data.frame(df[x,])) +
	geom_line(aes(x, df[x,1])) +
	xlab('Position in Read') +
	ylab('Raw Signal')
for (i in nrow(indec)) {
	x = seq(indec[i,]$start,indec[i,]$end)
	plot = plot + geom_line(aes(x, df[x,1]))
}

for (i in 1:nrow(indec)) {
	x = indec[i,]$start:indec[i,]$end
	x_diff = indec[i,]$start:(indec[i,]$end-1)
	len = length(x)
	ld = max(abs(df_diff[x_diff]))
	df[x,'group'] = rep(ld, len)
	#if (len == 2)
	#	textpos = 1
	#else
	#	textpos = len-2
	if (ld > 24)
		df[x,'g25'] = 1
	else
		df[x,'g25'] = 0
}
x=(28900):(29200)
tikz(file = paste0(path, '.jumps.25.tex'), width = 5, height = 5)
ggplot(data.frame(df[x,])) +
	geom_line(aes(x, df[x,1], color=g25)) +
	#geom_point(aes(x, df[x,1], color=group), size=1) +
	xlab('Position in Read') +
	ylab('Raw Signal') +
	#scale_colour_viridis_c() +
	#labs(colour = "Maximum\nAbsolute\nDelta") +
	#geom_text_repel(aes(x, df[x,1], color=group, label=g25))
	guides(color='none')
dev.off()

#print(df[x,])
#ggplot(data.frame(df[x,])) +
#	geom_line(aes(x, df[x,1], colour=df[x,'>25'])) +
#	xlab('Position in Read') +
#	ylab('Raw Signal') +
