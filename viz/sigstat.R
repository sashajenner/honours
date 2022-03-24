#!/usr/bin/env Rscript
# get stats about numbers in a column
USAGE = 'usage: ./sigstat.R DATA_FILE'

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

cat('min: ', min(df[,1]), '\n',
    'max: ', max(df[,1]), '\n',
    'mean: ', mean(df[,1]), '\n',
    'median: ', median(df[,1]), '\n',
    'mode: ', Mode(df[,1]), '\n',
    'sample sd: ', sd(df[,1]), '\n',
    sep = '')
