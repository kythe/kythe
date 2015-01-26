all: abstract.pdf

view: abstract.pdf
	google-chrome abstract.pdf &

.PHONY: abstract.pdf all clean

abstract.pdf: abstract.tex
	latexmk -pdf -pdflatex="pdflatex -interactive=nonstopmode" -use-make $<

clean:
	latexmk -CA
