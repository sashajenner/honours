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
NZDSIGS = 56735199469
NREADS = 500000
df$method = reorder(df$method, df$press_ratio)

#df_space_tex = df %>% mutate(bps = press_bytes/NZDSIGS * 8) %>%
#	mutate(press_gib = press_bytes/(1024^3)) %>%
#	select(method, press_ratio, bps, press_gib) %>%
#	arrange(press_ratio)
#print(df_space_tex)
#
#df_time_tex = df %>% mutate(press_time_mins = press_time/(60)) %>%
#	mutate(depress_time_mins = depress_time/(60)) %>%
#	arrange(press_time) %>%
#	select(method, press_time_mins, depress_time_mins, press_ratio)
#print(df_time_tex)

# vbe21

#df = df %>%
#    group_by(generic) %>%
#    mutate(position = rank(press_ratio))

df_ent = df %>%
    mutate(bps = press_bytes/NZDSIGS * 8) %>%
    add_row(method = 'zd entropy',
	    bps = 5.391453)
df_ent[df_ent$method == 'none','bps'] = 16
df_ent$method = reorder(df_ent$method, -df_ent$bps)

#tikz(file = paste0(path, '.size.tex'), width = 6, height = 5)
#ggplot(df, aes(x=generic, y=press_bytes/(1024^3),
#	       fill=generic,
#	       pattern=base, group=position)) +
#geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
#                   color = "black",
#                   #pattern_fill = "black"
#                   pattern_angle = 45,
#                   #pattern_density = 0.1,
#                   #pattern_spacing = 0.025,
#                   #pattern_key_scale_factor = 0.6,
#		   ) +
#       #geom_bar(stat='identity',position='dodge') +
#       ylab('Compression Size (GiB)') +
#       xlab('Second Layer') +
#       labs(pattern = "First Layer",
#	    fill = "Second Layer",
#       ) +
#       guides(pattern = guide_legend(override.aes = list(fill = "white")),
#       		fill = guide_legend(override.aes = list(pattern = "none")))
#       #theme(axis.text.x=element_text(angle=90,hjust=1))
##dev.off()
#
##tikz(file = paste0(path, '.ratio.tex'), width = 6, height = 5)
#ggplot(df, aes(x=generic, y=press_ratio,
#	       fill=generic,
#	       pattern=base, group=position)) +
#geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
#                   color = "black",
#                   #pattern_fill = "black"
#                   pattern_angle = 45,
#                   #pattern_density = 0.1,
#                   #pattern_spacing = 0.025,
#                   #pattern_key_scale_factor = 0.6,
#		   ) +
#       #geom_bar(stat='identity',position='dodge') +
#       ylab('Compression Ratio') +
#       xlab('Second Layer') +
#       labs(pattern = "First Layer",
#	    fill = "Second Layer",
#       ) +
#       guides(pattern = guide_legend(override.aes = list(fill = "white")),
#       		fill = guide_legend(override.aes = list(pattern = "none")))
#       #theme(axis.text.x=element_text(angle=90,hjust=1))
##dev.off()
#
##tikz(file = paste0(path, '.entropy.tex'), width = 6, height = 5)
#ggplot(df, aes(x=generic, y=press_bytes/56735699469*8,
#	       fill=generic,
#	       pattern=base, group=position)) +
#geom_bar_pattern(stat='identity',position = position_dodge(preserve = "single"),
#                   color = "black",
#                   #pattern_fill = "black"
#                   pattern_angle = 45,
#                   #pattern_density = 0.1,
#                   #pattern_spacing = 0.025,
#                   #pattern_key_scale_factor = 0.6,
#		   ) +
#       #geom_bar(stat='identity',position='dodge') +
#       ylab('Entropy (bits per symbol)') +
#       xlab('Second Layer') +
#       labs(pattern = "First Layer",
#	    fill = "Second Layer",
#       ) +
#       guides(pattern = guide_legend(override.aes = list(fill = "white")),
#       		fill = guide_legend(override.aes = list(pattern = "none")))
#       #theme(axis.text.x=element_text(angle=90,hjust=1))
##dev.off()

ggplot(df, aes(x=method, y=press_ratio, fill=press_time/3600)) +
       geom_bar(stat='identity',position='dodge') +
       ylab('Compression Ratio') +
       xlab('Method') +
       labs(fill = "Compression Time (hours)") +
       scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       theme(axis.text.x=element_text(angle=90,hjust=1))

ggplot(df, aes(x=method, y=press_ratio, fill=depress_time/3600)) +
       geom_bar(stat='identity',position='dodge') +
       ylab('Compression Ratio') +
       xlab('Method') +
       labs(fill = "Decompression Time (hours)") +
       scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       theme(axis.text.x=element_text(angle=90,hjust=1))

ggplot(df_ent, aes(x=method, y=bps, fill=press_time/NREADS)) +
       geom_bar(stat='identity',position='dodge') +
       ylab('Bits Per Data Point') +
       xlab('Method') +
       labs(fill = "Compression Time (seconds/read)") +
       scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       theme(axis.text.x=element_text(angle=90,hjust=1))

ggplot(df_ent, aes(x=method, y=bps, fill=depress_time/NREADS)) +
       geom_bar(stat='identity',position='dodge') +
       ylab('Bits Per Data Point') +
       xlab('Method') +
       labs(fill = "Decompression Time (seconds/read)") +
       #scale_fill_viridis_c() +
       scale_fill_continuous(trans = 'reverse',
			     #type = 'viridis'
			     ) +
       guides(fill = guide_colourbar(reverse = TRUE)) +
       theme(axis.text.x=element_text(angle=90,hjust=1))

ggplot(df[df$press_ratio > 2.8,], aes(x=method, y=press_ratio, fill=depress_time/NREADS)) +
       geom_bar(stat='identity',position='dodge') +
       ylab('Compression Ratio') +
       xlab('Method') +
       labs(fill = "Decompression Time (seconds/read)") +
       #scale_fill_viridis_c() +
       scale_fill_continuous(trans = 'reverse',
			     #type = 'viridis'
			     ) +
       guides(fill = guide_colourbar(reverse = TRUE)) +
       theme(axis.text.x=element_text(angle=90,hjust=1))
