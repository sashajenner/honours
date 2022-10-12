#!/usr/bin/env Rscript
# plot huffman stuff given number of bits of each huffman code
USAGE = 'usage: ./huffman.R DATA_FILE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

library(ggplot2)
library(tikzDevice)

tikz(file = paste0(path, '.len.tex'), width = 5, height = 5)
ggplot(df, aes(x=0:255, y=df[,1])) +
       geom_bar(stat='identity') +
       xlab('Raw Signal Zig-Zag Delta') +
       ylab('Huffman Code Length (Bits)')
dev.off()
