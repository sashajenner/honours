#!/usr/bin/env Rscript
# analyse the big zigzag deltas between numbers in a column with a header
USAGE = "./big_zd.R DATA_FILE"

library(plotly)
library(htmlwidgets)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)

big_zd = df[df[,1] > 50,1]
big_zd_pos = (1:nrow(df))[df[,1] > 50]
diff_big_zd_pos = diff(big_zd_pos)

plot(1:(length(big_zd_pos)-1), diff(big_zd_pos))
hist(diff(big_zd_pos))

plotly = plot_ly(x = 1:length(diff_big_zd_pos), y= diff_big_zd_pos,
		 type = 'scatter',
		 mode = 'lines')
saveWidget(plotly, 'Rplots.html')
