#!/usr/bin/env Rscript
# plot zigzag encoding

library(ggplot2)

tikzDevice::tikz(file = "./zigzag.tex", width = 3, height = 3)

z = function(x) {
	2 * abs(x) - (x < 0)
}

x = array(-15:15)
df = data.frame(x = x, y = apply(x, 1, z))
ggplot(df, aes(x,y)) +
	geom_point() +
	xlab("$x$") +
	ylab("$z(x)$")
