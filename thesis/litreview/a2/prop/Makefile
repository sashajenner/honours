.PHONY: clean

main.pdf: *.tex ../bib.bib
	xelatex main && \
	bibtex main && \
	xelatex main && \
	xelatex main
clean:
	rm *.aux *.log *.bbl *.blg *.lof *.lot *.out *.toc *.ilg *.nlo *.nls main.pdf -f
