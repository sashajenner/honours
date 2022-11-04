#!/usr/bin/env Rscript
# bar plot compression ratios
USAGE = 'usage: ./plot_ratio.R TEST.OUT'

library(ggplot2)
library(ggpattern)
library(tikzDevice)
library(dplyr)
library(ggtext)
library(ggrepel)

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
NSIGS = 56735699469
NREADS = 500000
df$method = reorder(df$method, df$press_ratio)

df_space_tex = df %>% mutate(bps = press_bytes/NZDSIGS * 8) %>%
	mutate(press_gib = press_bytes/(1024^3)) %>%
	mutate(space_saving = 1-press_bytes/depress_bytes) %>%
	select(method, press_ratio, space_saving, bps, press_gib) %>%
	arrange(press_ratio)
#print(df_space_tex)

#df_time_tex = df %>% mutate(press_time_mins = press_time/(60)) %>%
#	mutate(depress_time_mins = depress_time/(60)) %>%
#	arrange(press_time) %>%
#	select(method, press_time_mins, depress_time_mins, press_ratio)
df_time_tex = df %>% mutate(press_time_hpt = (press_time/3600)/(depress_bytes/(1024^4))) %>%
	mutate(depress_time_hpt = (depress_time/3600)/(depress_bytes/(1024^4))) %>%
	arrange(depress_time) %>%
	select(method, press_time_hpt, depress_time_hpt, press_ratio)
#print(df_time_tex)

df = df %>% mutate(bps = press_bytes/NZDSIGS * 8) %>%
	mutate(press_gib = press_bytes/(1024^3)) %>%
	mutate(space_saving = 1-press_bytes/depress_bytes) %>%
	mutate(press_time_hpt = (press_time/3600)/(depress_bytes/(1024^4))) %>%
	mutate(depress_time_hpt = (depress_time/3600)/(depress_bytes/(1024^4)))

space_time_boundary_c = c("none",
			"svb-zd",
			"svb16-zd",
			"zstd-svb-zd",
			"rc0-vbbe21-zd",
			"rc1-vbe21-zd",
			"rc1-vbbe21-zd",
			"shuff-vbbe21-zd",
			"rc01s-vbbe21-zd",
			"dstall-fz-1500",
			"dstall-fz")

space_time_boundary_d = c("none",
			  "zstd",
			  "zstd-svb-zd",
			  "rc1-vbe21-zd",
			  "rc1-vbbe21-zd",
			  "shuff-vbbe21-zd",
			  "rc01s-svb-zd",
			  "rc01s-svb16-zd",
			  "dstall-fz-1500",
			  "dstall-fz")

method_boundary_c = c()
for (method in df$method) {
	if (method %in% space_time_boundary_c) {
		method_boundary_c = c(method_boundary_c, method)
	} else {
		method_boundary_c = c(method_boundary_c, "")
	}
}
df$method_boundary_c = method_boundary_c
df_boundary_c = df[df$method_boundary_c != "",]

method_boundary_d = c()
for (method in df$method) {
	if (method %in% space_time_boundary_d) {
		method_boundary_d = c(method_boundary_d, method)
	} else {
		method_boundary_d = c(method_boundary_d, "")
	}
}
df$method_boundary_d = method_boundary_d
df_boundary_d = df[df$method_boundary_d != "",]

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

#tikz(file = paste0(path, '.ratio.bar.tex'), width = 5, height = 4)
plot = ggplot(df, aes(x=method, y=press_ratio
		,fill=factor(ifelse(method=="zstd-svb-zd","1","0"))
	       )) +
	       #, fill=press_time_hpt)) +
       scale_fill_manual(name = "method", values=c("grey50", "grey35")) +
       geom_col() +
       ylab('Compression Ratio') +
       xlab('Method') +
       geom_hline(yintercept=16/7.70) +
       geom_hline(yintercept=16/5.39, linetype='dashed') +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       scale_x_discrete(limits = rev(levels(df$method))) +
       coord_flip() +
       theme(legend.position='none')
       #+geom_segment(aes(yend = 3, xend = 14, y = 3.3, x = 14),
#		    arrow = arrow(length = unit(0.5, "cm")))
#dev.off()
ggsave(paste0(path, '.ratio.bar.pdf'), plot, width=5, height=4)

