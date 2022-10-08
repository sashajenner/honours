#!/usr/bin/env Rscript
# get quantiles given frequency data
USAGE = 'usage: ./get_quartile.R DATA_FILE'

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.delim(path)

get_median = function(df) {
	nsigs = sum(df$freq)

	if (nsigs %% 2 == 0) {
		mid = c(nsigs / 2, nsigs / 2 + 1)
	} else {
		mid = c((nsigs + 1) / 2)
	}

	i = 1
	j = 1
	sum_freq = 0
	midsig = vector(length=length(mid), mode='integer')
	while (j <= length(mid)) {
		while (i <= length(df$freq)) {
			sum_freq = sum_freq + df$freq[i]
			if (sum_freq >= mid[j]) {
				break
			}
			i = i + 1
		}
		midsig[j] = df$signal[i]
		j = j + 1
	}

	mean(midsig)
}

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

median = get_median(df)

df_left = df[df$signal <= median,]
df_right = df[df$signal >= median,]

q1 = get_median(df_left)
q3 = get_median(df_right)

q0025 = get_quantile(df, 0.025)
q0975 = get_quantile(df, 0.975)

q0005 = get_quantile(df, 0.005)
q0995 = get_quantile(df, 0.995)

prob_lesser_neg128 = sum((df[df$signal <= -128,])$freq) / sum(df$freq)
prob_greater_127 = sum((df[df$signal >= 127,])$freq) / sum(df$freq)

print(q0005)
print(q0025)
print(q1)
print(median)
print(q3)
print(q0975)
print(q0995)

print(prob_lesser_neg128)
print(prob_greater_127)
print(prob_lesser_neg128 + prob_greater_127)
