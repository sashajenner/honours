#!/usr/bin/env Rscript
# get the frequency of positive and negative repeats
# output as one column with '_signrep' appended to the header
USAGE = "./get_signrep.R DATA_FILE"

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

cnt = 0
cat(names(df)[1], '_signrep\n', sep = '')
for (x in df[,1]) {
	if (x > 0 && cnt > 0) {
		cnt = cnt + 1
	} else if (x < 0 && cnt < 0) {
		cnt = cnt - 1
	} else {
		if (cnt != 0) {
			cat(cnt, '\n', sep = '')
			cnt = 0
		}

		if (x > 0) {
			cnt = cnt + 1
		} else if (x < 0) {
			cnt = cnt - 1
		}
	}
}

if (cnt != 0) {
	cat(x, '\n', sep = '')
}
