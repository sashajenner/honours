#!/usr/bin/env Rscript
# ida test
USAGE = 'usage: ./ida_test.R TEST.OUT'

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
df$space = 1 - df$press_bytes / df$depress_bytes
df$sc = (df$press_time / 3600) / (df$depress_bytes/(1024^4))
df$sd = (df$depress_time / 3600) / (df$depress_bytes/(1024^4))
df$oan = df$space - 0.01 * df$sc - 0.03 * df$sd
df$oar = df$space - 1/480 * df$sc - 1/96 * df$sd

df$method = reorder(df$method, -df$oan)

#tikz(file = paste0(path, '.size.tex'), width = 6, height = 5)
ggplot(df, aes(x=method, y=oan)) +
       geom_bar(stat='identity') +
       #ylab('') +
       xlab('Method') +
       theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()

df$method = reorder(df$method, -df$oar)

#tikz(file = paste0(path, '.size.tex'), width = 6, height = 5)
ggplot(df, aes(x=method, y=oar)) +
       geom_bar(stat='identity') +
       #ylab('') +
       xlab('Method') +
       theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()
