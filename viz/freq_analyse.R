#!/usr/bin/env Rscript
# analyse frequency table
# print results
# e.g.
#
# n	666
# min	0
# q1	3
# mean	6
# median	5
# q3	7
# max	10
# mode	4
# var	4
# std	2
# entropy	10
#
# entropy is bits per symbol
USAGE = 'usage: ./freq_analyse.R FREQ_TABLE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path)

get_quantile = function(df, quantile) {
	nsigs = sum(df$freq)

	mid = nsigs * quantile

	i = 1
	j = 1
	sum_freq = 0
	while (i <= length(df$freq)) {
		sum_freq = sum_freq + df$freq[i]
		if (sum_freq >= mid[j]) {
			break
		}
		i = i + 1
	}
	midsig = df$signal[i]
	j = j + 1

	midsig
}

n = sum(df$freq)
min = min(df$signal)
q1 = get_quantile(df, 0.25)
mean = sum(df$signal * (df$freq / n))
median = get_quantile(df, 0.5)
q3 = get_quantile(df, 0.75)
max = max(df$signal)
mode = df$signal[which(df$freq == max(df$freq))]
var = sum((df$signal - mean) ^ 2 * df$freq) / n
std = sqrt(var)
df$prob = df$freq / n
prob_nonzero = df$prob[df$prob != 0]
ent = - prob_nonzero %*% log2(prob_nonzero)

cat('n\t', n, '\n',
    'min\t', min, '\n',
    'q1\t', q1, '\n',
    'mean\t', mean, '\n',
    'median\t', median, '\n',
    'q3\t', q3, '\n',
    'max\t', max, '\n',
    'mode\t', sep='')
cat(mode)
cat('\n',
    'var\t', var, '\n',
    'std\t', std, '\n',
    'entropy\t', ent, '\n',
    sep='')
