#!/usr/bin/env Rscript
# plot numbers in a column on the y-axis
USAGE = 'usage: ./sig.R DATA_FILE'

library(ggplot2)
library(plotly)
library(htmlwidgets)
library(tikzDevice)

args = commandArgs(TRUE)

if (length(args) != 1) {
	stop(USAGE)
}

path = args[1]
df = read.csv(path)
name = basename(path)
colname = names(df)[1]
title = paste(name, colname)
xtitle = 'Data Point'
ytitle = colname

#x=1:nrow(df)
#x=900:1100
#stall=c(20,778)
#x=stall[1]:stall[2]
#tikz(file = paste0(path, '.stall.tex'), width = 5, height = 5)
#plot = ggplot(data.frame(df[x,]), aes(x, df[x,1])) + #color=cut(x, c(1,14,789,831,3000)))) +
#	geom_line() +
#	xlab('Position in Read') +
#	ylab('Raw Signal') +
#	theme(axis.title.x=element_blank(),
#	      axis.text.x=element_blank(),
#	      axis.ticks.x=element_blank())
#	#xlim(0,1500) +
#	#geom_rect(aes(xmin=1, xmax=14, ymin=-Inf,ymax=Inf),fill='red',alpha=0.002) +
#	#geom_vline(xintercept=65719,colour='red',alpha=0.9) +
#	#geom_vline(xintercept=65949,colour='red',alpha=0.9)
#	#geom_vline(xintercept=22741,colour='dark orange',alpha=0.9) +
#	#geom_vline(xintercept=24384,colour='blue',alpha=0.9) +
#	#geom_vline(xintercept=25329,colour='purple',alpha=0.9) +
#	#geom_text(label='',colour='red',x=,y=,alpha=0.9)
#	#geom_text(label='TCCCAA',colour='dark orange',x=22741+215,y=475-10,alpha=0.9) +
#	#geom_text(label='CCCAAG',colour='dark orange',x=22741+215,y=475-20,alpha=0.9) +
#	#geom_text(label='CCAAGT',colour='blue',x=24384+215,y=420,alpha=0.9)
#	#scale_color_manual(values=c("[1,14]"="red",
#	#			    "(14,789]"="green",
#	#			    "(789,831]"="blue",
#	#			    "(831,3000]"="black"))
#ggsave(paste0(path, '.pres.nox.pdf'), plot, width=8, height=2.5)
#dev.off()

#print(df)
#for (i in 1:nrow(df)) {
#	cat(i, ',', df[i,1], '\n', sep='')
#}
#df_new = df - min(df)
#print(df_new)
#max(df_new)

#tikz(file = paste0(path, '.tex'), width = 5, height = 5)
plot = ggplot(df, aes(1:nrow(df), df[,1])) +
	geom_line() +
	xlab('Position in Read') +
	ylab('Raw Signal')
	#theme(text = element_text(size = 40,
	#			  family = 'serif'),
	#	plot.margin = unit(c(0,2,0,0), "cm"))
	#scale_y_continuous(sec.axis=sec_axis(trans=~(.-243)*748.580139/2048,
	#					name='Ionic Current (pA)'))
	#labs(title = title)
ggsave(paste0(path, '.pdf'), plot, width=20)
#dev.off()

plotly = plot_ly(x = 1:nrow(df), y = df[,1],
		 type = 'scatter',
		 mode = 'lines',
		 name = title) %>%
	layout(xaxis = list(title = xtitle),
	       yaxis = list(title = ytitle))
saveWidget(plotly, paste0(path, '.html'))
