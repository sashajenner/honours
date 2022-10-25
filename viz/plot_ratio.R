#!/usr/bin/env Rscript
# bar plot compression ratios
USAGE = 'usage: ./plot_ratio.R TEST.OUT'

library(ggplot2)
library(ggpattern)
library(tikzDevice)
library(dplyr)

# check args
args = commandArgs(TRUE)
if (length(args) != 1) {
	stop(USAGE)
}

# read file
path = args[1]
df = read.delim(path)

#df_P11 = df[df$data == 'P11',]
#df$method = reorder(df$method, df$press_ratio)

# vbe21

df = df %>%
    group_by(generic) %>%
    mutate(position = rank(press_ratio))

#tikz(file = paste0(path, '.size.tex'), width = 6, height = 5)
ggplot(df, aes(x=generic, y=press_bytes/(1024^3),
	       fill=generic,
	       pattern=base, group=position)) +
geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
                   color = "black",
                   #pattern_fill = "black"
                   pattern_angle = 45,
                   #pattern_density = 0.1,
                   #pattern_spacing = 0.025,
                   #pattern_key_scale_factor = 0.6,
		   ) +
       #geom_bar(stat='identity',position='dodge') +
       ylab('Compression Size (GiB)') +
       xlab('Second Layer') +
       labs(pattern = "First Layer",
	    fill = "Second Layer",
       ) +
       guides(pattern = guide_legend(override.aes = list(fill = "white")),
       		fill = guide_legend(override.aes = list(pattern = "none")))
       #theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()

#tikz(file = paste0(path, '.ratio.tex'), width = 6, height = 5)
ggplot(df, aes(x=generic, y=press_ratio,
	       fill=generic,
	       pattern=base, group=position)) +
geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
                   color = "black",
                   #pattern_fill = "black"
                   pattern_angle = 45,
                   #pattern_density = 0.1,
                   #pattern_spacing = 0.025,
                   #pattern_key_scale_factor = 0.6,
		   ) +
       #geom_bar(stat='identity',position='dodge') +
       ylab('Compression Ratio') +
       xlab('Second Layer') +
       labs(pattern = "First Layer",
	    fill = "Second Layer",
       ) +
       guides(pattern = guide_legend(override.aes = list(fill = "white")),
       		fill = guide_legend(override.aes = list(pattern = "none")))
       #theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()

#tikz(file = paste0(path, '.entropy.tex'), width = 6, height = 5)
ggplot(df, aes(x=generic, y=press_bytes/56735699469*8,
	       fill=generic,
	       pattern=base, group=position)) +
geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
                   color = "black",
                   #pattern_fill = "black"
                   pattern_angle = 45,
                   #pattern_density = 0.1,
                   #pattern_spacing = 0.025,
                   #pattern_key_scale_factor = 0.6,
		   ) +
       #geom_bar(stat='identity',position='dodge') +
       ylab('Entropy (bits per symbol)') +
       xlab('Second Layer') +
       labs(pattern = "First Layer",
	    fill = "Second Layer",
       ) +
       guides(pattern = guide_legend(override.aes = list(fill = "white")),
       		fill = guide_legend(override.aes = list(pattern = "none")))
       #theme(axis.text.x=element_text(angle=90,hjust=1))
#dev.off()
