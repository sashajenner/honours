#!/usr/bin/env Rscript
# bar plot compression ratios
USAGE = 'usage: ./plot_ratio.R TEST.OUT'

library(ggplot2)

# check args
args = commandArgs(TRUE)
if (length(args) != 1) {
	stop(USAGE)
}

# read file
path = args[1]
df = read.delim(path)

df_P11 = df[df$data == 'P11',]
df_P11$method = reorder(df_P11$method, df_P11$press_ratio)

ggplot(df_P11, aes(x=method, y=press_ratio)) +
       geom_bar(stat='identity') +
       theme(axis.text.x=element_text(angle=90,hjust=1))
