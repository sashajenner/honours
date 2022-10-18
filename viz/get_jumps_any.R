#!/usr/bin/env Rscript

USAGE = 'usage: ./get_jumps_any.R DATA_FILE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

library(ggplot2)
library(ggtext)
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
		indec[nindec,] = c(start, i + 1)

		start = i + 1
	}

	trend = new_trend
}

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
	len = indec[i,]$end - indec[i,]$start + 1
	ld = max(abs(df_diff[x_diff]))
	df[x,'group'] = rep(ld, len)
	if (ld > 25)
		df[x,'>25'] = rep(30, len)
	else
		df[x,'>25'] = rep(0, len)
}
x=(28900):(29200)
tikz(file = paste0(path, '.jumps.epsilon.tex'), width = 6, height = 5)
ggplot(data.frame(df[x,])) +
	geom_line(aes(x, df[x,1], color=group)) +
	xlab('Position in Read') +
	ylab('Raw Signal') +
	scale_colour_viridis_c() +
	labs(colour = "Maximum\nAbsolute\nDelta")
dev.off()

#print(df[x,])
#ggplot(data.frame(df[x,])) +
#	geom_line(aes(x, df[x,1], colour=df[x,'>25'])) +
#	xlab('Position in Read') +
#	ylab('Raw Signal') +
