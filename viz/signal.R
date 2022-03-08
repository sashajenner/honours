#!/usr/bin/env Rscript
# plot numbers in a column on the y-axis
usage = "usage: ./signal.R DATA_FILE"

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(usage)
}
