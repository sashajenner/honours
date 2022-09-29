#!/usr/bin/env Rscript
# get jumps from diff file
# 1 or more points
# diff > 25 for at least 1 point
# all diffs +/-ve
USAGE = 'usage: ./get_jumps.R DIFF_DATA_FILE DATA_FILE'

library(plotly)
library(htmlwidgets)

args = commandArgs(TRUE)

if (length(args) != 2) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

sigs = read.csv(args[2])

nr_jumps = 0

jump_starts = vector(mode='numeric', length =nrow(df))
jump_sizes = vector(mode='numeric', length =nrow(df))
jump_diffs = vector(mode='list', length=nrow(df))
jump_is_up = vector(length=nrow(df))

in_jump = 0
is_up = 0
start = 1
is_zero = 0
for (i in 1:nrow(df)) {
	diff = df[i,1]
	if (!in_jump) {
		if (is_zero)
			start = i

		if (diff > 0) {
			if (!is_up) {
				start = i
				is_up = 1
			}
			is_zero = 0
		} else if (diff < 0) {
			if (is_up) {
				start = i
				is_up = 0
			}
			is_zero = 0
		} else {
			is_zero = 1
		}
	} else {
		if (diff > 0) {
			if (!is_up) {
				nr_jumps = nr_jumps + 1
				jump_starts[nr_jumps] = start
				jump_sizes[nr_jumps] = i - start
				jump_diffs[[nr_jumps]] = df[start:(i-1),1]
				jump_is_up[nr_jumps] = is_up

				start = i
				in_jump = 0
			}
			is_up = 1
		} else if (diff < 0) {
			if (is_up) {
				nr_jumps = nr_jumps + 1
				jump_starts[nr_jumps] = start
				jump_sizes[nr_jumps] = i - start
				jump_diffs[[nr_jumps]] = df[start:(i-1),1]
				jump_is_up[nr_jumps] = is_up

				start = i
				in_jump = 0
			}
			is_up = 0
		} else {
			nr_jumps = nr_jumps + 1
			jump_starts[nr_jumps] = start
			jump_sizes[nr_jumps] = i - start
			jump_diffs[[nr_jumps]] = df[start:(i-1),1]
			jump_is_up[nr_jumps] = is_up

			is_zero = 1
			in_jump = 0
		}

	}

	if (abs(diff) > 25)
		in_jump = 1
}

hist(diff(jump_starts[1:nr_jumps]))
hist(jump_sizes[1:nr_jumps])
print(jump_is_up[1:nr_jumps])
print(sum(jump_is_up[1:nr_jumps] == 1))
print(sum(jump_is_up[1:nr_jumps] == 0))

jumps = data.frame(start_idx = jump_starts[1:nr_jumps],
		   size = jump_sizes[1:nr_jumps],
		   is_up = jump_is_up[1:nr_jumps])

jump_pos = vector(mode='numeric', length =nrow(df))
j = 1
for (i in 1:nr_jumps) {
	for (k in 0:(jump_sizes[i]-1)) {
		jump_pos[j] = jump_starts[i]+k
		j=j+1
	}
}
jump_pos = jump_pos[1:(j-1)]

jump_diffs = df[jump_pos,1]
plotly = plot_ly(x = ~jump_diffs, type='histogram')
saveWidget(plotly, paste0(path, '.jumps_diff_hist.html'))

nonjump_pos = vector(mode='numeric', length=nrow(df)-length(jump_diffs))
nonjump_diffs = vector(mode='numeric', length=nrow(df)-length(jump_diffs))
l = 1
for (i in 1:nrow(df)) {
	if (!(i %in% jump_pos)) {
		nonjump_pos[l] = i
		nonjump_diffs[l] = df[i,1]
		l = l + 1
	}
}
plotly = plot_ly(x = ~nonjump_diffs, type='histogram')
saveWidget(plotly, paste0(path, '.nonjumps_diff_hist.html'))

#plotly = plot_ly(x = 1:nrow(sigs), y = sigs[,1],
#		 type = 'scatter',
#		 mode = 'lines')
#for (i in 1:nr_jumps) {
#	pos = seq(jump_starts[i],jump_starts[i]+jump_sizes[i],1)
#	plotly = plotly %>%
#		add_trace(x = pos,
#			  y = sigs[pos,1],
#		  type = 'scatter',
#		  mode = 'lines',
#		  inherit = FALSE,
#		color = 'orange')
#}
#saveWidget(plotly, paste0(path, '.jumps.html'))