#tikz(file = paste0(path, '.bps.bar.tex'), width = 5, height = 5)
plot = ggplot(df, aes(x=method, y=bps
		,fill=factor(ifelse(method=="zstd-svb-zd","1","0"))
	       )) +
	       #, fill=press_time_hpt)) +
       scale_fill_manual(name = "method", values=c("grey50", "grey35")) +
       geom_col() +
       ylab('Bits per Symbol') +
       xlab('Method') +
       geom_hline(yintercept=7.70) +
       geom_hline(yintercept=5.39, linetype='dashed') +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       scale_x_discrete(limits = rev(levels(df$method))) +
       coord_flip() +
       theme(legend.position='none') +
       scale_y_continuous(name = "Bits per Symbol", sec.axis =
			  sec_axis(~./8*NZDSIGS/(1024^3), name = "Compressed Size (GiB)"))
#dev.off()
ggsave(paste0(path, '.bps.bar.pdf'), plot, width=5, height=5)

#tikz(file = paste0(path, '.ss.bar.tex'), width = 5, height = 4)
plot = ggplot(df, aes(x=method, y=space_saving
		,fill=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_col() +
       ylab('Space Saving') +
       xlab('Method') +
       scale_fill_manual(name = "method", values=c("grey50", "grey35")) +
       geom_hline(yintercept=1-7.70/16) +
       geom_hline(yintercept=1-5.39/16, linetype='dashed') +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       scale_x_discrete(limits = rev(levels(df$method))) +
       coord_flip() +
       theme(legend.position='none')
#dev.off()
ggsave(paste0(path, '.ss.bar.pdf'), plot, width=5, height=4)

ggplot(df_boundary_c, aes(x=space_saving, y=press_time_hpt
		#,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_line(alpha=0.4) +
       geom_point(aes(colour=factor(ifelse(method=="zstd-svb-zd","1","0")))) +
       ylab('Compression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_c, colour=factor(ifelse(method=="zstd-svb-zd","1","0"))))

ggplot(df_boundary_c, aes(x=press_ratio, y=press_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point() +
       ylab('Compression Time (hr / TiB)') +
       xlab('Compression Ratio') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_c))

#tikz(file = paste0(path, '.ss-ct.tex'), width = 4, height = 4)
plot = ggplot(df, aes(x=space_saving, y=press_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point(alpha=0.6) +
       ylab('Compression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_c))
#dev.off()
ggsave(paste0(path, '.ss-ct.pdf'), plot, width=4, height=4)

df_06 = df_boundary_c[df_boundary_c$space_saving > 0.6,]
plot = ggplot(df_06, aes(x=space_saving, y=press_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point(alpha=0.8) +
       ylab('Compression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_c))
#dev.off()
ggsave(paste0(path, '.ss-ct06.pdf'), plot, width=4, height=4)

#tikz(file = paste0(path, '.ss-dt.boundary.tex'), width = 5, height = 5)
ggplot(df_boundary_d, aes(x=space_saving, y=depress_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point() +
       ylab('Decompression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_d))
#dev.off()

#tikz(file = paste0(path, '.ss-dt.tex'), width = 4, height = 4)
plot = ggplot(df, aes(x=space_saving, y=depress_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point(alpha=0.6) +
       ylab('Decompression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_d))
#dev.off()
ggsave(paste0(path, '.ss-dt.pdf'), plot, width=4, height=4)

df_06 = df_boundary_d[df_boundary_d$space_saving > 0.6,]
plot = ggplot(df_06, aes(x=space_saving, y=depress_time_hpt
		,colour=factor(ifelse(method=="zstd-svb-zd","1","0"))
		)) +
	       #, fill=press_time_hpt)) +
       geom_point(alpha=0.8) +
       ylab('Decompression Time (hr / TiB)') +
       xlab('Space Saving') +
       scale_color_manual(name = "method", values=c("grey50", "red")) +
       #labs(fill = "Compression Time (hrs / TiB)") +
       #scale_fill_continuous(high = "#132B43", low = "#56B1F7") +
       #theme(axis.text.x=element_text(angle=90,hjust=1)) +
       #scale_x_discrete(limits = rev(levels(df$method))) +
       #coord_flip() +
       theme(legend.position='none') +
	geom_text_repel(aes(label=method_boundary_d))
#dev.off()
ggsave(paste0(path, '.ss-dt06.pdf'), plot, width=4, height=4)

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

df_06 = df[df$space_saving > 0.6,]
plot = ggplot(df_06, aes(y=press_time_hpt, x=depress_time_hpt)) +
	geom_point(alpha=0.8, aes(colour=space_saving)) +
	xlab('Decompression Time (hr / TiB)') +
	ylab('Compression Time (hr / TiB)') +
       scale_colour_viridis_c() +
       #scale_color_manual(name = "method", values=c("grey50", "red")) +
       labs(colour = "Space Saving") +
	geom_text_repel(aes(label=method), max.time=10, max.iter=1000000)
ggsave(paste0(path, '.ct-dt.pdf'), plot, width=7, height=6)
