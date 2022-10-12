#!/usr/bin/env Rscript
# plot histogram given frequency data
USAGE = 'usage: ./freqhist.R DATA_FILE'

library(ggplot2)
library(plotly)
library(htmlwidgets)
library(tikzDevice)

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
ytitle = names(df)[2]

min0 = 175
min100 = 178
minmil = min(df[df$freq > 10^6,1])
max0 = 1048
max100 = 932
maxmil = max(df[df$freq > 10^6,1])
mean = 475.224468

print(minmil)
print(maxmil)

print(sum(df[df$signal>100,2]))
print(sum(df$signal <= 255 & df$freq != 0))

#tikz(file = paste0(path, '.hist.tex'), width = 5, height = 5)
#ggplot(df, aes(x=df[,1], y=df[,2]/10^9)) +
#	geom_histogram(stat='identity') +
#	xlab('Raw Signal Zig-Zag Delta') +
#	ylab('Frequency ($\\times 10^9$)') +
#	xlim(c(-1,maxmil))
#dev.off()
#
#plot = ggplot(df, aes(x=df[,1], y=df[,2])) +
#	geom_histogram(stat="identity") +
#	xlab(xtitle) +
#	labs(title = title)
#ggsave(paste0(path, '.hist.pdf'), plot)
#
#plotly = plot_ly(x = df[,1], y=df[,2],
#		 type = 'bar',
#		 name = title) %>%
#	layout(xaxis = list(title = xtitle))
#saveWidget(plotly, paste0(path, '.hist.html'))

#huf_bits = 0
#for (i in 1:nrow(df)) {
#	zig = 2 * abs(df$signal[i])
#	if (zig < 0)
#		zig = zig - 1
#	huf_bits = huf_bits + df$freq[i] * (zig + 1)
#}
#print(huf_bits / 8)

#n = sum(df$freq)
#for (i in c(2^1,2^2,2^3,2^4,2^5,2^6,2^7,2^8)) {
#	cat(log2(i), sum(df$freq[1:i]) / n, '\n')
#}
