#!/usr/bin/env Rscript
# ida of stall
USAGE = 'usage: ./stall.R DATA_FILE'

library(ggplot2)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

Mode <- function(x) {
	ux <- unique(x)
	ux[which.max(tabulate(match(x, ux)))]
}

path = args[1]
df = read.csv(path)

x=(14):(784)
stall = df[x,1]

cat('median:',median(stall),'\n')
cat('mode:',Mode(stall),'\n')
cat('min:',min(stall),'\n')
cat('max:',max(stall),'\n')

hist(stall-min(stall))
hist(diff(stall))
