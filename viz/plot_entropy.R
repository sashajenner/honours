#!/usr/bin/env Rscript
# bar plot entropies
USAGE = 'usage: ./plot_entropy.R TEST.OUT'

library(ggplot2)
#library(ggpattern)
#library(tikzDevice)

# check args
args = commandArgs(TRUE)
if (length(args) != 1) {
	stop(USAGE)
}

# read file
path = args[1]
df = read.delim(path)

#df_P11 = df[df$data == 'P11',]
df$method = reorder(df$method, -df$entropy)

#tikz(file = paste0(path, '.size.tex'), width = 6, height = 5)
ggplot(df, aes(x=method, y=entropy)) +
       geom_bar(stat='identity') +
       ylab('Entropy (bits per symbol)') +
       xlab('Method')
       #theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()
