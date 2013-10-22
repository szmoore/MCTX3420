#Makefile for thesis

all : 
	make clean
	make report.pdf

report.pdf : report.tex
	rm -f *.aux *.bbl *.log *.toc *.lof *.blg *.lot

	pdflatex --shell-escape report
	bibtex report
	pdflatex --shell-escape report
	pdflatex --shell-escape report

	rm -f *.bbl *.log *.toc *.lof *.blg *.lot


clean : 
	rm -f report.pdf
